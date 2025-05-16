cmake_minimum_required(VERSION 3.13)

project(appmodules)

include(${REACT_ANDROID_DIR}/cmake-utils/ReactNative-application.cmake)

add_definitions(-Wno-vla-cxx-extension)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE 
../../../../../cpp/NativeSuperpoweredModule.cpp
../../../../../cpp/SuperpoweredAndroidAudioIO.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ../../../../../cpp)

# Full path to static libs
set(SUPERPOWERED_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../../cpp)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        log
        android
        OpenSLES
        ${SUPERPOWERED_DIR}/libSuperpoweredAndroid${ANDROID_ABI}.a
)