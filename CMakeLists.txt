cmake_minimum_required(VERSION 3.22)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(PortyServer)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

if (MINGW)
    set(PLATFORM_LIBS ws2_32 wsock32)
    set(PLATFORM_DEFINITIONS _WIN32_WINDOWS=0x0601)
endif()

add_executable(PortyServer Server.cpp)
# YES, SSL support too because FUCK YOU
target_link_libraries(PortyServer PRIVATE ${PLATFORM_LIBS} argparse::argparse OpenSSL::SSL OpenSSL::Crypto Boost::boost)
target_compile_definitions(PortyServer PRIVATE ${PLATFORM_DEFINITIONS})
target_include_directories(PortyServer PRIVATE include)