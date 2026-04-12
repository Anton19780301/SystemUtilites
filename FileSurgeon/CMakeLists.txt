cmake_minimum_required(VERSION 3.16)

project(FileSurgeon VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # WinAPI и Qt6 хорошо дружат с 17 стандартом
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

# Указываем все файлы здесь для порядка
qt_add_executable(appFileSurgeon
    main.cpp
    pemodel.h pemodel.cpp
    fileprocessor.h fileprocessor.cpp
)

qt_add_qml_module(appFileSurgeon
    URI FileSurgeon
    QML_FILES Main.qml
)

# Для работы с некоторыми функциями WinAPI иногда нужно явно линковать user32 или shell32,
# но для IMAGE_DOS_HEADER (kernel32) это не обязательно.
target_link_libraries(appFileSurgeon
    PRIVATE Qt6::Quick
)

set_target_properties(appFileSurgeon PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
