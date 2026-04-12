import QtQuick
import QtQuick.Controls

Popup {
    id: root

    parent: Overlay.overlay
    x: (parent.width - width) / 2
    y: parent.height - height - 50
    width: Math.max(250, label.implicitWidth + 40)
    height: 45

    background: Rectangle {
        color: "#2b2b2b"
        radius: 10
        border.color: "#555555"
        border.width: 1
    }

    contentItem: Label {
        id: label
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 13
        text: "" // Изначально пусто
    }

    Timer {
        id: closeTimer
        interval: 3000
        onTriggered: root.close()
    }

    // Эта функция теперь железно устанавливает текст
    function show(msg) {
        label.text = msg
        root.open()
        closeTimer.restart()
    }
}
