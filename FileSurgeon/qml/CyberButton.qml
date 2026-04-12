import QtQuick
import QtQuick.Controls

Button {
    id: control
    text: "BUTTON"

    // Кастомные свойства для удобства
    property color accentColor: "#00f0ff" // Тот самый "кибер" голубой

    contentItem: Text {
        text: control.text
        font.family: "Consolas"
        font.pixelSize: 11
        font.bold: true
        color: control.hovered ? control.accentColor : "#aaaaaa"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 32
        color: control.pressed ? "#111111" : "#252526"
        border.color: control.hovered ? control.accentColor : "#444444"
        border.width: 1
        radius: 2

        // Нижняя светящаяся полоска
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: control.hovered ? parent.width : 0
            height: 2
            color: control.accentColor
            opacity: 0.6

            Behavior on width {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
