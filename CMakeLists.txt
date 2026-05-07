cmake_minimum_required(VERSION 3.10)
project(AddressBookQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql)

add_executable(addressbook_qt
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(addressbook_qt PRIVATE Qt5::Widgets Qt5::Sql)
