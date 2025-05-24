file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test ${SOURCES})

set(MYSQL_CONNECTOR_CPP_INCLUDE_DIR /usr/include/mysql-cppconn/jdbc)

#cors  database  http  middleware  router  session  ssl
target_include_directories(test PUBLIC 
  ${PROJECT_SOURCE_DIR}/include/cors
  ${PROJECT_SOURCE_DIR}/include/database
  ${PROJECT_SOURCE_DIR}/include/http
  ${PROJECT_SOURCE_DIR}/include/middleware
  ${PROJECT_SOURCE_DIR}/include/router
  ${PROJECT_SOURCE_DIR}/include/session
  ${PROJECT_SOURCE_DIR}/include/ssl
  ${PROJECT_SOURCE_DIR}/include/app
  ${PROJECT_SOURCE_DIR}/include/file
  ${MYSQL_CONNECTOR_CPP_INCLUDE_DIR}
)

set(MUDUO_LIBRARY_DIR /usr/local/lib)

find_library(MUDUO_BASE_LIBRARY NAMES muduo_base PATHS ${MUDUO_LIBRARY_DIR})
find_library(MUDUO_NET_LIBRARY NAMES muduo_net PATHS ${MUDUO_LIBRARY_DIR})

find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)

target_link_libraries(test PUBLIC
  ${MUDUO_BASE_LIBRARY}
  ${MUDUO_NET_LIBRARY}
  OpenSSL::Crypto
  OpenSSL::SSL
  mysqlcppconn
)