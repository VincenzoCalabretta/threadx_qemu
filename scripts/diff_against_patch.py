#!/usr/bin/env python3
"""Compare the current repo against the state workstate.patch would produce.

workstate.patch is a set of `diff --git` hunks. Hunks whose source side is
/dev/null are pure file creations: the target file can be rebuilt entirely
from the '+' lines. Hunks that modify a pre-existing file need the a-version,
which this repo (having been rebuilt from scratch) does not have. Those files
are listed but not diffed.

The output has three sections:
  1. patch-only paths       : patch adds them, current repo doesn't have them
                              (i.e. things you chose to drop)
  2. common paths + diffs   : patch adds them, current repo has them
                              (shows what you reshaped)
  3. modify-only patch paths: patch only modifies them (base needed, skipped)
"""

from __future__ import annotations

import argparse
import difflib
import os
import re
import stat
import subprocess
import sys
import tempfile
from pathlib import Path


PATCH_FILE_HEADER = re.compile(r"^diff --git a/(.+) b/(.+)$")
FILE_MODE_RE = re.compile(r"^new file mode (\d+)$")


def parse_patch(text: str):
    """Yield dicts describing each file the patch touches.

    Each dict:
      "b_path"      : target path
      "is_new"      : True if source is /dev/null
      "added_lines" : '+' lines (populated only for new files)
      "raw_block"   : full slice of the patch for this file (from
                      `diff --git` up to but excluding the next one)
    """
    lines = text.splitlines(keepends=True)
    i = 0
    n = len(lines)
    while i < n:
        m = PATCH_FILE_HEADER.match(lines[i].rstrip("\n"))
        if not m:
            i += 1
            continue

        b_path = m.group(2)

        # Advance until the next file header (or EOF).
        j = i + 1
        while j < n and not lines[j].startswith("diff --git "):
            j += 1

        block = lines[i:j]                # full block, including 'diff --git ...'
        body = lines[i + 1 : j]           # everything after the diff --git line
        is_new = any(l.startswith("--- /dev/null") for l in body)

        mode = "100644"
        for l in body:
            m2 = FILE_MODE_RE.match(l.rstrip("\n"))
            if m2:
                mode = m2.group(1)
                break

        added: list[str] = []
        if is_new:
            in_hunk = False
            for l in body:
                if l.startswith("@@ "):
                    in_hunk = True
                    continue
                if not in_hunk:
                    continue
                if l.startswith("+") and not l.startswith("+++"):
                    added.append(l[1:])

        yield {
            "b_path": b_path,
            "is_new": is_new,
            "added_lines": added,
            "raw_block": "".join(block),
            "mode": mode,
        }
        i = j


def repo_root() -> Path:
    out = subprocess.check_output(
        ["git", "rev-parse", "--show-toplevel"], text=True
    ).strip()
    return Path(out)


def color(s: str, code: str, use_color: bool) -> str:
    if not use_color:
        return s
    return f"\033[{code}m{s}\033[0m"


# --------------------------------------------------------------------------- #
# Delta-patch emission                                                        #
# --------------------------------------------------------------------------- #


def _emit_lines(lines: list[str], prefix: str) -> str:
    """Prefix each line for a hunk body, tacking on `\\ No newline` if needed."""
    out: list[str] = []
    for l in lines:
        if l.endswith("\n"):
            out.append(prefix + l)
        else:
            out.append(prefix + l + "\n")
            out.append("\\ No newline at end of file\n")
    return "".join(out)


def make_delete_block(path: str, lines: list[str], mode: str) -> str:
    n = len(lines)
    body = _emit_lines(lines, "-") if n else ""
    hunk = f"@@ -1,{n} +0,0 @@\n" if n else ""
    return (
        f"diff --git a/{path} b/{path}\n"
        f"deleted file mode {mode}\n"
        f"--- a/{path}\n"
        f"+++ /dev/null\n"
        f"{hunk}{body}"
    )


def make_create_block(path: str, lines: list[str], mode: str) -> str:
    n = len(lines)
    body = _emit_lines(lines, "+") if n else ""
    hunk = f"@@ -0,0 +1,{n} @@\n" if n else ""
    return (
        f"diff --git a/{path} b/{path}\n"
        f"new file mode {mode}\n"
        f"--- /dev/null\n"
        f"+++ b/{path}\n"
        f"{hunk}{body}"
    )


def file_mode(p: Path) -> str:
    return "100755" if (p.stat().st_mode & stat.S_IXUSR) else "100644"


def list_current_files(root: Path, exclude: set[str]) -> list[str]:
    """Enumerate every non-ignored file in the repo, sorted, relative to root."""
    raw = subprocess.check_output(
        ["git", "-C", str(root), "ls-files", "-o", "-c", "--exclude-standard"],
        text=True,
    )
    files = sorted({l for l in raw.splitlines() if l and l not in exclude})
    return files


DELTA_HEADER_TMPL = """\
# delta.patch - generated by scripts/diff_against_patch.py
#
# Goal:  base + workstate.patch + delta.patch  ==  current threadx_qemu_nix repo
#
# This file has three sections. All are unified-diff `diff --git` blocks.
#
#   [A] Deletions  ({n_del} files) — files workstate.patch created that this
#       repo no longer wants. These blocks are self-verifying (they carry the
#       exact content workstate.patch would have produced) and apply cleanly.
#
#   [B] Fresh creations  ({n_new} files) — files this repo added that
#       workstate.patch never touched. Apply cleanly.
#
#   [C] Overriding creations  ({n_over} files) — files workstate.patch
#       *modifies* on top of an unknown base, and this repo has its own
#       replacement. Applying naively will conflict because the target file
#       already exists after workstate.patch. To apply this section:
#         for f in {overrides_flat}; do rm -f "$f"; done
#         git apply delta.patch
#       Or use `patch -p1 --force < delta.patch`.
#
# Modify-only paths workstate.patch touches that this repo does NOT ship
# ({n_skip} files) are left alone by this delta; delete them by hand if you
# don't want them:
#   {skipped_flat}
#
"""


def render_header(
    n_del: int,
    n_new: int,
    n_over: int,
    overrides: list[str],
    skipped: list[str],
) -> str:
    return DELTA_HEADER_TMPL.format(
        n_del=n_del,
        n_new=n_new,
        n_over=n_over,
        n_skip=len(skipped),
        overrides_flat=" ".join(overrides) if overrides else "(none)",
        skipped_flat=" ".join(skipped) if skipped else "(none)",
    )


def build_delta(
    entries: list[dict], root: Path, delta_out: Path
) -> tuple[list[str], list[str], list[str], list[str]]:
    """Return (deletes, safe_creates, override_creates, skipped_modifies)."""
    entries_by_path = {e["b_path"]: e for e in entries}
    touched_paths = set(entries_by_path)

    deletes: list[str] = []
    override_creates: list[str] = []
    skipped_modifies: list[str] = []

    for e in entries:
        b = e["b_path"]
        current = root / b
        if e["is_new"]:
            if not current.exists():
                deletes.append(b)
        else:
            if current.exists():
                override_creates.append(b)
            else:
                skipped_modifies.append(b)

    excludes = {"workstate.patch", str(delta_out.relative_to(root))}
    all_current = list_current_files(root, excludes)
    safe_creates = [f for f in all_current if f not in touched_paths]

    return deletes, safe_creates, override_creates, skipped_modifies


def write_delta(
    entries: list[dict],
    root: Path,
    delta_out: Path,
) -> tuple[int, int, int, int]:
    entries_by_path = {e["b_path"]: e for e in entries}
    deletes, safe_creates, override_creates, skipped = build_delta(
        entries, root, delta_out
    )

    parts: list[str] = []
    parts.append(
        render_header(
            n_del=len(deletes),
            n_new=len(safe_creates),
            n_over=len(override_creates),
            overrides=override_creates,
            skipped=skipped,
        )
    )

    parts.append("# [A] Deletions\n")
    for b in deletes:
        e = entries_by_path[b]
        parts.append(make_delete_block(b, e["added_lines"], e["mode"]))

    parts.append("# [B] Fresh creations\n")
    for b in safe_creates:
        p = root / b
        try:
            lines = p.read_text().splitlines(keepends=True)
        except UnicodeDecodeError:
            print(f"skip (binary): {b}", file=sys.stderr)
            continue
        parts.append(make_create_block(b, lines, file_mode(p)))

    parts.append("# [C] Overriding creations (may conflict on apply)\n")
    for b in override_creates:
        p = root / b
        try:
            lines = p.read_text().splitlines(keepends=True)
        except UnicodeDecodeError:
            print(f"skip (binary): {b}", file=sys.stderr)
            continue
        parts.append(make_create_block(b, lines, file_mode(p)))

    delta_out.write_text("".join(parts))
    return len(deletes), len(safe_creates), len(override_creates), len(skipped)


def validate_delta(
    entries: list[dict],
    delta_out: Path,
) -> tuple[bool, str]:
    """Simulate `base + workstate.patch` for the deletable files and confirm
    that `git apply --check delta.patch` succeeds against that state.

    We can only reproduce workstate.patch's CREATE side, so validation only
    covers section [A] (deletes) plus sections [B] and [C] in the case where
    the modify-only targets happen to be absent. It confirms delta.patch is a
    well-formed unified diff.
    """
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        env = {**os.environ, "GIT_CONFIG_GLOBAL": "/dev/null", "GIT_CONFIG_SYSTEM": "/dev/null"}
        subprocess.check_call(
            ["git", "-C", str(td), "init", "-q", "-b", "main"], env=env
        )
        subprocess.check_call(
            ["git", "-C", str(td), "config", "user.email", "test@example"], env=env
        )
        subprocess.check_call(
            ["git", "-C", str(td), "config", "user.name", "test"], env=env
        )

        for e in entries:
            if not e["is_new"]:
                continue
            f = td / e["b_path"]
            f.parent.mkdir(parents=True, exist_ok=True)
            f.write_text("".join(e["added_lines"]))
            if e["mode"] == "100755":
                f.chmod(0o755)

        subprocess.check_call(["git", "-C", str(td), "add", "-A"], env=env)
        subprocess.check_call(
            ["git", "-C", str(td), "commit", "-q", "-m", "workstate.patch state"],
            env=env,
        )

        result = subprocess.run(
            ["git", "-C", str(td), "apply", "--check", str(delta_out)],
            capture_output=True,
            text=True,
            env=env,
        )
        return result.returncode == 0, (result.stderr or result.stdout)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--patch",
        default="workstate.patch",
        help="path to the reference patch (default: workstate.patch)",
    )
    ap.add_argument(
        "--context",
        type=int,
        default=3,
        help="lines of unified-diff context (default: 3)",
    )
    ap.add_argument(
        "--no-color",
        action="store_true",
        help="disable ANSI color even when stdout is a tty",
    )
    ap.add_argument(
        "--save",
        metavar="PATH",
        help=(
            "write a delta patch to PATH instead of printing the human report. "
            "Intended to be applied after workstate.patch: "
            "'git apply workstate.patch && git apply PATH'."
        ),
    )
    ap.add_argument(
        "--no-validate",
        action="store_true",
        help="skip 'git apply --check' validation of the generated delta",
    )
    args = ap.parse_args()

    use_color = sys.stdout.isatty() and not args.no_color

    root = repo_root()
    patch_path = (root / args.patch).resolve()
    if not patch_path.is_file():
        print(f"error: {patch_path} not found", file=sys.stderr)
        return 1

    entries = list(parse_patch(patch_path.read_text()))

    if args.save:
        delta_out = (root / args.save).resolve()
        delta_out.parent.mkdir(parents=True, exist_ok=True)
        n_del, n_new, n_over, n_skip = write_delta(entries, root, delta_out)
        print(f"wrote {delta_out.relative_to(root)}:")
        print(f"  [A] deletions             : {n_del}")
        print(f"  [B] fresh creations       : {n_new}")
        print(f"  [C] overriding creations  : {n_over}  (may conflict; see header)")
        print(f"  skipped modify-only paths : {n_skip}")
        if args.no_validate:
            return 0
        ok, msg = validate_delta(entries, delta_out)
        if ok:
            print("validation: git apply --check succeeded (against synthetic base)")
            return 0
        print("validation: git apply --check FAILED:", file=sys.stderr)
        print(msg, file=sys.stderr)
        return 2

    created = [e for e in entries if e["is_new"]]
    modified = [e for e in entries if not e["is_new"]]

    patch_only: list[str] = []
    common: list[tuple[str, list[str], list[str]]] = []

    for e in created:
        b = e["b_path"]
        current = root / b
        patch_lines = e["added_lines"]
        if not current.exists():
            patch_only.append(b)
            continue
        current_lines = current.read_text().splitlines(keepends=True)
        common.append((b, patch_lines, current_lines))

    bold = lambda s: color(s, "1", use_color)
    yellow = lambda s: color(s, "33", use_color)
    cyan = lambda s: color(s, "36", use_color)
    dim = lambda s: color(s, "2", use_color)

    def write_diff_line(line: str) -> None:
        if line.startswith("+") and not line.startswith("+++"):
            sys.stdout.write(color(line, "32", use_color))
        elif line.startswith("-") and not line.startswith("---"):
            sys.stdout.write(color(line, "31", use_color))
        elif line.startswith("@@"):
            sys.stdout.write(color(line, "36", use_color))
        else:
            sys.stdout.write(line)

    print(bold(f"Comparing current repo ({root}) against {patch_path.name}"))
    print()

    print(bold(f"[1] Patch-only paths ({len(patch_only)}) — added by patch, absent here:"))
    for p in patch_only:
        print(f"    {yellow('-')} {p}")
    if not patch_only:
        print(dim("    (none)"))
    print()

    print(bold(f"[2] Common paths ({len(common)}) — diffs, patch state → current repo:"))
    if not common:
        print(dim("    (none)"))
    for b, patch_lines, current_lines in common:
        print()
        print(cyan(f"    === {b} ==="))
        diff = difflib.unified_diff(
            patch_lines,
            current_lines,
            fromfile=f"a/{b}  (from patch)",
            tofile=f"b/{b}  (current repo)",
            n=args.context,
        )
        wrote_any = False
        for line in diff:
            wrote_any = True
            write_diff_line(line)
        if not wrote_any:
            print(dim("    (identical)"))
    print()

    print(
        bold(
            f"[3] Modify-only paths ({len(modified)}) — patch tweaks these; raw hunks "
            "shown verbatim (no pre-image available in this tree):"
        )
    )
    if not modified:
        print(dim("    (none)"))
    for e in modified:
        b = e["b_path"]
        current = root / b
        marker = "present" if current.exists() else "absent"
        print()
        print(cyan(f"    === {b}  (current repo: {marker}) ==="))
        for line in e["raw_block"].splitlines(keepends=True):
            write_diff_line(line)
    print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
