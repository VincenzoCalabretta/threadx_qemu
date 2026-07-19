"""(c) The Exploration Company.

Downloads Eclipse ThreadX source from GitHub and exposes the common kernel plus
the cortex-m7/GNU port as a cc_library. Only the pieces we build are checked
here; port-selection assumes the toolchain resolution has already picked the
Cortex-M7 Arm GNU toolchain (see //tools/toolchain/cpp:arm_gnu_m7_toolchain).
"""

_BASE_URL = "https://github.com/eclipse-threadx/threadx/archive/refs/tags"

def _download_threadx(rctx):
    url = "{base}/v{version}_rel.tar.gz".format(
        base = rctx.attr.base_url,
        version = rctx.attr.version,
    )
    strip_prefix = "threadx-{version}_rel".format(version = rctx.attr.version)

    rctx.download_and_extract(
        url = url,
        sha256 = rctx.attr.sha256,
        stripPrefix = strip_prefix,
    )

    # Static BUILD file lives in-repo so edits do not require re-releasing the
    # repository rule; symlink it in as the fetched tree's root BUILD.bazel.
    rctx.symlink(
        Label("//third_party/threadx:threadx.BUILD"),
        "BUILD.bazel",
    )

download_threadx = repository_rule(
    implementation = _download_threadx,
    attrs = {
        "base_url": attr.string(
            default = _BASE_URL,
            doc = "Base URL of the Eclipse ThreadX GitHub release archive.",
        ),
        "sha256": attr.string(
            mandatory = True,
            doc = "SHA256 of the release tarball.",
        ),
        "version": attr.string(
            mandatory = True,
            doc = "ThreadX release version, e.g. '6.4.1'. The tag is '<version>_rel'.",
        ),
    },
)
