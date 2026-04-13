import QtQuick

Item {
    id: spot
    width: 500; height: 500
    visible: true

    property int durationX: Math.random() * 4000 + 6000
    property int durationY: Math.random() * 4000 + 7000

    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var r = width / 2;
            var cx = width / 2;
            var cy = height / 2;

            // Растягиваем градиент для мягкости краев
            var grad = ctx.createRadialGradient(cx, cy, r * 0.1, cx, cy, r);
            grad.addColorStop(0, "white");      // Центр
            grad.addColorStop(0.3, "white");    // Зона полной видимости
            grad.addColorStop(1, "transparent"); // Плавное исчезновение к краям

            ctx.fillStyle = grad;

            ctx.beginPath();
            for (var i = 0; i < 8; i++) {
                var angle = i * Math.PI / 4;
                var x = cx + r * Math.cos(angle);
                var y = cy + r * Math.sin(angle);
                if (i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            }
            ctx.closePath();
            ctx.fill();
        }
        Component.onCompleted: requestPaint()
    }

    // Анимации оставляем как были
    SequentialAnimation on x {
        loops: Animation.Infinite
        NumberAnimation { to: parent.width - spot.width; duration: spot.durationX; easing.type: Easing.InOutQuad }
        NumberAnimation { to: 0; duration: spot.durationX; easing.type: Easing.InOutQuad }
    }
    SequentialAnimation on y {
        loops: Animation.Infinite
        NumberAnimation { to: parent.height - spot.height; duration: spot.durationY; easing.type: Easing.InOutQuad }
        NumberAnimation { to: 0; duration: spot.durationY; easing.type: Easing.InOutQuad }
    }
}
