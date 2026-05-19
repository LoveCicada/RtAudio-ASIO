cmake_minimum_required(VERSION 3.16)

project(LtcAsioPlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_PREFIX_PATH)
    if(EXISTS "D:/soft/Qt5.12.12/5.12.12/msvc2017_64")
        list(APPEND CMAKE_PREFIX_PATH "D:/soft/Qt5.12.12/5.12.12/msvc2017_64")
    endif()
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Concurrent)

if(NOT RTAUDIO_ASIO_SDK_ROOT)
    set(RTAUDIO_ASIO_SDK_ROOT "${CMAKE_SOURCE_DIR}/third_party/ASIO" CACHE PATH "Steinberg ASIO SDK root")
endif()

set(RTAUDIO_API_ASIO ON CACHE BOOL "" FORCE)
set(RTAUDIO_API_WASAPI OFF CACHE BOOL "" FORCE)
set(RTAUDIO_API_DS OFF CACHE BOOL "" FORCE)
set(RTAUDIO_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(RTAUDIO_BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/rtaudio)

add_executable(LtcAsioPlayer
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/LtcWavPlayer.h
    src/LtcWavPlayer.cpp
    src/WavReader.h
    src/WavReader.cpp
)

target_include_directories(LtcAsioPlayer PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/third_party/rtaudio
)

target_compile_definitions(LtcAsioPlayer PRIVATE
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(LtcAsioPlayer PRIVATE /W4 /EHsc /utf-8 /UUNICODE /U_UNICODE)
endif()

target_link_libraries(LtcAsioPlayer PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    rtaudio
)
