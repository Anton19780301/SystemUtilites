import QtQuick
import QtQuick.Layouts


RowLayout {
    id: root
    spacing: 15

    // СИГНАЛЫ (проверь, что они тут есть!)
    signal openClicked()
    signal saveClicked()
    signal addClicked()
    signal removeClicked()
    signal stripClicked()
    signal repackClicked()

    property bool canDelete: false

    StyledButton {
        text: "Стриппинг"
        onClicked: stripClicked()
    }

    StyledButton {
        text: "Открыть файл"
        onClicked: root.openClicked()
    }

    StyledButton {
        text: "Сохранить"
        onClicked: root.saveClicked()
    }

    Item { Layout.fillWidth: true }

    StyledButton {
        text: "Добавить раздел"
        onClicked: root.addClicked()
    }

    StyledButton {
        text: "Удалить"
        isDanger: true
        enabled: root.canDelete
        onClicked: root.removeClicked()
    }

    StyledButton {
        text: "Упаковать"
        onClicked: repackClicked()
        enabled: false //TODO: переделать алгоритм, сейчас он приводит к нарушению сигментации файлов
    }
}
