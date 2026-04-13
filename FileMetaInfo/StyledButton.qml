import QtQuick
import QtQuick.Controls
import QtQuick.Effects // Для эффекта свечения (Qt 6.5+)

Button {
    id: control
    property bool isDanger: false

    implicitWidth: 140
    implicitHeight: 40

    contentItem: Text {
        text: control.text
        font.family: "Monospace"
        font.pixelSize: 12
        font.bold: true
        font.capitalization: Font.AllUppercase // Sci-Fi стиль любит капс

        // Основной цвет текста
        color: !control.enabled ? "#444" : (isDanger ? "#ff4444" : "#00f2ff")

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        // Добавляем эффект "неонового свечения" тексту
        layer.enabled: control.enabled && (control.hovered || control.visualFocus)
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 0.5
            brightness: 0.5
            colorizationColor: isDanger ? "#ff4444" : "#00f2ff"
            colorization: 1.0
        }
    }

    background: Rectangle {
        color: control.down ? "#3300f2ff" : (control.hovered ? "#1a00f2ff" : "transparent")
        border.color: !control.enabled ? "#333" : (isDanger ? "#ff4444" : "#00f2ff")
        border.width: control.visualFocus ? 2 : 1
        radius: 2

        // Декоративные "уголки" терминала
        Rectangle {
            width: 4; height: 4
            color: parent.border.color
            anchors.left: parent.left
            anchors.top: parent.top
        }
        Rectangle {
            width: 4; height: 4
            color: parent.border.color
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }

        // Анимация свечения рамки при наведении
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.color: parent.border.color
            border.width: 2
            opacity: control.hovered ? 0.5 : 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    // Плавное изменение прозрачности при нажатии
    opacity: control.enabled ? 1.0 : 0.5
    Behavior on opacity { NumberAnimation { duration: 100 } }
}
