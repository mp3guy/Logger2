cmake_minimum_required(VERSION 2.8)
find_package(ZLIB REQUIRED)
find_package(Qt4 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost COMPONENTS date_time REQUIRED)

include(${QT_USE_FILE})
 
qt4_wrap_cpp(main_moc_SRCS
             main.h)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/Include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ALL_DYN_LINK ON)   # force dynamic linking for all libraries

set(CMAKE_CXX_FLAGS "-std=gnu++0x -O3 -msse2 -msse3")

if(WITH_REALSENSE)
    include_directories(${REALSENSE_INCLUDE_DIR})
    add_definitions(-DWITH_REALSENSE)
    set(EXTRA_LIBS ${EXTRA_LIBS} /usr/local/lib/librealsense.so)
    message(STATUS "EXTRA_LIBS: " ${EXTRA_LIBS})
endif()

#Magic hack to check if host is 32 or 64 bit
if(APPLE)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2.dylib ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.dylib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFile.dylib ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.dylib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080.dylib ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.dylib COPYONLY)
elseif(UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2x64.so ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libDummyDevicex64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libDummyDevice.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFilex64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080x64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.so COPYONLY)
    else()
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2x86.so ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libDummyDevicex86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libDummyDevice.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFilex86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080x86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.so COPYONLY)
    endif()
endif()

set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,. -L${CMAKE_CURRENT_BINARY_DIR}")

set(srcs main.cpp
         Logger2.cpp
         Communicator.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/OpenNI2Interface.cpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/RealSenseInterface.cpp)

add_executable(Logger2 ${main_moc_SRCS} ${srcs})

message(STATUS "OpenCV_LIBS = " ${OpenCV_LIBS})

target_link_libraries(Logger2                       
                      ${ZLIB_LIBRARY}
                      ${Boost_LIBRARIES}
                      ${OpenCV_LIBS} 
		      ${EXTRA_LIBS}
                      ${QT_LIBRARIES}
                      boost_system
                      boost_filesystem
                      boost_thread
                      OpenNI2)

