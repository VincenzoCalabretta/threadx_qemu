# Lauterbach TRACE32 udev rules

Verbatim copies of the udev rules that ship with the TRACE32 installer, taken
from `<installer>/files/bin/pc_linux64/udev.conf/kernel_starting_4.14/`. Both
files are provided so you can grant a non-root user access to Lauterbach
hardware without running the full `setup_udevrules.sh`.

- `10-lauterbach.rules` — Lauterbach USB devices (PowerDebug, uTrace, ...);
  matches vendor `0x0897` and creates `/dev/lauterbach/...` symlinks. Grants
  `MODE=0666`.
- `10-lauterbach_target.rules` — USB debug target devices (Intel Debug
  Consoles, various xHCI-side targets, etc.). Also `MODE=0666`.

The `TEST==<path_to>/t32usb_id` line in `10-lauterbach.rules` intentionally
references an unresolved path. The `TEST==` guards the following
`IMPORT{program}=`, so the placeholder is harmless — the persistent
`by-id` symlink is simply not populated. Only relevant if you rely on
per-device rules keyed by serial.

## Manual installation (any distro)

```
sudo install -m 0644 10-lauterbach.rules        /etc/udev/rules.d/
sudo install -m 0644 10-lauterbach_target.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger --subsystem-match=usb
```

Then unplug/replug the debugger.

## NixOS system flake integration

`services.udev.packages` expects packages that install rules under
`$out/lib/udev/rules.d/`. Point it at this directory from your system flake.

### Option A — reference the rules from this repo (recommended)

In the system flake (`flake.nix` for your NixOS configuration), add this repo
as an input:

```nix
inputs.threadx-qemu-nix = {
  url = "path:/absolute/path/to/threadx_qemu_nix"; # or git+ssh://...
  flake = false;
};
```

Then, in the NixOS module, build a tiny package that stages the two files
into the layout udev expects and hand it to `services.udev.packages`:

```nix
{ pkgs, inputs, ... }:

let
  lauterbachUdev = pkgs.runCommand "lauterbach-udev-rules" { } ''
    install -Dm0644 \
      ${inputs.threadx-qemu-nix}/tools/t32/udev/10-lauterbach.rules \
      $out/lib/udev/rules.d/10-lauterbach.rules
    install -Dm0644 \
      ${inputs.threadx-qemu-nix}/tools/t32/udev/10-lauterbach_target.rules \
      $out/lib/udev/rules.d/10-lauterbach_target.rules
  '';
in {
  services.udev.packages = [ lauterbachUdev ];
}
```

### Option B — inline the rules with `extraRules`

If you'd rather not add another flake input, paste the contents of both
`.rules` files into `services.udev.extraRules`:

```nix
services.udev.extraRules = ''
  ${builtins.readFile ./10-lauterbach.rules}
  ${builtins.readFile ./10-lauterbach_target.rules}
'';
```

This writes them into `/etc/udev/rules.d/99-local.rules` at priority 99, which
still fires before the default USB permission rules — order doesn't matter for
these because they only tighten `MODE`.

### Locking access to a group instead of world-writable

The rules ship with `MODE:="0666"`. To restrict access to a `plugdev` group,
either edit the copies in this repo before installing, or add an override in
the NixOS module:

```nix
users.groups.plugdev = { };
users.users.<you>.extraGroups = [ "plugdev" ];

services.udev.extraRules = ''
  SUBSYSTEM=="usb", ATTR{idVendor}=="0897", MODE="0660", GROUP="plugdev"
'';
```

After `nixos-rebuild switch`, unplug/replug the debugger — the rules are
applied on device attach.
