import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    property alias model: listView.model
    property alias currentIndex: listView.currentIndex
    signal sectionSelected(string name)

    color: "transparent"
    border.color: "#1d212d"

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 2
        clip: true
        delegate: ItemDelegate {
            id: control
            width: listView.width
            height: 35

            // Передаем состояние подсветки
            highlighted: listView.currentIndex === index

            contentItem: Text {
                text: modelData
                // Если выбрано — текст светится Cyan, если нет — приглушенный серый
                color: control.highlighted ? "#00f2ff" : "#888ea8"
                font.family: "Monospace"
                font.bold: control.highlighted
                font.pointSize: 10
                verticalAlignment: Text.AlignVCenter
                leftPadding: 15

                // Эффект небольшого свечения для выбранного текста
                layer.enabled: control.highlighted
                layer.effect: ShaderEffect {
                    // Можно добавить небольшое размытие, но для начала просто Bold + Цвет
                }
            }

            background: Rectangle {
                // Фон всей ячейки при наведении или выборе
                color: control.highlighted ? "#1a00f2ff" : (control.hovered ? "#0a00f2ff" : "transparent")

                // Неоновая вертикальная полоска слева у выбранного элемента
                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 3
                    height: control.highlighted ? parent.height * 0.8 : 0
                    color: "#00f2ff"

                    // Плавное появление полоски
                    Behavior on height { NumberAnimation { duration: 200 } }
                }

                // Тонкая разделительная линия снизу
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 1
                    color: "#1d212d"
                    opacity: 0.5
                }
            }

            onClicked: {
                listView.currentIndex = index
                root.sectionSelected(modelData)
            }
        }
    }
}