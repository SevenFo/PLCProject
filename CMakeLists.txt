cmake_minimum_required(VERSION 3.5)

project(HKVS)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")  #设置c的编译选项


set(HKVS_LIBS_PATH D:\\programs\\HKVSlib\\lib)
set(HKVS_INCLUDE_PATH D:\\programs\\HKVSlib\\include)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${HKVS_INCLUDE_PATH})
include_directories(${OpenCV_INCLUDE_DIRS})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        camerahandler.cpp
        hkvsplayer.cpp
        preveiwwidget.cpp
        mainwindow.h
        DataType.h
        HCNetSDK.h
        DecodeCardSdk.h
        plaympeg4.h
        camerahandler.h
        mainwindow.ui
        preveiwwidget.h
        hkvsplayer.h
        WindowsPlayM4.h
        PlayM4.h
        hikvisonhandler.cpp
        hikvisonhandler.h
        hikvisonhandlercstyelfuncs.cpp
        hikvisonhandlercstyelfuncs.h
        imagesender.cpp
        imagesender.h
)

link_directories(${HKVS_LIBS_PATH})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HKVS
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(HKVS SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(HKVS
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(HKVS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(HKVS PRIVATE HCNetSDK.lib PlayCtrl.lib)
target_link_libraries(HKVS PRIVATE Qt6::Network ${OpenCV_LIBS})
#target_link_libraries(HKVS PRIVATE OpenCV_LIBS)
