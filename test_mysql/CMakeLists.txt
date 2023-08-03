cmake_minimum_required(VERSION 3.24)
project(test_mysql)

set(CMAKE_CXX_STANDARD 17)


include_directories(/usr/include/mysql)


add_executable(test_mysql main.cpp MySQLLibrary.h)
target_link_libraries(test_mysql mysqlclient)
