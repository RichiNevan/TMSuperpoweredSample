cmake_minimum_required(VERSION 3.22)
project(native-specs)

add_library(
native-specs 
SHARED
  
../../cpp/OnLoad.cpp 
../../cpp/MySuperpoweredModule.cpp 
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../node_modules/react-native/ReactAndroid/cmake-utils/ReactNative-application.cmake)

include_directories("../../cpp")

find_library(log-lib log)

target_link_libraries(
native-specs
${log-lib}
)