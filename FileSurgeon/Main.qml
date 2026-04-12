import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

// Добавь это для FileDialog
ApplicationWindow {
    width: 900
    height: 600
    visible: true
    title: "PE File Surgeon"

    // Объект диалога выбора файла
    FileDialog {
        id: fileDialog
        title: "Выберите исполняемый файл"
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.DocumentsLocation)
        nameFilters: ["Executable files (*.exe *.dll)", "All files (*)"]
        onAccepted: {
            // Передаем выбранный файл в бэкенд
            _backend.openFile(selectedFile)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Button {
                text: "Выбрать файл"
                onClicked: fileDialog.open()
            }
            Button {
                text: "Сохранить"
                onClicked: _backend.saveChanges(textArea.text)
            }
            Button {
                text: "Добавить секцию"
                onClicked: addSectionDialog.open()
            }
            Button {
                text: "Удалить секцию"
                // Кнопка активна, если в текстовом поле что-то есть (значит секция выбрана)
                enabled: textArea.text !== ""
                onClicked: {
                    _backend.removeCurrentSection()
                }
            }
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                SplitView.preferredWidth: 300
                TreeView {
                    id: peTreeView
                    model: _peModel
                    delegate: ItemDelegate {
                        text: model.display
                        width: parent.width
                        onClicked: {
                            // Передаем индекс нажатого элемента в бэкенд
                            // peTreeView.modelIndex(row) — получает индекс строки
                            _backend.selectNode(peTreeView.index(row, column))
                        }
                    }
                }
            }

            ScrollView {
                TextArea {
                    id: textArea
                    font.family: "Courier New"
                    // Используем правильное имя свойства из C++ (currentContent)
                    text: _peModel.currentContent
                }
            }
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
