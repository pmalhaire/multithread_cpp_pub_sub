cmake_minimum_required(VERSION 3.14)
project(test_pub_sub)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)


include_directories(./include)
include_directories(./tests)
FILE(GLOB_RECURSE SRC_FILES ./src/*.c*)


add_executable(${PROJECT_NAME} ${SRC_FILES})
include(GoogleTest)

enable_testing()

target_link_libraries(${PROJECT_NAME}
                        pthread
                        gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})


