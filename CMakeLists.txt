cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-lssl -lcrypto -lsqlite3")
project(Server)

find_package(Boost 1.67.0 COMPONENTS system filesystem REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(OpenSSL)

if(OpenSSL_FOUND)
    message(STATUS "OpenSSL_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "OpenSSL_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "OpenSSL_VERSION: ${Boost_VERSION}")

    include_directories(${OpenSSL_INCLUDE_DIRS})
endif()

find_package(Cereal)

if(Cereal_FOUND)
    message(STATUS "Cereal_INCLUDE_DIRS: ${Cereal_INCLUDE_DIRS}")
    message(STATUS "Cereal_LIBRARIES: ${Cereal_LIBRARIES}")
    message(STATUS "Cereal_VERSION: ${Cereal_VERSION}")

    include_directories(${Cereal_INCLUDE_DIRS})
endif()

add_executable(Server server/main.cpp server/Session.cpp server/Session.h server/SessionManager.cpp server/SessionManager.h server/Server.cpp server/Server.h server/BlogManager.cpp server/BlogManager.h server/CSRFTokenManager.cpp server/CSRFTokenManager.h server/CredentialsManager.cpp server/CredentialsManager.h server/Credentials.cpp server/Credentials.h server/Blog.cpp server/Blog.h)

if(Boost_FOUND)
    target_link_libraries(Server ${Boost_LIBRARIES})
endif()
if(Cereal_FOUND)
    target_link_libraries(Server ${Cereal_LIBRARIES})
endif()

if(OpenSSL_FOUND)
    target_link_libraries(Server ${OpenSSL_LIBRARIES})
endif()
