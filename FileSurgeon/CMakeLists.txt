cmake_minimum_required(VERSION 3.16)
project(FileSurgeon VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup()

# Подключаем папку с C++ (там будет только qt_add_executable)
add_subdirectory(src)

# Описываем модуль ЗДЕСЬ, потому что папка qml/ находится на этом уровне
qt_add_qml_module(appFileSurgeon
    URI FileSurgeon
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        qml/Main.qml
        qml/Notification.qml
        qml/Style.qml
        qml/TreeDelegate.qml
)
