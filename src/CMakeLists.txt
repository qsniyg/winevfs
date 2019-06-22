set(SOURCES
  vfs.cpp
  main.cpp
  )

add_executable(winevfs ${SOURCES})
target_link_libraries(winevfs stdc++fs)
