import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property alias text: textArea.text
    property alias placeholderText: textArea.placeholderText
    property bool isHexMode: modeSwitch.checked // Состояние режима

    color: "#cc12141d"
    border.color: "#00f2ff"
    radius: 2

    // Сигнал для Main.qml, чтобы обновить контент при смене режима
    signal modeChanged(bool hex)

    // Панель переключения режима
    Row {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        z: 10
        spacing: 8

        Text {
            text: modeSwitch.checked ? "HEX MODE" : "TEXT MODE"
            color: "#00f2ff"
            font.family: "Monospace"
            font.pixelSize: 10
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Switch {
            id: modeSwitch
            onToggled: root.modeChanged(checked)
        }
    }

    ScrollView {
        anchors.fill: parent
        TextArea {
            id: textArea
            textFormat: TextEdit.PlainText
            color: root.isHexMode ? "#ae81ff" : "#00ff9f" // Пурпурный для HEX, зеленый для текста
            font.family: "Monospace"
            font.pointSize: 10
            background: null
            leftPadding: 15
            topPadding: 35 // Отступ сверху, чтобы текст не залез под переключатель
            readOnly: root.isHexMode // В HEX-режиме редактирование обычно запрещено

            Component.onCompleted: {
                if (typeof elfManager !== "undefined") elfManager.setupHighlighter(textArea)
            }
        }
    }
}
