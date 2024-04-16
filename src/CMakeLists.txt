find_package(Boost REQUIRED COMPONENTS log_setup log date_time)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY})
else(Boost_FOUND)
    message(FATAL_ERROR "boost library not found")
endif(Boost_FOUND)

set(SERVER_SRC server/main.cpp)
list(APPEND SERVER_SRC server/log/GlobalLog.cpp)

add_executable(SERVER ${SERVER_SRC})
target_link_libraries(SERVER Boost::log_setup Boost::log Boost::date_time)
