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
find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    OpenGLWidgets
    Network
)

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


qt_add_executable(VisualSyncComposer
    MACOSX_BUNDLE
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${app_icon_macos}
)

add_subdirectory(src/f1_driver)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

include_directories( ${FREETYPE_INCLUDE_DIRS} ${AVAHI_INCLUDE_DIRS})

target_link_libraries(VisualSyncComposer PRIVATE
    f1_driver
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Network
    ${OPENGL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${AVAHI_LIBRARIES}
    ${Boost_LIBRARIES}
    ${KISSFFT_LIBRARIES}
    RtAudio::rtaudio
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)

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
