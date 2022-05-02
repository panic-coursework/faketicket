cmake_minimum_required(VERSION 3.16)
project(ticket)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g")

set(TICKET_INCLUDES
  ${ticket_SOURCE_DIR}/lib
  ${ticket_SOURCE_DIR}/src
)

set(TICKET_LIB_SOURCES
  lib/datetime.cpp
)

enable_testing()
set(TICKET_TEST_SOURCES
  lib/datetime_test.cpp
  lib/file/bptree_test.cpp
  lib/hashmap_test.cpp
  lib/map_test.cpp
  lib/result_test.cpp
  lib/variant_test.cpp
)

foreach(test ${TICKET_TEST_SOURCES})
  string(MD5 testmd5 ${test})
  string(SUBSTRING ${testmd5} 0 6 testmd5)
  get_filename_component(TName ${test} NAME_WE)
  set(testexe ${testmd5}-${TName})
  add_executable(${testexe} ${test} ${TICKET_LIB_SOURCES})
  target_include_directories(${testexe} PRIVATE ${TICKET_INCLUDES})
  add_test(NAME ${testexe} COMMAND bin/run-test ${testexe})
endforeach()
