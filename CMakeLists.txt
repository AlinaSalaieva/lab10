cmake_minimum_required(VERSION 3.21)
project(GPSMonitorORM_V21_Lab10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_library(gps_patterns STATIC
    src/TrackPointState.cpp
    src/SpeedStrategy.cpp
    src/TrackObserver.cpp
)
target_include_directories(gps_patterns PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(gps_core STATIC
    src/SQLiteGpsObjectRepository.cpp
    src/TrackPointRepository.cpp
    src/GpsMonitoringFacade.cpp
)
target_link_libraries(gps_core PUBLIC gps_patterns sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)

add_executable(GPSMonitorORM_V21 src/main.cpp src/TestData.cpp)
target_link_libraries(GPSMonitorORM_V21 PRIVATE gps_core)

add_executable(pattern_tests tests/PatternTests.cpp)
target_link_libraries(pattern_tests PRIVATE gps_patterns)
add_executable(integration_tests tests/IntegrationTests.cpp)
target_link_libraries(integration_tests PRIVATE gps_core)

foreach(target gps_patterns gps_core GPSMonitorORM_V21 pattern_tests integration_tests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

enable_testing()
add_test(NAME PatternTests COMMAND pattern_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
