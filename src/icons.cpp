// Helper function to dynamically draw a simple Line icon
#include "icons.h"

QIcon createLineIcon(unsigned size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
    // Draw diagonal line
    painter.drawLine(3, size - 10, size - 10, 3);
    return QIcon(pixmap);
}

// Helper function to dynamically draw a simple Circle icon
QIcon createCircleIcon(unsigned size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    // Draw circle in center
    painter.drawEllipse(5, 5, size - 10, size - 10);
    return QIcon(pixmap);
}

QIcon createRectangleIcon(unsigned size) {
    // 1. Create a 16x16 pixmap with a transparent background
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    // 3. Define pen (3px outline) and no brush
    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(5, 5, size - 10, size -10);
    painter.end();
    // 5. Wrap the QPixmap into a QIcon
    return QIcon(pixmap);
}

QIcon createDragIcon(unsigned size, const QColor &dotColor) {
    // 1. Create a transparent pixmap buffer
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(dotColor);

    // 2. Define grid parameters (2 columns x 3 rows of grip dots)
    int dotRadius = size / 13; // Radius of each dot
    if (dotRadius < 2) dotRadius = 2;

    int spacingY = size / 4;  // Vertical distance between dots
    int col1_X = size / 3;    // Left column X
    int col2_X = (size * 2) / 3; // Right column X
    int startY = size / 4;    // Top row Y

    // 3. Draw the 6 dots forming a vertical drag grip handle
    for (int i = 0; i < 3; ++i) {
        int y = startY + (i * spacingY);
        // Left column dot
        painter.drawEllipse(QPoint(col1_X, y), dotRadius, dotRadius);
        // Right column dot
        painter.drawEllipse(QPoint(col2_X, y), dotRadius, dotRadius);
    }
    return QIcon(pixmap);
}

QIcon createPencilIcon(const QColor &color) {
    QPixmap pixmap(ICON_SIZE/2, ICON_SIZE/2);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Pencil Body (Filled with active color)
    QPolygonF body;
    body << QPointF(5.5, 12.5)
         << QPointF(3.5, 10.5)
         << QPointF(10.5, 3.5)
         << QPointF(12.5, 5.5);

    painter.setPen(QPen(QColor("#1E293B"), 1)); // Dark outline
    painter.setBrush(color);                   // Dynamic user color
    painter.drawPolygon(body);

    // 2. Pencil Tip (Wood core)
    QPolygonF tip;
    tip << QPointF(5.5, 12.5)
        << QPointF(3.5, 10.5)
        << QPointF(1.5, 14.5);

    painter.setBrush(QColor("#1E293B")); // Wood color
    painter.drawPolygon(tip);

    // 3. Lead Tip
    QPolygonF graphite;
    graphite << QPointF(2.5, 13.5)
             << QPointF(1.5, 14.5)
             << QPointF(2.5, 12.5);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1E293B"));
    painter.drawPolygon(graphite);

    // 4. Eraser Cap
    QPolygonF eraser;
    eraser << QPointF(10.5, 3.5)
           << QPointF(12.5, 5.5)
           << QPointF(14.0, 4.0)
           << QPointF(12.0, 2.0);

    painter.setPen(QPen(QColor("#1E293B"), 1));
    painter.setBrush(QColor("#EC4899")); // Pink eraser
    painter.drawPolygon(eraser);

    painter.end();
    return QIcon(pixmap);
}

QIcon createBrushIcon(const QColor &fillColor) {
    QPixmap pixmap(ICON_SIZE*2/3, ICON_SIZE*2/3);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // --- A. Draw the Tilted Paint Bucket Body ---
    QPainterPath bucketPath;
    bucketPath.moveTo(6, 6);   // Top-left opening
    bucketPath.lineTo(13, 13); // Top-right opening
    bucketPath.lineTo(10, 20); // Bottom-right base
    bucketPath.lineTo(3, 13);  // Bottom-left base
    bucketPath.closeSubpath();

    painter.setPen(QPen(QColor("#2c3e50"), 1.2)); // Dark border
    painter.setBrush(QColor("#7f8c8d"));         // Metallic bucket metal
    painter.drawPath(bucketPath);

    // --- B. Draw Metal Bucket Handle ---
    QPainterPath handlePath;
    handlePath.moveTo(6, 6);
    handlePath.quadTo(3, 2, 7, 2);
    handlePath.quadTo(12, 2, 13, 13);

    painter.setPen(QPen(QColor("#2c3e50"), 1.2, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(handlePath);

    // --- C. Paint Surface Inside Bucket Opening ---
    QPainterPath openingPath;
    openingPath.moveTo(6, 6);
    openingPath.lineTo(13, 13);
    openingPath.lineTo(11, 15);
    openingPath.lineTo(4, 8);
    openingPath.closeSubpath();

    painter.setPen(QPen(fillColor.darker(130), 1));
    painter.setBrush(fillColor);
    painter.drawPath(openingPath);

    // --- D. Paint Pour Spout / Liquid Stream ---
    QPainterPath pourPath;
    pourPath.moveTo(3, 13); // Starts from lower edge of bucket
    pourPath.quadTo(1, 15, 2, 17);
    pourPath.quadTo(4, 18, 5, 15);
    pourPath.closeSubpath();

    painter.setPen(QPen(fillColor.darker(130), 1));
    painter.setBrush(fillColor);
    painter.drawPath(pourPath);

    // --- E. Liquid Drop at Bottom Tip (Hotspot location) ---
    QPainterPath dropPath;
    dropPath.moveTo(3, 20);
    dropPath.quadTo(1, 22, 3, 23);
    dropPath.quadTo(5, 22, 3, 20);
    dropPath.closeSubpath();

    painter.setPen(Qt::NoPen);
    painter.setBrush(fillColor);
    painter.drawPath(dropPath);

    painter.end();

    // 2. Wrap and return as QIcon
    return QIcon(pixmap);
}

QIcon createPolygonIcon() {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 2. Define the 5 vertices of the polygon for 32x32 bounds
    QPolygonF polygon;
    polygon << QPointF(16.0, 3.5)   // Top point
            << QPointF(28.0, 11.5)  // Top-right
            << QPointF(23.0, 27.5)  // Bottom-right
            << QPointF(9.0, 27.5)   // Bottom-left
            << QPointF(4.0, 11.5);   // Top-left

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::NoBrush);
    
    // Draws both fill and border outline
    painter.drawPolygon(polygon);
    painter.end();
    return QIcon(pixmap);
}