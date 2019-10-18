add_executable(ntrip_client
  ntrip_client.cc
)
add_dependencies(ntrip_client ntrip)
target_link_libraries(ntrip_client
  ntrip
  pthread
)
