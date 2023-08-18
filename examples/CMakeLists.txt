if (NTRIP_BUILD_CASTER)
  add_executable(ntrip_caster_exam ntrip_caster_exam.cc)
  add_dependencies(ntrip_caster_exam ntrip)
  target_link_libraries(ntrip_caster_exam ntrip)
endif (NTRIP_BUILD_CASTER)

add_executable(ntrip_client_exam ntrip_client_exam.cc)
add_dependencies(ntrip_client_exam ntrip)
target_link_libraries(ntrip_client_exam ntrip)

if (NTRIP_BUILD_SERVER)
  add_executable(ntrip_server_exam ntrip_server_exam.cc)
  add_dependencies(ntrip_server_exam ntrip)
  target_link_libraries(ntrip_server_exam ntrip)

  add_executable(ntrip_client_to_ntrip_server_exam ntrip_client_to_ntrip_server_exam.cc)
  add_dependencies(ntrip_client_to_ntrip_server_exam ntrip)
  target_link_libraries(ntrip_client_to_ntrip_server_exam ntrip)
endif (NTRIP_BUILD_SERVER)
