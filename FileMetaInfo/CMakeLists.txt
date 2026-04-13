cmake_minimum_required(VERSION 3.16)

project(FileMetaInfo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appFileMetaInfo
    main.cpp
    elf_manager.cpp
    elf_manager.h
)

qt_add_qml_module(appFileMetaInfo
    URI FileMetaInfo
    QML_FILES
        Main.qml
        QML_FILES SectionListView.qml
        QML_FILES EditorPanel.qml
        QML_FILES ControlButtons.qml
        QML_FILES StyledButton.qml
        QML_FILES InfoBanner.qml
        SOURCES jsonhighlighter.h jsonhighlighter.cpp
        QML_FILES HoneycombBackground.qml
        QML_FILES Scanline.qml
        QML_FILES DigitalNoise.qml
        QML_FILES ScanningSpot.qml
        SOURCES elf_types.h
        QML_FILES ElfMap.qml
        QML_FILES FileInfoPanel.qml
        SOURCES elfdisassembler.h elfdisassembler.cpp
)

set_target_properties(appFileMetaInfo PROPERTIES
    OUTPUT_NAME "elf_meta_editor"
)

target_link_libraries(appFileMetaInfo
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        ${LIBELF_PATH}
)

target_include_directories(appFileMetaInfo PRIVATE /usr/include/libelf)

install(TARGETS appFileMetaInfo
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
