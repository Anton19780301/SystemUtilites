import QtQuick

Canvas {
    id: noiseCanvas
    anchors.fill: parent
    z: 98
    opacity: 0.25 // Едва заметная зернистость

    onPaint: {
        var ctx = getContext("2d");
        var w = width;
        var h = height;
        ctx.clearRect(0, 0, w, h);

        // Рисуем случайные точки
        for (var i = 0; i < 1500; i++) {
            var x = Math.random() * w;
            var y = Math.random() * h;
            var size = Math.random() * 2;
            ctx.fillStyle = Qt.rgba(1, 1, 1, Math.random());
            ctx.fillRect(x, y, size, size);
        }
    }

    Timer {
        interval: 80 // Обновляем ~12 раз в секунду для эффекта мерцания
        running: true
        repeat: true
        onTriggered: noiseCanvas.requestPaint()
    }
}
