cmake_minimum_required(VERSION 3.20)
project(SmartPointers CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Загрузка Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 2. Создание исполняемого файла (СТРОГО ДО target_include_directories)
add_executable(SmartPointers
        tests/SharedPtr_tests.cpp
        tests/UniquePtr_tests.cpp
)

# 3. Пути к заголовкам
target_include_directories(SmartPointers PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/LAB2
)

# 4. Линковка Google Test с готовым main()
target_link_libraries(SmartPointers PRIVATE
        GTest::gtest_main
)

# 5. Регистрация тестов
include(GoogleTest)
gtest_discover_tests(SmartPointers)