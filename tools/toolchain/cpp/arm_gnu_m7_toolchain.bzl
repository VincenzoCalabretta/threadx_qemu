"""(c) The Exploration Company.

Downloads and configures the Arm GNU bare-metal toolchain for the Cortex-M7 target.
Procures the binary from arm published binaries as a function of the
host architecture.
"""

load("//tools/toolchain/cpp:probe.bzl", "probe_builtin_include_dirs")

_ARCH_TO_ARM_HOST = {
    "aarch64": "aarch64",
    "amd64": "x86_64",
    "arm64": "aarch64",
    "x86_64": "x86_64",
}

_ARCH_TO_BAZEL_CPU = {
    "aarch64": "aarch64",
    "amd64": "x86_64",
    "arm64": "aarch64",
    "x86_64": "x86_64",
}

_M7_PROBE_FLAGS = [
    "-mcpu=cortex-m7",
    "-mthumb",
    "-mfpu=fpv5-d16",
    "-mfloat-abi=hard",
]

def _download_arm_gnu_m7_toolchain(rctx):
    host_arch = rctx.os.arch
    arm_host = _ARCH_TO_ARM_HOST.get(host_arch)
    bazel_cpu = _ARCH_TO_BAZEL_CPU.get(host_arch)
    if not arm_host:
        fail("Unsupported host architecture for arm gnu toolchain: '{}'. Supported: {}".format(
            host_arch,
            list(_ARCH_TO_ARM_HOST.keys()),
        ))

    sha256 = rctx.attr.sha256.get(arm_host)
    if not sha256:
        fail("Missing sha256 for host '{}'. Provided keys: {}".format(arm_host, list(rctx.attr.sha256.keys())))

    tarball_stem = "arm-gnu-toolchain-{version}-{host}-arm-none-eabi".format(
        version = rctx.attr.version,
        host = arm_host,
    )
    url = "{base}/{version}/binrel/{stem}.tar.xz".format(
        base = rctx.attr.base_url,
        version = rctx.attr.version,
        stem = tarball_stem,
    )

    rctx.download_and_extract(
        url = url,
        sha256 = sha256,
        stripPrefix = tarball_stem,
    )

    gcc_path = str(rctx.path("bin/arm-none-eabi-gcc"))

    ver_result = rctx.execute([gcc_path, "-dumpversion"])
    if ver_result.return_code != 0:
        fail("Failed to query arm-none-eabi-gcc version: " + ver_result.stderr)
    gcc_version = ver_result.stdout.strip()

    include_dirs = probe_builtin_include_dirs(rctx, gcc_path, probe_flags = _M7_PROBE_FLAGS)

    substitutions = {
        "{{bazel_arch}}": bazel_cpu,
        "{{gcc_version}}": gcc_version,
        "{{include_dirs}}": repr(include_dirs),
        "{{repo_path}}": str(rctx.path("")),
        "{{toolchain_name}}": rctx.attr.toolchain_name,
    }

    rctx.template(
        "BUILD.bazel",
        Label("//tools/toolchain/cpp:arm_gnu_m7_BUILD.tmpl"),
        substitutions = substitutions,
    )
    rctx.template(
        rctx.attr.toolchain_name + "_config.bzl",
        Label("//tools/toolchain/cpp:arm_gnu_m7_config.bzl.tmpl"),
        substitutions = substitutions,
    )

download_arm_gnu_m7_toolchain = repository_rule(
    implementation = _download_arm_gnu_m7_toolchain,
    attrs = {
        "base_url": attr.string(
            default = "https://developer.arm.com/-/media/Files/downloads/gnu",
            doc = "Base URL of the Arm GNU toolchain release directory.",
        ),
        "sha256": attr.string_dict(
            mandatory = True,
            doc = "SHA256 of the tarball, keyed by Arm host ('x86_64', 'aarch64').",
        ),
        "toolchain_name": attr.string(mandatory = True),
        "version": attr.string(
            mandatory = True,
            doc = "Arm GNU toolchain version, e.g. '15.2.rel1'.",
        ),
    },
)
