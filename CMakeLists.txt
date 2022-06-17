cmake_minimum_required(VERSION 3.16)
project(ticket)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wno-invalid-offsetof")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(DEFINED INTERACTIVE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTICKET_INTERACTIVE")
endif()

set(TICKET_INCLUDES
  ${ticket_SOURCE_DIR}/lib
  ${ticket_SOURCE_DIR}/src
)
include_directories(${TICKET_INCLUDES})

set(TICKET_LIB_SOURCES
  lib/datetime.cpp
  lib/utility.cpp
)
add_library(ticketutils OBJECT ${TICKET_LIB_SOURCES})

set(TICKET_SOURCES
  src/misc.cpp
  src/order.cpp
  src/parser.cpp
  src/response.cpp
  src/rollback.cpp
  src/train.cpp
  src/user.cpp
)

if(DEFINED CMAKE_JS_INC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_NODEJS")
  include_directories(${CMAKE_JS_INC})
  add_library(ticket SHARED src/node.cpp ${TICKET_SOURCES} $<TARGET_OBJECTS:ticketutils> ${CMAKE_JS_SRC})
  execute_process(COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
  )
  string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
  string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
  target_include_directories(ticket PRIVATE ${NODE_ADDON_API_DIR})
  set_target_properties(ticket PROPERTIES PREFIX "" SUFFIX ".node")
  target_link_libraries(ticket ${CMAKE_JS_LIB})
else()
  add_executable(code ${TICKET_SOURCES} $<TARGET_OBJECTS:ticketutils> src/main.cpp)

  enable_testing()
  set(TICKET_TEST_SOURCES
    lib/algorithm_test.cpp
    lib/datetime_test.cpp
    lib/file/bptree_test.cpp
    lib/hashmap_test.cpp
    lib/lru-cache_test.cpp
    lib/map_test.cpp
    lib/result_test.cpp
    lib/utility_test.cpp
    lib/variant_test.cpp
  )

  foreach(test ${TICKET_TEST_SOURCES})
    string(MD5 testmd5 ${test})
    string(SUBSTRING ${testmd5} 0 6 testmd5)
    get_filename_component(TName ${test} NAME_WE)
    set(testexe ${TName}-${testmd5})
    add_executable(${testexe} ${test} $<TARGET_OBJECTS:ticketutils>)
    target_include_directories(${testexe} PRIVATE ${TICKET_INCLUDES})
    add_test(NAME ${testexe} COMMAND bin/run-unit-test ${testexe})
  endforeach()
endif()
