import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import FileMetaInfo

ApplicationWindow {
    width: 1200
    height: 900
    visible: true
    title: "ELF Meta Editor v2.0"

    background: Rectangle
    {
        color: "#0a0b10"

    }

    ElfManager {
        id: elfManager
    }

    FileDialog {
        id: fileDialog
        onAccepted: elfManager.loadFile(selectedFile)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        FileInfoPanel {
            Layout.fillWidth: true
        }

        ElfMap {
            Layout.fillWidth: true
            Layout.preferredHeight: 45
        }

        RowLayout {
            spacing: 10
            Layout.fillHeight: true
            Layout.fillWidth: true

            SectionListView {
                id: sections
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                model: elfManager.sectionNames
                onSectionSelected: (name) => {
                    // Проверяем состояние переключателя прямо в момент клика
                    if (editor.isHexMode) {
                        editor.text = elfManager.getSectionHex(name)
                    } else {
                        editor.text = elfManager.getSectionContent(name)
                    }
                }
            }

            EditorPanel {
                id: editor
                Layout.fillWidth: true
                Layout.fillHeight: true
                placeholderText: "Выберите раздел..."

                onModeChanged: (isHex) => {
                    if (sections.currentIndex !== -1) {
                        let name = elfManager.sectionNames[sections.currentIndex]
                        // Обновляем текст в зависимости от режима
                        editor.text = isHex ? elfManager.getSectionHex(name)
                                            : elfManager.getSectionContent(name)
                    }
                }
            }
        }

        ControlButtons {
            id: controls
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            canDelete: sections.currentIndex !== -1 &&
                       elfManager.isDeletable(elfManager.sectionNames[sections.currentIndex])

            onOpenClicked: fileDialog.open()
            onAddClicked: addSectionDialog.open()
            onSaveClicked: {
                let currentName = elfManager.sectionNames[sections.currentIndex]
                elfManager.saveSection(currentName, editor.text)
            }
            onRemoveClicked: removeConfirmation.open()
            onStripClicked: stripConfirmation.open()
            onRepackClicked: elfManager.repackElf()
        }
    }


    Dialog {
        id: addSectionDialog
        title: "Новый раздел"
        standardButtons: Dialog.Ok | Dialog.Cancel

        Column {
            spacing: 10
            width: parent.width
            Label { text: "Имя раздела (например, .note.info):" }
            TextField {
                id: newSectionName
                width: 200
                text: ".new_section"
            }
        }

        onAccepted: {
            elfManager.addSection(newSectionName.text, "Sample text")
        }
    }

    Dialog {
        id: removeConfirmation
        title: "Внимание"
        standardButtons: Dialog.Yes | Dialog.No
        Label { text: "Вы уверены, что хотите удалить этот раздел?" }
        onAccepted: {
            if (sections.currentIndex !== -1) {
                elfManager.removeSection(elfManager.sectionNames[sections.currentIndex])
            }
        }
    }

    Dialog {
        id: stripConfirmation
        title: "Очистка ELF"
        standardButtons: Dialog.Yes | Dialog.No
        Label { text: "Удалить все отладочные секции и комментарии? Это уменьшит размер файла." }
        onAccepted: elfManager.stripAll()
    }

    InfoBanner {
        id: infoBanner
    }

    Connections {
        target: elfManager
        // Ловим сигнал из C++ и показываем баннер
        function onMessageReady(msg, msgType) { infoBanner.show(msg, msgType) }
    }

    Scanline {
        width: parent.width
    }

    DropArea {
        id: dragAndDropHandler
        anchors.fill: parent

        onEntered: (drag) => {
            // Подсвечиваем рамку или показываем баннер, что файл можно бросить
            if (drag.hasUrls) {
                drag.accept()
                infoBanner.show("Бросайте файл для анализа", "info")
            }
        }

        onDropped: (drop) => {
            if (drop.hasUrls) {
                // QML возвращает список URL, берем первый
                let filePath = drop.urls[0].toString()
                if (elfManager.loadFile(filePath)) {
                    infoBanner.show("Файл загружен через Drop", "success")
                }
                drop.accept()
            }
        }
    }

    HoneycombBackground { anchors.fill: parent }
    DigitalNoise { anchors.fill: parent }


}
