set(SOURCES
  vfs.cpp
  functions.cpp
  main.cpp
  )

add_executable(winevfs ${SOURCES})
target_link_libraries(winevfs stdc++fs dl)
