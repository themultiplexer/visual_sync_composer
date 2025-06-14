cmake_minimum_required(VERSION 3.5)

project(VisualSyncComposer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)
find_package(sdbus-c++ REQUIRED)
find_package(Boost 1.74 COMPONENTS serialization system)
find_package(kissfft REQUIRED)
find_package(RtAudio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)

pkg_check_modules(AVAHI REQUIRED avahi-client)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VisualSyncComposer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET VisualSyncComposer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(VisualSyncComposer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(VisualSyncComposer
            src/main.cpp
            src/mainwindow.cpp
            src/mainwindow.h
            src/thread.h
            src/downloader.h
            src/timeline/GraphicsScene.cpp
            src/timeline/GraphicsScene.h
            src/timeline/GraphicsView.cpp
            src/timeline/GraphicsView.h
            src/timeline/Indicator.cpp
            src/timeline/Indicator.h
            src/timeline/Track.cpp
            src/timeline/Track.h
            src/timeline/Timeline.h
            src/timeline/Timeline.cpp
            src/timeline/ColorLabel.h
            src/timeline/AlignedTextItem.h
            src/timeline/AlignedTextItem.cpp
            src/timeline/TrackGroup.h
            src/timeline/TrackGroup.cpp
            src/timeline/GridGraphicsGroup.h
            src/timeline/EventProcessor.h
            src/timeline/EventModel.h
            src/timeline/EventModel.cpp
            src/espnowsender.cpp
            src/espnowsender.h
            src/espnowtypes.cpp
            src/espnowtypes.h
            src/ProjectModel.h
            src/audioanalyzer.h
            src/audioanalyzer.cpp
            src/oglwidget.h
            src/oglwidget.cpp
            src/wifieventprocessor.h
            src/wifieventprocessor.cpp
            src/esphandler.h
            src/esphandler.cpp
            src/gltext.h
            src/gltext.cpp
            src/font_rendering.h
            src/netdevice.h
            src/netdevice.cpp
            src/vscslider.h
            src/vscslider.cpp
            src/helper.h
            src/helper.cpp
            src/audiowindow.cpp
            src/audiowindow.h
            src/effectpresetbutton.h
            src/effectpresetbutton.cpp
            src/effectpresetmodel.h
            src/effectpresetmodel.cpp
            src/espreceiver.h
            src/espreceiver.cpp
            src/vsctube.cpp
            src/vsctube.h
            src/tubewidget.cpp
            src/tubewidget.h
            src/mdnsflasher.h
            src/mdnsflasher.cpp
            src/devicereqistry.h
            src/devicereqistry.cpp
            src/frequencyregion.h
            src/frequencyregion.cpp
            src/fullscreenwindow.h
            src/fullscreenwindow.cpp
            src/horizontal_line.h
            src/horizontal_line.cpp
        )
    endif()
endif()

include_directories( ${FREETYPE_INCLUDE_DIRS} ${AVAHI_INCLUDE_DIRS} )
target_link_libraries(VisualSyncComposer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OPENGL_LIBRARIES}  Qt${QT_VERSION_MAJOR}::Network ${FREETYPE_LIBRARIES} ${AVAHI_LIBRARIES} SDBusCpp::sdbus-c++ RtAudio::rtaudio cpprestsdk::cpprest Boost::system Boost::serialization OpenSSL::SSL OpenSSL::Crypto "/usr/lib/x86_64-linux-gnu/libkissfft-float.so.131.1.0")

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VisualSyncComposer)
endif()
set_target_properties(VisualSyncComposer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS VisualSyncComposer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VisualSyncComposer)
endif()
