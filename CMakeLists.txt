cmake_minimum_required(VERSION 3.16)
project(AddressBookQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(addressbook_qt
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(addressbook_qt PRIVATE Qt6::Widgets)
