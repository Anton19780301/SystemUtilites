import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import FileSurgeon

ApplicationWindow {
    id: window
    width: 1000
    height: 700
    visible: true
    title: "PE File Surgeon"

    // 1. Устанавливаем фон явно
    background: Rectangle {
        color: "#1e1e1e"
    }

    Notification {
        id: notification
    }

    Connections {
        target: _backend
        function onStatusMessage(msg) {
            notification.show(msg)
        }
    }

    // Основной контейнер
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Панель кнопок
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Открыть"
                onClicked: fileDialog.open()
            }
            Button {
                text: "Сохранить"
                highlighted: true
                onClicked: _backend.saveChanges(textArea.text)
            }
            Button {
                text: "Добавить"
                onClicked: addSectionDialog.open()
            }
            Button {
                text: "Удалить"
                enabled: textArea.text !== ""
                onClicked: _backend.removeCurrentSection()
            }
        }

        // Рабочая область
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Разделитель
            handle: Rectangle {
                implicitWidth: 4
                color: "#333"
            }

            // Левая часть: Дерево
            Rectangle {
                SplitView.preferredWidth: 250
                color: "#252526"
                border.color: "#333"

                ScrollView {
                    anchors.fill: parent
                    clip: true // Чтобы дерево не вылезало за границы

                    TreeView {
                        id: peTreeView
                        anchors.fill: parent
                        model: _peModel
                        delegate: TreeDelegate {} // Тот самый файл

                        selectionModel: ItemSelectionModel {
                            model: _peModel
                        }
                    }
                }
            }

            // Правая часть: Редактор
            Rectangle {
                color: "#1e1e1e"
                border.color: "#333"

                ScrollView {
                    anchors.fill: parent

                    TextArea {
                        id: textArea
                        color: "#d4d4d4"
                        font.family: "Consolas"
                        font.pointSize: 11
                        text: _peModel.currentContent
                        background: null // Убираем стандартный белый фон
                        placeholderText: "Выберите секцию..."
                        placeholderTextColor: "#666"
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Выберите исполняемый файл"
        // StandardPaths требует import QtCore
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.DocumentsLocation)
        nameFilters: ["Executable files (*.exe *.dll)", "All files (*)"]
        onAccepted: {
            // selectedFile — это свойство самого FileDialog в Qt 6
            _backend.openFile(selectedFile)
        }
    }

    Dialog {
        id: addSectionDialog
        title: "Добавить новую секцию"
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent

        TextField {
            id: sectionNameInput
            placeholderText: "Имя (макс. 8 симв.)"
            maximumLength: 8
            width: parent.width
        }

        onAccepted: {
            _backend.addNewSection(sectionNameInput.text)
            sectionNameInput.clear()
        }
    }
}
