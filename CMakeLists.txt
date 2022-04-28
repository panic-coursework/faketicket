cmake_minimum_required(VERSION 3.16)
project(ticket)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g")

set(TICKET_INCLUDES
  ${ticket_SOURCE_DIR}/lib
  ${ticket_SOURCE_DIR}/src
)

enable_testing()
set(TICKET_TEST_SOURCES
  lib/variant_test.cpp
  lib/result_test.cpp
  lib/map_test.cpp
  lib/hashmap_test.cpp
)

foreach(test ${TICKET_TEST_SOURCES})
  string(MD5 testmd5 ${test})
  string(SUBSTRING ${testmd5} 0 6 testmd5)
  get_filename_component(TName ${test} NAME_WE)
  set(testexe ${testmd5}-${TName})
  add_executable(${testexe} ${test})
  target_include_directories(${testexe} PRIVATE ${TICKET_INCLUDES})
  add_test(NAME ${testexe} COMMAND ./run-test ${testexe})
endforeach()
