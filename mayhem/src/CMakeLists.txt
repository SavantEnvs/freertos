cmake_minimum_required(VERSION 3.21)
project("FreeRTOS TCP/IP Mayhem Harnesses" VERSION 1.0.0 LANGUAGES C)

# The FreeRTOS-Plus-TCP mirror root (two levels up: mayhem/src -> mayhem -> repo root).
# Upstream source is built in place and NOT modified (the integration is additive).
get_filename_component(PLUS_TCP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# freertos_config: the integration's FreeRTOS + FreeRTOS-Plus-TCP configuration headers.
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles
)

# Fuzzing-engine knobs threaded from the build environment (see mayhem/build.sh):
#   LIB_FUZZING_ENGINE  -> the libFuzzer binary's link flag (-fsanitize=fuzzer)
#   STANDALONE_FUZZ_MAIN -> a run-once driver main() for the *-standalone reproducers
separate_arguments(FUZZ_ENGINE_FLAGS UNIX_COMMAND "$ENV{LIB_FUZZING_ENGINE}")
if(NOT FUZZ_ENGINE_FLAGS)
    set(FUZZ_ENGINE_FLAGS -fsanitize=fuzzer)
endif()
set(STANDALONE_FUZZ_MAIN "$ENV{STANDALONE_FUZZ_MAIN}")
if(NOT STANDALONE_FUZZ_MAIN)
    set(STANDALONE_FUZZ_MAIN /opt/mayhem/StandaloneFuzzTargetMain.c)
endif()

# Custom FreeRTOS kernel port (provides freertos_kernel_port for A_CUSTOM_PORT).
add_subdirectory(port)

set(FREERTOS_PORT A_CUSTOM_PORT)
set(FREERTOS_PLUS_TCP_NETWORK_IF "LOOPBACK" CACHE STRING "" FORCE)
# Defaults normally supplied by the upstream top-level CMakeLists (not used here — we
# add_subdirectory(source) directly, so its defaulting logic never runs).
set(FREERTOS_PLUS_TCP_BUFFER_ALLOCATION "2" CACHE STRING "" FORCE)
set(FREERTOS_PLUS_TCP_COMPILER "" CACHE STRING "" FORCE)

# Vendored FreeRTOS-Kernel (pinned) — defines freertos_kernel + freertos_kernel_include.
add_subdirectory(freertos_kernel)

# Upstream FreeRTOS-Plus-TCP, built from the mirror root without modification.
add_subdirectory(${PLUS_TCP_ROOT}/tools  ${CMAKE_BINARY_DIR}/ptcp_tools)
add_subdirectory(${PLUS_TCP_ROOT}/source ${CMAKE_BINARY_DIR}/ptcp_source)

# The Mayhem harness scaffolding (network adapter, callbacks, heap).
add_subdirectory(middleware)

# Each harness becomes two binaries at /mayhem/:
#   <name>              in-process libFuzzer binary (the Mayhem target)
#   <name>-standalone   run-once reproducer over the same LLVMFuzzerTestOneInput
function(create_harness name)
  add_executable(${name} harnesses/${name}/harness.c fuzz/fuzz_entry.c)
  target_link_libraries(${name} PRIVATE middleware)
  target_compile_options(${name} PRIVATE ${FUZZ_ENGINE_FLAGS})
  target_link_options(${name} PRIVATE ${FUZZ_ENGINE_FLAGS})
  set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY /mayhem
    OUTPUT_NAME "${name}")

  add_executable(${name}-standalone harnesses/${name}/harness.c fuzz/fuzz_entry.c "${STANDALONE_FUZZ_MAIN}")
  target_link_libraries(${name}-standalone PRIVATE middleware)
  set_target_properties(${name}-standalone PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY /mayhem
    OUTPUT_NAME "${name}-standalone")
endfunction()

create_harness(icmp)
create_harness(arp)
create_harness(udp)
create_harness(tcp)
create_harness(dns)
create_harness(netbios)
create_harness(dns-handle-packet)
create_harness(nd-icmp-v6)
create_harness(nd-check-resolution)
create_harness(full-stack)
