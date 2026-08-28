add_library(middleware OBJECT
  callbacks.c
  network.c
  ${CMAKE_SOURCE_DIR}/freertos_kernel/portable/MemMang/heap_3.c
)

target_link_libraries(middleware
  PUBLIC
  freertos_kernel
  freertos_plus_tcp
  freertos_config)

target_include_directories(middleware PUBLIC .)
