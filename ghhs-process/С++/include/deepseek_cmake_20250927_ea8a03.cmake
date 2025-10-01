cmake_minimum_required(VERSION 3.16)
project(GHHS-Process)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts)

qt_standard_project_setup()

qt_add_executable(GHHS-Process
    src/main.cpp
    src/MainWindow.cpp
    src/ProcessManager.cpp
    src/SystemMonitor.cpp
)

target_link_libraries(GHHS-Process Qt6::Core Qt6::Widgets Qt6::Charts)

if(WIN32)
    target_link_libraries(GHHS-Process psapi advapi32)
endif()