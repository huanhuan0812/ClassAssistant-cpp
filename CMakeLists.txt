cmake_minimum_required(VERSION 3.16)

project(ClassAssistant VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(ClassAssistant
    main.cpp
    SettingsView.h
    SettingsView.cpp
    OrganizeView.h
    OrganizeView.cpp
    SearchView.h
    SearchView.cpp
    FileView.h
    FileView.cpp
    settings.h
    utils.h
    utils.cpp
    settings.cpp
    classifier.h
    fileInterface.h
    fileUnit/pptxunit.h
    fileUnit/pptxunit.cpp
    fileUnit/vocabulary_loader.h
    fileUnit/vocabulary_loader.cpp
    fileUnit/docxunit.h
    fileUnit/docxunit.cpp
    fileUnit/TextProcessor/JiebaSingleton.h
    fileUnit/TextProcessor/JiebaSingleton.cpp
    fileUnit/TextProcessor/Utils.h
    fileUnit/TextProcessor/Utils.cpp
    fileUnit/TextProcessor/TextCleaner.h
    fileUnit/TextProcessor/TextCleaner.cpp
    fileUnit/TextProcessor/TextSegmenter.h
    fileUnit/TextProcessor/TextSegmenter.cpp
    fileUnit/TextProcessor/FilenameSegmenter.h
    fileUnit/TextProcessor/FilenameSegmenter.cpp
    mainwindow.h
    mainwindow.cpp
)

add_subdirectory("fluntqt")

target_link_libraries(ClassAssistant PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ClassAssistant PRIVATE FluentQt::FluentQt)

add_subdirectory("pptx-reader")
target_link_libraries(ClassAssistant PRIVATE pptx_reader)

find_package(docxcpp CONFIG REQUIRED)
target_link_libraries(ClassAssistant PRIVATE docxcpp::docxcpp)

set_target_properties(ClassAssistant PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ========== onnxruntime ==========
find_package(onnxruntime CONFIG)
if(NOT onnxruntime_FOUND)
    message(STATUS "⚠️ onnxruntime not found via CMake, trying pkg-config...")
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(ONNXRUNTIME onnxruntime)
        message("onnxfound")
    endif()
endif()

# 链接 onnxruntime
if(onnxruntime_FOUND)
    target_link_libraries(ClassAssistant PRIVATE onnxruntime::onnxruntime)
elseif(ONNXRUNTIME_FOUND)
    target_link_libraries(ClassAssistant PRIVATE ${ONNXRUNTIME_LIBRARIES})
    target_include_directories(ClassAssistant PRIVATE ${ONNXRUNTIME_INCLUDE_DIRS})
else()
    find_library(ONNXRUNTIME_LIB onnxruntime)
    if(ONNXRUNTIME_LIB)
        target_link_libraries(ClassAssistant ${ONNXRUNTIME_LIB})
        target_include_directories(ClassAssistant PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
    else()
        message(WARNING "⚠️ onnxruntime library not found, please ensure it's installed")
    endif()
endif()

set(CPPJIEGBA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppjieba)
if(NOT EXISTS ${CPPJIEGBA_DIR}/include/cppjieba/Jieba.hpp)
    message(FATAL_ERROR "cppjieba not found. Please run: git submodule update --init --recursive")
endif()

include_directories(${CPPJIEGBA_DIR}/include)

include(GNUInstallDirs)
install(TARGETS ClassAssistant
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
