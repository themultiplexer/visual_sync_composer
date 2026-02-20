cmake_minimum_required(VERSION 3.20)

project(VisualSyncComposer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(MACOSX_BUNDLE_ICON_FILE show.icns)
set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/show.icns)
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")


find_package(Boost COMPONENTS serialization system beast)
find_package(kissfft REQUIRED)
find_package(RtAudio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets)

if(APPLE)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGLWidgets Network REQUIRED)
else()
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)
endif()

if(APPLE)
  pkg_check_modules(KISSFFT REQUIRED kissfft-float)
else()
  pkg_check_modules(KISSFFT REQUIRED kissfft-float)
  pkg_check_modules(AVAHI REQUIRED avahi-client)
endif()



file(GLOB PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VisualSyncComposer
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_macos}
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
        )
    endif()
endif()

add_subdirectory(src/f1_driver)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

include_directories( ${FREETYPE_INCLUDE_DIRS} ${AVAHI_INCLUDE_DIRS})

if(APPLE)
  target_link_libraries(VisualSyncComposer PRIVATE f1_driver Qt${QT_VERSION_MAJOR}::Widgets ${OPENGL_LIBRARIES} Qt${QT_VERSION_MAJOR}::Network ${FREETYPE_LIBRARIES} ${AVAHI_LIBRARIES} RtAudio::rtaudio nlohmann_json::nlohmann_json ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto Qt6::OpenGLWidgets ${KISSFFT_LIBRARIES} )
else()
  target_link_libraries(VisualSyncComposer PRIVATE f1_driver Qt${QT_VERSION_MAJOR}::Widgets ${OPENGL_LIBRARIES} Qt${QT_VERSION_MAJOR}::Network ${FREETYPE_LIBRARIES} ${AVAHI_LIBRARIES} RtAudio::rtaudio nlohmann_json::nlohmann_json ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto ${KISSFFT_LIBRARIES})
endif()


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
    qt_finalize_executable(VisualSyncComposer src/main.cpp)
endif()
