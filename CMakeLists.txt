cmake_minimum_required(VERSION 3.16)

project(ClassAssistant VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Qml Quick)

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
    tempdata.h
)

add_subdirectory("fluntqt")

target_link_libraries(ClassAssistant PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Quick)
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
        target_link_libraries(ClassAssistant PRIVATE ${ONNXRUNTIME_LIB})
        target_include_directories(ClassAssistant PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
    else()
        message(WARNING "⚠️ onnxruntime library not found, please ensure it's installed")
    endif()
endif()

set(CPPJIEGBA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppjieba)
if(NOT EXISTS ${CPPJIEGBA_DIR}/include/cppjieba/Jieba.hpp)
    message(FATAL_ERROR "cppjieba not found. Please run: git submodule update --init --recursive")
endif()

# ========== vosk ==========
# 包含vosk头文件
include_directories(vosk)

if(WIN32)
    # Windows平台配置
    # 链接vosk库 (使用libvosk.dll.a或vosk.lib)
    if(EXISTS ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.dll)
        target_link_libraries(ClassAssistant PRIVATE ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.dll)
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.lib)
        target_link_libraries(ClassAssistant PRIVATE ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.lib)
    else()
        message(WARNING "⚠️ Vosk library not found in ./vosk/win/")
    endif()
    
    # 复制dll到输出目录
    add_custom_command(TARGET ClassAssistant POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.dll
            $<TARGET_FILE_DIR:ClassAssistant>/libvosk.dll
        COMMENT "Copying Vosk DLL to output directory"
    )
    
    # 如果存在其他依赖dll，也可以一并复制
    if(EXISTS ${CMAKE_SOURCE_DIR}/vosk/win/vosk_deps)
        add_custom_command(TARGET ClassAssistant POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/vosk/win/vosk_deps
                $<TARGET_FILE_DIR:ClassAssistant>
            COMMENT "Copying Vosk dependency DLLs"
        )
    endif()
    
elseif(APPLE)
    # macOS平台配置 (原有配置)
    target_link_libraries(ClassAssistant PRIVATE ${CMAKE_SOURCE_DIR}/vosk/mac/libvosk.dylib)
    add_custom_command(TARGET ClassAssistant POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/vosk/mac/libvosk.dylib
            $<TARGET_FILE_DIR:ClassAssistant>/libvosk.dylib
        COMMENT "Copying Vosk library to app bundle"
    )
else()
    # Linux或其他平台配置
    # 假设使用系统安装的vosk或特定路径
    find_library(VOSK_LIB vosk)
    if(VOSK_LIB)
        target_link_libraries(ClassAssistant PRIVATE ${VOSK_LIB})
        message(STATUS "Found Vosk library: ${VOSK_LIB}")
    else()
        message(WARNING "⚠️ Vosk library not found for this platform")
    endif()
endif()

# ========== jieba ==========
include_directories(${CPPJIEGBA_DIR}/include)

include(GNUInstallDirs)
install(TARGETS ClassAssistant
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Windows平台安装额外的dll
if(WIN32)
    install(FILES ${CMAKE_SOURCE_DIR}/vosk/win/libvosk.dll
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        OPTIONAL
    )
endif()