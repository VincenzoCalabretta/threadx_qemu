{
  description = "Dev environment for NYX development on Linux x86 machines";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    # The TRACE32 installer directory.
    # One-time setup on a new machine:
    #   sudo mv /path/to/untarred-trace32-installer /opt/trace32-installer
    # The directory must stay in place as every `nix develop` re-hashes it.
    # To override for a different location:
    #   nix develop --override-input trace32-installer path:/absolute/path/to/installer
    trace32-installer = {
      url = "path:/opt/trace32-installer";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, trace32-installer }:
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };

        trace32 = pkgs.callPackage ./tools/t32/trace32.nix {
          src = trace32-installer;
        };

        # Render <target-dir>/config.t32 from its template and invoke t32marm.
        # Accepts either:
        #   t32-run <target-dir>              - for manual use
        #   t32-run <elf-path> <target-dir>   - for Bazel integration
        # The render step ensures the LB script points to the nix-managed LB binary.
        # When an ELF path is provided, it's passed via T32_ELF_PATH env var.
        t32-run = pkgs.writeShellApplication {
          name = "t32-run";
          runtimeInputs = [ trace32 pkgs.gnused pkgs.coreutils ];
          text = ''
            if [ $# -lt 1 ]; then
              echo "Usage: t32-run <target-dir>" >&2
              echo "   or: t32-run <elf-path> <target-dir>" >&2
              echo "" >&2
              echo "  <target-dir> must contain config.t32.template and load.cmm" >&2
              echo "  <elf-path> is passed to load.cmm via T32_ELF_PATH env var" >&2
              exit 1
            fi

            # Detect if first arg is an ELF file or a directory
            if [ -f "$1" ] && [ $# -ge 2 ]; then
              # Two-argument form: <elf-path> <target-dir>
              elf_path="$(realpath "$1")"
              target_dir="$2"
              export T32_ELF_PATH="$elf_path"
              echo "T32: Loading ELF from $elf_path"
            elif [ -d "$1" ]; then
              # One-argument form: <target-dir>
              target_dir="$1"
            else
              echo "Error: Invalid arguments. First arg must be a directory or ELF file." >&2
              exit 1
            fi

            if [ ! -d "$target_dir" ]; then
              echo "t32-run: not a directory: $target_dir" >&2
              exit 1
            fi
            if [ ! -f "$target_dir/config.t32.template" ]; then
              echo "t32-run: missing config.t32.template in $target_dir" >&2
              exit 1
            fi
            if [ ! -f "$target_dir/load.cmm" ]; then
              echo "t32-run: missing load.cmm in $target_dir" >&2
              exit 1
            fi

            sed "s#@T32SYS@#${trace32}#g" \
              "$target_dir/config.t32.template" > "$target_dir/config.t32"
            exec ${trace32}/bin/t32marm \
              -c "$target_dir/config.t32" \
              -s "$target_dir/load.cmm"
          '';
        };

        bazel-wrapper = pkgs.writeScriptBin "bazel" ''
          #!/usr/bin/env bash

# --- 8 unknown base line(s) omitted (workstate.patch did not touch them) ---
              --bazelrc=$repo_root/.bazelrc.nix-shell \
              "$@"
        '';
      in {
        packages = {
          inherit trace32 t32-run;
          default = trace32;
        };

        devShells.default = pkgs.mkShell {
          packages = [
            # Python
            pkgs.python313
            pkgs.uv
            pkgs.ty

            # C++ compilation
            pkgs.gcc
            pkgs.binutils
            pkgs.zlib
            pkgs.cbmc

            # Other tools
            pkgs.clang-tools
            bazel-wrapper
            pkgs.pre-commit
            pkgs.coreutils

            # ARM embedded toolchain and debugging
            pkgs.gcc-arm-embedded # arm-none-eabi-*
            pkgs.gnumake
            pkgs.gdb # host gdb
            trace32
            t32-run
          ];

          shellHook = ''
            export T32SYS="${trace32}"

            # Render tools/t32/<target>/config.t32 for every target so SYS= points into the nix store.
            for tmpl in tools/t32/*/config.t32.template; do
              [ -f "$tmpl" ] || continue
              ${pkgs.gnused}/bin/sed "s#@T32SYS@#$T32SYS#g" "$tmpl" > "''${tmpl%.template}"
            done

            if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
              git config --local core.commentChar ';' >/dev/null 2>&1 || true
              pre-commit install
            fi

            cat <<EOF
            Welcome to NYX dev shell
            EOF
          '';
        };

        # t32 sanity check
        checks.trace32-exists = pkgs.runCommand "trace32-exists" { } ''
          test -x ${trace32}/bin/t32marm
          test -f ${trace32}/config.t32
          touch $out
        '';
      });
}
