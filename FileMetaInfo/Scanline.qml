import QtQuick

Rectangle {
    id: root
    width: parent.width
    height: 3
    z: 100
    opacity: 0.5

    // Начальная позиция выше экрана
    y: -20

    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: "transparent" }
        GradientStop { position: 0.5; color: "#00f2ff" }
        GradientStop { position: 1.0; color: "transparent" }
    }

    // Свечение (необязательно, но добавляет веса)
    layer.enabled: true

    PropertyAnimation on y {
        from: -50
        to: root.parent ? root.parent.height + 50 : 800
        duration: 5000
        loops: Animation.Infinite
        running: true
    }
}
