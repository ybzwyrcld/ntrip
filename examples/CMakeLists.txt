add_executable(ntrip_caster
  ntrip_caster.cc
)
add_dependencies(ntrip_caster ntrip)
target_link_libraries(ntrip_caster
  ntrip
  pthread
)

add_executable(ntrip_client
  ntrip_client.cc
)
add_dependencies(ntrip_client ntrip)
target_link_libraries(ntrip_client
  ntrip
  pthread
)

add_executable(ntrip_server
  ntrip_server.cc
)
add_dependencies(ntrip_server ntrip)
target_link_libraries(ntrip_server
  ntrip
  pthread
)
