cmake_minimum_required(VERSION 3.16)
project(FileSurgeon VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup()

# 1. Сначала ЯВНО создаем исполняемый файл
qt_add_executable(appFileSurgeon
    src/main.cpp
    src/pemodel.h src/pemodel.cpp
    src/fileprocessor.h src/fileprocessor.cpp
    src/jsonhighlighter.h src/jsonhighlighter.cpp
)

# 2. Привязываем QML-модуль к уже СОЗДАННОМУ исполняемому файлу
qt_add_qml_module(appFileSurgeon
    URI FileSurgeon
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Notification.qml
        qml/Style.qml
        qml/TreeDelegate.qml
        qml/CyberButton.qml
)

# 3. Линкуем библиотеки
target_link_libraries(appFileSurgeon PRIVATE Qt6::Quick Qt6::QuickControls2)

# 4. Добавляем путь к инклудам, чтобы main.cpp видел хедеры из папки src
target_include_directories(appFileSurgeon PRIVATE src)
