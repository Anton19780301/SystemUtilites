import QtQuick
import QtQuick.Layouts

RowLayout {
    spacing: 20

    // Блок Entry Point
    Column {
        Text { text: "ENTRY POINT"; color: "#00f2ff"; font.pixelSize: 10; font.bold: true }
        Text {
            text: elfManager.entryPoint
            color: "#ffffff"
            font.family: "Monospace"
            font.pointSize: 12
        }
    }

    // Блок Architecture
    Column {
        Text { text: "ARCHITECTURE"; color: "#00f2ff"; font.pixelSize: 10; font.bold: true }
        Text {
            text: elfManager.machineType
            color: "#ffffff"
            font.family: "Monospace"
            font.pointSize: 12
        }
    }

    // Блок File Size
    Column {
        Text { text: "TOTAL SIZE"; color: "#00f2ff"; font.pixelSize: 10; font.bold: true }
        Text {
            text: (elfManager.fileSize / 1024).toFixed(2) + " KB"
            color: "#ffffff"
            font.family: "Monospace"
            font.pointSize: 12
        }
    }
}
