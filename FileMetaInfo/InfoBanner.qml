import QtQuick
import QtQuick.Controls

Popup {
    id: root
    property string type: "info"
    property alias message: label.text

    x: (parent.width - width) / 2
    y: parent.height - 80
    width: contentRow.implicitWidth + 40
    height: 45
    padding: 0
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        // Темный техно-фон с цветной полоской снизу для акцента
        color: "#1e1e1e"
        border.color: "#333"
        radius: 4

        Rectangle {
            width: parent.width
            height: 2
            anchors.bottom: parent.bottom
            color: root.type === "success" ? "#00ff00" :
                   (root.type === "error" ? "#ff4444" : "#00bfff")
        }
    }

    contentItem: Row {
        id: contentRow
        spacing: 12
        leftPadding: 15
        rightPadding: 15
        anchors.verticalCenter: parent.verticalCenter

        Text {
            text: root.type === "success" ? "✔" : (root.type === "error" ? "⚠" : "ℹ")
            color: root.type === "success" ? "#00ff00" :
                   (root.type === "error" ? "#ff4444" : "#00bfff")
            font.bold: true
            font.pointSize: 12
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: label
            color: "#e0e0e0"
            font.family: "Monospace"
            font.pointSize: 10
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // Анимации
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200 }
        NumberAnimation { property: "y"; from: parent.height; to: parent.height - 80; duration: 300; easing.type: Easing.OutCubic }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 500 }
    }

    Timer {
        id: hideTimer
        interval: 3500 // 3.5 секунды вполне достаточно
        onTriggered: root.close()
    }

    function show(msg, msgType = "info") {
        message = msg;
        type = msgType;
        open();
        hideTimer.restart();
    }
}
