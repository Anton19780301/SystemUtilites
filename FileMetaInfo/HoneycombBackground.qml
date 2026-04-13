import QtQuick
import QtQuick.Effects

Item {
    id: root
    anchors.fill: parent

    //Сетка (Источник)
    Canvas {
        id: honeycombCanvas
        anchors.fill: parent
        visible: false
        // Включаем слой, чтобы MultiEffect гарантированно видел отрисованное
        layer.enabled: true

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = "#00f2ff";
            ctx.lineWidth = 1.5;
            ctx.lineCap = "round";

            var r = 30; // Радиус (расстояние от центра до угла)
            var h = r * Math.sqrt(3); // Физическая высота соты

            // Шаг X должен быть 1.5 * r для плотной стыковки плоских граней
            // Шаг Y должен быть h (высота соты)
            for (var x = 0; x < width + r * 2; x += r * 1.5) {
                // Смещаем каждый второй столбец по вертикали на половину высоты
                var isOdd = (Math.floor(x / (r * 1.5)) % 2);
                var offsetY = isOdd ? h / 2 : 0;

                for (var y = 0; y < height + h; y += h) {
                    drawHex(ctx, x, y + offsetY, r);
                }
            }
        }

        function drawHex(ctx, x, y, r) {
            ctx.beginPath();
            for (var i = 0; i < 6; i++) {
                // Угол i * 60 градусов
                var angle = i * Math.PI / 3;
                ctx.lineTo(x + r * Math.cos(angle), y + r * Math.sin(angle));
            }
            ctx.closePath();
            ctx.stroke();
        }
    }

    //Контейнер масок
    Item {
        id: maskContainer
        anchors.fill: parent
        visible: false
        // КРИТИЧЕСКИ ВАЖНО: заставляет Qt обновлять анимации пятен внутри скрытого айтема
        layer.enabled: true

        ScanningSpot { }
        ScanningSpot { durationX: 8000; durationY: 4500 }
    }

    //Финальный эффект
    MultiEffect {
        anchors.fill: parent
        source: honeycombCanvas
        maskSource: maskContainer

        maskEnabled: true

        // Эффекты для самих сот
        blurEnabled: true
        blur: 0.4
        brightness: 0.6

        // Анимация "дыхания" (мерцания)
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.15; to: 0.5; duration: 3000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 0.5; to: 0.15; duration: 3000; easing.type: Easing.InOutQuad }
        }
    }

}
