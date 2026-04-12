pragma Singleton

// Делаем его доступным везде без импорта
import QtQuick

QtObject {
    readonly property color bgColor: "#1e1e1e"
    readonly property color treeBg: "#252526"
    readonly property color border: "#333333"
    readonly property color textPrimary: "#d4d4d4"
    readonly property color accent: "#007acc" // Синий как в VS Code
    readonly property font monoFont: Qt.font({
                                                 "family": "Consolas",
                                                 "pointSize": 10
                                             })
}
