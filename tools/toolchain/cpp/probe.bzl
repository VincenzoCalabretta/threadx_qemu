# (c) The Exploration Company

"""Probe a C/C++ compiler for its built-in include search paths.

Referenced from rules_cc/cc/private/toolchain/unix_cc_configure.bzl
where a similar implementation is used to auto-configure the host toolchain.
The returned paths feed cxx_builtin_include_directories on
create_cc_toolchain_config_info.
"""

_START_MARKER = "#include <...> search starts here:"
_END_MARKER = "End of search list."

def probe_builtin_include_dirs(rctx, cc_path, probe_flags = [], language = "c++"):
    """Return the built-in include directories reported by cc_path.

    Multilib toolchains report different paths depending on configuraiton flags,
    so the passed probe_flags are used to call the tool provided by cc_path
    with the expected ones flags.

    Args:
      rctx: Repository rule context.
      cc_path: Absolute path to the compiler.
      probe_flags: Extra flags to pass on to the probe command line.
      language: "c" or "c++". Selects -xc vs -xc++. Defaults to
        "c++", which returns the union of C and C++ search paths.

    Returns:
      List of absolute include directory paths in the order the compiler
      reports.

    Fails if the compiler exits non-zero or if the search-path markers are
    absent from stderr.
    """
    if language not in ("c", "c++"):
        fail("probe_builtin_include_dirs: language must be 'c' or 'c++', got '{}'".format(language))

    probe = "_cc_probe_input." + ("cc" if language == "c++" else "c")
    rctx.file(probe, "")

    args = [cc_path] + list(probe_flags) + ["-x" + language, "-E", "-v", probe]
    result = rctx.execute(args)
    if result.return_code != 0:
        fail("probe_builtin_include_dirs: '{}' failed:\n{}".format(" ".join(args), result.stderr))

    dirs = []
    inside = False
    for line in result.stderr.splitlines():
        if _START_MARKER in line:
            inside = True
            continue
        if inside and _END_MARKER in line:
            break
        if inside:
            d = line.strip()
            if d:
                dirs.append(d)

    if not dirs:
        fail("probe_builtin_include_dirs: no include dirs parsed from output of:\n{}\nstderr:\n{}".format(
            " ".join(args),
            result.stderr,
        ))

    return dirs
