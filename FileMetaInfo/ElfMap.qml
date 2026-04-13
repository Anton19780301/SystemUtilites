import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    height: 50
    color: "#0a0b10" // Цвет "пустоты"
    border.color: "#1d212d"
    radius: 2
    clip: true

    property var details: elfManager.sectionDetails
    property real totalSize: elfManager.fileSize

    // Сетка для Sci-Fi эффекта
    Canvas {
        anchors.fill: parent
        opacity: 0.1
        onPaint: {
            var ctx = getContext("2d");
            ctx.strokeStyle = "#00f2ff";
            for(var i=0; i<width; i+=20) { ctx.moveTo(i,0); ctx.lineTo(i,height); }
            ctx.stroke();
        }
    }

    // Отрисовка секций
    Repeater {
        model: root.details
        delegate: Rectangle {
            // Позиция X строго по офсету
            x: (modelData.offset / root.totalSize) * root.width
            width: Math.max((modelData.size / root.totalSize) * root.width, 1)
            height: parent.height

            // Цвета: системные — Cyan, пользовательские — Green
            color: modelData.name.startsWith(".") ? "#8800f2ff" : "#8800ff9f"
            border.color: highlighted ? "#ffffff" : "#4400f2ff"
            border.width: highlighted ? 2 : 1

            property bool highlighted: sections.currentIndex === modelData.index

            // Эффект свечения для активной секции
            Rectangle {
                anchors.fill: parent
                color: parent.color
                opacity: parent.highlighted ? 0.3 : 0
                visible: parent.highlighted
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: sections.currentIndex = modelData.index

                ToolTip.visible: containsMouse
                ToolTip.text: "<b>" + modelData.name + "</b><br>" +
                              "Offset: 0x" + modelData.offset.toString(16).toUpperCase() + "<br>" +
                              "Size: " + modelData.size + " bytes"
            }
        }
    }
}
