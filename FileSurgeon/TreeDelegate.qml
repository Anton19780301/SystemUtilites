import QtQuick
import QtQuick.Controls

ItemDelegate {
    id: control
    width: parent.width

    // 1. Используем id самого дерева из Main.qml (peTreeView)
    // Либо используем встроенное свойство TreeView.view (если оно подхватится)
    property bool isSelected: peTreeView.selectionModel.hasSelection
                              && peTreeView.selectionModel.currentIndex === peTreeView.index(
                                  row, column)

    contentItem: Text {
        text: model.display
        color: control.isSelected ? "white" : "#cccccc"
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        leftPadding: 10
    }

    background: Rectangle {
        color: control.isSelected ? "#37373d" : (control.hovered ? "#2a2d2e" : "transparent")

        Rectangle {
            width: 3
            height: parent.height
            color: "#007acc"
            visible: control.isSelected
        }
    }

    onClicked: {
        // Прямое обращение по id дерева гораздо стабильнее
        let idx = peTreeView.index(row, column)
        peTreeView.selectionModel.setCurrentIndex(
                    idx, ItemSelectionModel.ClearAndSelect)
        _backend.selectNode(idx)
    }
}
