set(BASE_SOURCES
  vfs.cpp
  functions.cpp
  )

add_library(winevfs_lib64 SHARED ${BASE_SOURCES})
target_link_libraries(winevfs_lib64 stdc++fs dl)
set_target_properties(winevfs_lib64 PROPERTIES OUTPUT_NAME "winevfs_lib")
set_target_properties(winevfs_lib64 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

add_library(winevfs_lib32 SHARED ${BASE_SOURCES})
target_link_libraries(winevfs_lib32 stdc++fs dl)
set_target_properties(winevfs_lib32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(winevfs_lib32 PROPERTIES OUTPUT_NAME "winevfs_lib")
set_target_properties(winevfs_lib32 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib32")

set(SOURCES
  ${BASE_SOURCES}
  main.cpp
  )

add_executable(winevfs ${SOURCES})
target_link_libraries(winevfs stdc++fs dl)
set_target_properties(winevfs PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
