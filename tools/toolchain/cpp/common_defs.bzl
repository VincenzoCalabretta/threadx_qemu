"""Action-name groupings referenced by the cc_toolchain_config templates."""

load("@rules_cc//cc:action_names.bzl", "ACTION_NAMES")

C_COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
]

CPP_COMPILE_ACTIONS = [
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.cpp_header_parsing,
]

CPP_ASSEMBLE_ACTIONS = [
    ACTION_NAMES.assemble,
    ACTION_NAMES.preprocess_assemble,
]

CPP_LINK_ACTIONS = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]
