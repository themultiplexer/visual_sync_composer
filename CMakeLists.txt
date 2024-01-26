cmake_minimum_required(VERSION 3.5)

project(VisualSyncComposer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)
find_package(sdbus-c++ REQUIRED)
find_package(Boost 1.55 COMPONENTS serialization system)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

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
            ${PROJECT_SOURCES}
            thread.h
            downloader.h

            Timeline/GraphicsScene.cpp Timeline/GraphicsScene.h Timeline/GraphicsView.cpp Timeline/GraphicsView.h Timeline/Indicator.cpp Timeline/Indicator.h             Timeline/Track.cpp Timeline/Track.h
            Timeline/Timeline.h
            Timeline/Timeline.cpp
            Timeline/ColorLabel.h
            Timeline/AlignedTextItem.h Timeline/AlignedTextItem.cpp

            ProjectModel.h
            EventModel.h


            Timeline/TrackGroup.h
            Timeline/TrackGroup.cpp
            TODO.md
            EventModel.cpp

        )
    endif()
endif()

target_link_libraries(VisualSyncComposer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network SDBusCpp::sdbus-c++ ${Boost_LIBRARIES})

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
