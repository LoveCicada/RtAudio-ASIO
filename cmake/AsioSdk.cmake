# Validates Steinberg ASIO SDK layout and exposes include dirs + sources for RtAudio ASIO build.

if(NOT RTAUDIO_ASIO_SDK_ROOT)
    set(RTAUDIO_ASIO_SDK_ROOT "${CMAKE_SOURCE_DIR}/third_party/ASIO" CACHE PATH "Steinberg ASIO SDK root directory")
endif()

set(ASIO_SDK_COMMON_DIR "${RTAUDIO_ASIO_SDK_ROOT}/common")
set(ASIO_SDK_HOST_DIR "${RTAUDIO_ASIO_SDK_ROOT}/host")
set(ASIO_SDK_HOST_PC_DIR "${RTAUDIO_ASIO_SDK_ROOT}/host/pc")

set(ASIO_SDK_SOURCES
    "${ASIO_SDK_COMMON_DIR}/asio.cpp"
    "${ASIO_SDK_HOST_DIR}/asiodrivers.cpp"
    "${ASIO_SDK_HOST_PC_DIR}/asiolist.cpp"
)

foreach(_asio_file IN LISTS ASIO_SDK_SOURCES)
    if(NOT EXISTS "${_asio_file}")
        message(FATAL_ERROR "ASIO SDK file not found: ${_asio_file}\nRun scripts/setup-third-party.ps1 or set -DRTAUDIO_ASIO_SDK_ROOT=...")
    endif()
endforeach()

set(ASIO_SDK_INCLUDE_DIRS
    "${ASIO_SDK_COMMON_DIR}"
    "${ASIO_SDK_HOST_DIR}"
    "${ASIO_SDK_HOST_PC_DIR}"
)
