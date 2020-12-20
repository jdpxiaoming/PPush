cmake_minimum_required(VERSION 3.4.1)
#采用 C编译器
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
#编译静态库librtmp.a
add_library(
        rtmp
        STATIC
        amf.c
        hashswf.c
        log.c
        parseurl.c
        rtmp.c
)