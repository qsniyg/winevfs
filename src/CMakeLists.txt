set(BASE_SOURCES
  vfs.cpp
  functions.cpp
  )

add_library(winevfs_lib64 SHARED ${BASE_SOURCES})
target_link_libraries(winevfs_lib64 stdc++fs dl)

set(SOURCES
  ${BASE_SOURCES}
  main.cpp
  )

add_executable(winevfs ${SOURCES})
target_link_libraries(winevfs stdc++fs dl)
