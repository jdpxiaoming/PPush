cmake_minimum_required(VERSION 3.4.1)
# 设置编译时的源文件查找目录.
file(GLOB SOURCE
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/VideoChannel.cpp
        ${CMAKE_SOURCE_DIR}/safe_queue.h
        ${CMAKE_SOURCE_DIR}/macro.h
        )

add_library(
        native-lib
        SHARED
        ${SOURCE}
        )

# 引入子目录
add_subdirectory(librtmp)

find_library(log-lib
              log )
# 设置x264头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

target_link_libraries(
                       native-lib
                       rtmp
                       ${log-lib}
                        x264
                        faac
                      )