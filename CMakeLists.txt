# cmake_minimum_required(VERSION 3.10)
# project(deribit_client)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Add include directory
# include_directories(include)

# # Create executable
# add_executable(deribit_client
#     main.cpp
#     src/deribit_client.cpp
# )

# # Find and link Boost and OpenSSL
# find_package(Boost REQUIRED COMPONENTS system)
# find_package(OpenSSL REQUIRED)

# target_link_libraries(deribit_client
#     OpenSSL::SSL
#     OpenSSL::Crypto
#     Boost::system
#     pthread
# )



cmake_minimum_required(VERSION 3.16)
project(DeribitClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add your source files
set(SOURCES
    main.cpp
    src/deribit_client.cpp
    src/message_parser.cpp
)

# Include directories
include_directories(
    include
)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link required libraries
target_link_libraries(${PROJECT_NAME}
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# Use -Wall and -Wextra for warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# Show summary
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
