#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "pemodel.h"
#include "fileprocessor.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    PEModel peModel;
    FileProcessor backend(&peModel);

    QQmlApplicationEngine engine;

    // Соединяем сигнал ошибки, чтобы приложение закрывалось, если QML не загрузился
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Регистрируем бэкенд и модель в контексте QML
    engine.rootContext()->setContextProperty("_peModel", &peModel);
    engine.rootContext()->setContextProperty("_backend", &backend);

    // Загружаем Main.qml из модуля FileSurgeon (URI из CMake)
    // "Main" — это имя файла без расширения .qml
    engine.loadFromModule("FileSurgeon", "Main");

    return app.exec();
}
