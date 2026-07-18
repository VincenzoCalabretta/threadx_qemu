{
  description = "Cortex-M7 development: bare-metal hello world on QEMU (mps2-an500) + optional Lauterbach TRACE32 debugging on rdb3 hardware";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    # The TRACE32 installer directory.
    # One-time setup on a new machine:
    #   sudo mv /path/to/untarred-trace32-installer /opt/trace32-installer
    # The directory must stay in place as every `nix develop .#t32` re-hashes it.
    # To override for a different location:
    #   nix develop .#t32 --override-input trace32-installer path:/absolute/path
    trace32-installer = {
      url = "path:/opt/trace32-installer";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, trace32-installer }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };

        qemu = pkgs.qemu;
        bazel = pkgs.bazel_7;

        trace32 = pkgs.callPackage ./tools/t32/trace32.nix {
          src = trace32-installer;
        };

        # Render <target-dir>/config.t32 from its template and invoke t32marm.
        # Accepts either:
        #   t32-run <target-dir>              - for manual use
        #   t32-run <elf-path> <target-dir>   - for Bazel integration
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

            if [ -f "$1" ] && [ $# -ge 2 ]; then
              elf_path="$(realpath "$1")"
              target_dir="$2"
              export T32_ELF_PATH="$elf_path"
              echo "T32: Loading ELF from $elf_path"
            elif [ -d "$1" ]; then
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

        # Bazel wrapper shim. Currently just forwards to the nix-store bazel;
        # kept in place so we have a hook for future repo-root-relative flags
        # (bazelrc injection, env passthroughs, etc.) without touching every
        # user's shell.
        bazel-wrapper = pkgs.writeShellScriptBin "bazel" ''
          set -eu
          exec ${bazel}/bin/bazel "$@"
        '';

        commonPackages = [
          bazel-wrapper
          qemu
          pkgs.coreutils
          pkgs.gnused
          pkgs.gnugrep
          pkgs.findutils
          pkgs.git
          pkgs.python3
        ];

        commonEnv = ''
          export NIX_QEMU_BIN="${qemu}/bin/qemu-system-arm"
        '';
      in {
        packages = {
          inherit trace32 t32-run;
        };

        devShells = {
          # Default: QEMU-first workflow. Does NOT pull in trace32, so
          # `nix develop` works out of the box without a real T32 installer.
          default = pkgs.mkShell {
            packages = commonPackages;
            shellHook = ''
              ${commonEnv}
              echo "threadx_qemu_nix dev shell (QEMU)"
              echo "  qemu-system-arm   : $NIX_QEMU_BIN"
              echo "  bazel             : $(command -v bazel)"
              echo ""
              echo "  For TRACE32 support: nix develop .#t32 \\"
              echo "                       --override-input trace32-installer path:/opt/trace32-installer"
            '';
          };

          # T32 shell: adds Lauterbach TRACE32 + the t32-run wrapper. Requires
          # a real installer at the trace32-installer input (override it).
          t32 = pkgs.mkShell {
            packages = commonPackages ++ [ trace32 t32-run ];
            shellHook = ''
              ${commonEnv}
              export T32SYS="${trace32}"

              # Render tools/t32/<target>/config.t32 for every target so SYS=
              # points into the nix store.
              for tmpl in tools/t32/*/config.t32.template; do
                [ -f "$tmpl" ] || continue
                ${pkgs.gnused}/bin/sed "s#@T32SYS@#$T32SYS#g" "$tmpl" > "''${tmpl%.template}"
              done

              echo "threadx_qemu_nix dev shell (QEMU + TRACE32)"
              echo "  qemu-system-arm   : $NIX_QEMU_BIN"
              echo "  trace32           : $T32SYS"
              echo "  bazel             : $(command -v bazel)"
            '';
          };
        };

        # `nix flake check` sanity: qemu is available. The trace32-exists
        # check only passes when trace32-installer is a real T32 installer
        # (i.e. after the input override).
        checks = {
          qemu-exists = pkgs.runCommand "qemu-exists" { } ''
            test -x ${qemu}/bin/qemu-system-arm
            touch $out
          '';
          trace32-exists = pkgs.runCommand "trace32-exists" { } ''
            test -x ${trace32}/bin/t32marm
            test -f ${trace32}/config.t32
            touch $out
          '';
        };
      });
}
