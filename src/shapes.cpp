// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line & Circle)
// -------------------------------------------------------------
#include "shapes.h"
#include <QToolTip>
#include <algorithm>

// Define the pen and the brush for the selected shapes
Shape::Shape(QPen pen, QBrush brush) :
    m_pen(pen), m_brush(brush),
    m_shape_select_brush(QBrush(Qt::darkGray, Qt::DiagCrossPattern)),
    m_shape_select_pencil(QPen(Qt::darkYellow, 1.5, Qt::DashLine)),
    m_hook_pen(QPen(Qt::blue, 2, Qt::SolidLine)),
    m_hook_brush(QBrush(Qt::white, Qt::SolidPattern)) {
}

// ========================== LINE SHAPE ==========================
LineShape::LineShape(const QLineF &line, const QPen &pen, const QBrush& brush)
    : Shape(pen, brush), m_line(line) {}

void LineShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.drawLine(m_line);
}

void LineShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());

    QLineF line(*shape_data.start, *shape_data.end);
    painter.setPen(shape_data.pencil);
    painter.setBrush(shape_data.brush);
    painter.drawLine(line);
}

void LineShape::drawSelect(QPainter &painter, const bool draw_hooks) const {
    painter.setPen(m_shape_select_pencil);
    painter.drawLine(m_line);
}

std::unique_ptr<Shape> LineShape::clone() const {
    return std::make_unique<LineShape>(*this);
}

ShapeType LineShape::type() const { 
    return ShapeType::Line;
}

void LineShape::serialize(QDataStream &out) const {
    out << m_line << m_pen; // QDataStream natively supports Qt types!
}

void LineShape::deserialize(QDataStream &in) {
    in >> m_line >> m_pen;
}

bool LineShape::contains(const QPointF &point) const{
    // verify if the point is on the line segment using 
    // distance and length comparison
    QLineF p1_to_p(m_line.p1(), point);
    QLineF p_to_p2(point, m_line.p2());
    qreal totalLength = m_line.length();
    qreal splitLength = p1_to_p.length() + p_to_p2.length();
    // Check if total length matches split length within tolerance
    return std::abs(totalLength - splitLength) < EPSILON; // 1.0 is the epsilon for tolerance
}

void LineShape::addPoint(const QPointF& p) {
}

QPolygonF LineShape::getPoints() {
    return QPolygonF({m_line.p1(), m_line.p2()});
}

void LineShape::setShapeData(const ShapeData_t& shape_data) {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());
    m_line.setP1(shape_data.start.value());
    m_line.setP1(shape_data.end.value());
}

void LineShape::moveRelative(const QPointF &delta) {
    m_line.setP1(m_line.p1() + delta);
    m_line.setP2(m_line.p2() + delta);
}

void LineShape::resizeShape(const QPointF &delta, const HandlePosition hp) {
}

void LineShape::zoomInOut(const qreal& factor) {
    // factor > 1.0 lengthens the line, factor < 1.0 shortens it
    QPointF center = m_line.center();
    // Scale vectors from center to endpoints
    m_line.setP1(center + (m_line.p1() - center) * factor);
    m_line.setP2(center + (m_line.p2() - center) * factor);
}

void LineShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
}

HandlePosition LineShape::hookTest(const QPointF& pt) const {
    return None;
}

// ========================== RECTANGLE SHAPE ==========================
RectangleShape::RectangleShape(const QRectF &rectangle, const QPen &pen, const QBrush& brush)
    : Shape(pen, brush), m_rectangle(rectangle) {
        m_rectangle = m_rectangle.normalized();
    }

void RectangleShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    painter.drawRect(m_rectangle);
}

void RectangleShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());

    QRectF rect(*shape_data.start, *shape_data.end);
    rect = rect.normalized();
    painter.setPen(shape_data.pencil);
    painter.setBrush(shape_data.brush);
    painter.drawRect(rect);
}

QRectF RectangleShape::getHandleRect(const QPointF& center) const {
    return QRectF(center.x() - handle_size / 2.0,
        center.y() - handle_size / 2.0,
        handle_size,
        handle_size);
}

void RectangleShape::drawSelect(QPainter &painter, const bool draw_hooks) const {
    painter.setPen(m_shape_select_pencil);
    painter.setBrush(m_shape_select_brush);
    painter.drawRect(m_rectangle);
    const QPointF topCenter(m_rectangle.center().x(), m_rectangle.top());
    const QPointF bottomCenter(m_rectangle.center().x(), m_rectangle.bottom());
    const QPointF leftCenter(m_rectangle.left(), m_rectangle.center().y());
    const QPointF rightCenter(m_rectangle.right(), m_rectangle.center().y());
    const QPointF handleCenters[] = {
        topCenter,
        bottomCenter,
        leftCenter,
        rightCenter
    };
    if (draw_hooks) {
        painter.setPen(m_hook_pen);
        painter.setBrush(m_hook_brush);
        // draw hooking points around the shape
        for (const QPointF& handle : handleCenters) {
            painter.drawRect(getHandleRect(handle));
        }
    }
}

std::unique_ptr<Shape> RectangleShape::clone() const {
    return std::make_unique<RectangleShape>(*this);
}

ShapeType RectangleShape::type() const { 
    return ShapeType::Rectangle;
}

void RectangleShape::serialize(QDataStream &out) const {
    out << m_rectangle << m_pen << m_brush; // QDataStream natively supports Qt types!
}

void RectangleShape::deserialize(QDataStream &in) {
    in >> m_rectangle >> m_pen >> m_brush;
}

bool RectangleShape::contains(const QPointF &point) const{
    return m_rectangle.contains(point);
}

void RectangleShape::addPoint(const QPointF& p) {
}

QPolygonF RectangleShape::getPoints() {
    return QPolygonF(m_rectangle);
}

void RectangleShape::setShapeData(const ShapeData_t& shape_data) {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());
    m_rectangle.setTopLeft(shape_data.start.value());
    m_rectangle.setBottomRight(shape_data.end.value());
    m_rectangle = m_rectangle.normalized();
}

void RectangleShape::moveRelative(const QPointF &delta) {
    m_rectangle.setTopLeft( m_rectangle.topLeft() + delta);
    m_rectangle.setBottomRight( m_rectangle.bottomRight() + delta);
    m_rectangle = m_rectangle.normalized();
}

void RectangleShape::resizeShape(const QPointF &delta, const HandlePosition hp) {
    // AI generated and adapted code
    qreal topEnlarge    = -delta.y(); // Moving UP (-y) ENLARGES top
    qreal bottomEnlarge =  delta.y(); // Moving DOWN (+y) ENLARGES bottom
    qreal leftEnlarge   = -delta.x(); // Moving LEFT (-x) ENLARGES left
    qreal rightEnlarge  =  delta.x(); // Moving RIGHT (+x) ENLARGES right

    switch (hp) {
        case TopCenter:
            // Moving mouse up (-delta.y) moves top higher
            m_rectangle.setTop(m_rectangle.top() - topEnlarge);
            break;
        case BottomCenter:
            m_rectangle.setBottom(m_rectangle.bottom() + bottomEnlarge);
            break;
        case LeftCenter:
            m_rectangle.setLeft(m_rectangle.left() - leftEnlarge);
            break;
        case RightCenter:
            m_rectangle.setRight(m_rectangle.right() + rightEnlarge);
            break;
        default:
            break;
    }
}

void RectangleShape::zoomInOut(const qreal& factor) {
    QPointF center = m_rectangle.center();
    qreal newWidth = m_rectangle.width() * factor;
    qreal newHeight = m_rectangle.height() * factor;
    m_rectangle.setSize(QSizeF(newWidth, newHeight));
    m_rectangle.moveCenter(center);
}

void RectangleShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
}

// Rectangle hook checker
HandlePosition RectangleShape::hookTest(const QPointF& pt) const {
    // AI generated and adapted code
    const qreal tolerance = handle_size / 2.0 + hit_padding;

    auto isNear = [](const QPointF& p1, const QPointF& p2, qreal tol) {
        return QRectF(p2.x() - tol, p2.y() - tol, tol * 2.0, tol * 2.0).contains(p1);
    };

    // Calculate edge midpoints
    const QPointF topCenter(m_rectangle.center().x(), m_rectangle.top());
    const QPointF bottomCenter(m_rectangle.center().x(), m_rectangle.bottom());
    const QPointF leftCenter(m_rectangle.left(), m_rectangle.center().y());
    const QPointF rightCenter(m_rectangle.right(), m_rectangle.center().y());

    // 1. Check Edge-Center Handles Only
    if (isNear(pt, topCenter, tolerance))
        return HandlePosition::TopCenter;
    else if (isNear(pt, bottomCenter, tolerance))
        return HandlePosition::BottomCenter;
    else if (isNear(pt, leftCenter, tolerance))
        return HandlePosition::LeftCenter;
    else if (isNear(pt, rightCenter, tolerance))
        return HandlePosition::RightCenter;
    else
        return HandlePosition::None;
}

// ========================== CIRCLE SHAPE ==========================
CircleShape::CircleShape(
    const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush)
        : Shape(pen, brush), m_center(center),
        m_radius_x(radius),
        m_radius_y(radius) {
    }

void CircleShape::draw(QPainter &painter) const {
    if (m_radius_x >0 && m_radius_y >0) {
        painter.setPen(m_pen);
        painter.setBrush(m_brush);
        painter.drawEllipse(m_center, m_radius_x, m_radius_y);
    }
}

void CircleShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    // working assumption
    assert(shape_data.radius.has_value());
    if (shape_data.radius > 0) {
        painter.setPen(shape_data.pencil);
        painter.setBrush(shape_data.brush);
        painter.drawEllipse(*shape_data.start, *shape_data.radius, *shape_data.radius);
    }
}

void CircleShape::drawSelect(QPainter &painter, const bool draw_hooks) const {
    if (m_radius_x <= 0.0 || m_radius_y <= 0)
        return;

    painter.setRenderHint(QPainter::Antialiasing);
    // 1. Draw the circle outline / fill
    painter.setPen(m_shape_select_pencil);
    painter.setBrush(m_shape_select_brush);
    painter.drawEllipse(m_center, m_radius_x, m_radius_y);

    // 2. Compute the 4 handle midpoints on the circle perimeter
    const QPointF topCenter(m_center.x(), m_center.y() - m_radius_y);
    const QPointF rightCenter(m_center.x() + m_radius_x, m_center.y());
    const QPointF bottomCenter(m_center.x(), m_center.y() + m_radius_y);
    const QPointF leftCenter(m_center.x() - m_radius_x, m_center.y());

    const QPointF handleMidpoints[] = {
        topCenter,
        rightCenter,
        bottomCenter,
        leftCenter
    };

    // Draw small handle rectangles centered on each point
    if (draw_hooks) {
        painter.setPen(m_hook_pen);
        painter.setBrush(m_hook_brush);
        for (const QPointF& center : handleMidpoints) {
            QRectF handleRect(
                center.x() - handle_size / 2.0,
                center.y() - handle_size / 2.0,
                handle_size,
                handle_size
            );
            painter.drawRect(handleRect);
        }
    }
}

std::unique_ptr<Shape> CircleShape::clone() const {
    return std::make_unique<CircleShape>(*this);
}

ShapeType CircleShape::type() const { 
    return ShapeType::Circle;
}

void CircleShape::serialize(QDataStream &out) const {
    out << m_center << m_radius_x << m_radius_y << m_pen << m_brush;
}

void CircleShape::deserialize(QDataStream &in) {
    in >> m_center >> m_radius_x >> m_radius_y >> m_pen >> m_brush;
}

bool CircleShape::contains(const QPointF &point) const{
    QRectF boundingRect(
        m_center.x() - m_radius_x,
        m_center.y() - m_radius_y,
        m_radius_x * 2.0,
        m_radius_y * 2.0
    );

    QPainterPath path;
    path.addEllipse(boundingRect);
    return path.contains(point);
}

QPolygonF CircleShape::getPoints() {
    /*
        transform a circle into a 32 segments polygon;
        Will be used for the grouping operation when is necessary
        to evaluate if the selection zone overlaps a circle
    */
    QPolygonF polygon;
    unsigned int segments = 32;
    polygon.reserve(segments);
    qreal angleStep = (2.0 * M_PI) / segments;
    for (int i = 0; i < segments; ++i) {
        qreal angle = i * angleStep;
        qreal x = m_center.x() + m_radius_x * qCos(angle);
        qreal y = m_center.y() + m_radius_y * qSin(angle);
        polygon.append(QPointF(x, y));
    }
    return polygon;
}

void CircleShape::addPoint(const QPointF& p) {
}

void CircleShape::setShapeData(const ShapeData_t& shape_data) {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());
    m_radius_x = std::hypot(shape_data.end.value().x()
         - shape_data.start.value().x(), shape_data.end.value().y() - shape_data.start.value().y());
    m_radius_y = m_radius_x;
}

void CircleShape::moveRelative(const QPointF &delta) {
    m_center = m_center + delta;
}

void CircleShape::resizeShape(const QPointF &delta, const HandlePosition hp) {
    qreal startRadiusX = m_radius_x;
    qreal startRadiusY = m_radius_y;
    QPointF startCenter = m_center;
    
    switch (hp) {
        case HandlePosition::RightCenter: {
            m_radius_x = qMax(min_radius, m_radius_x + delta.x() / 2.0);
            m_center.setX(startCenter.x() + (m_radius_x - startRadiusX));
            break;
        }
        case HandlePosition::LeftCenter: {
            m_radius_x = qMax(min_radius, startRadiusX - delta.x() / 2.0);
            m_center.setX(startCenter.x() - (m_radius_x - startRadiusX));
            break;
        }
        case HandlePosition::TopCenter: {
            m_radius_y = qMax(min_radius, startRadiusY - delta.y() / 2.0);
            m_center.setY(startCenter.y() - (m_radius_y - startRadiusY));
            break;
        }
        case HandlePosition::BottomCenter: {
            m_radius_y = qMax(min_radius, startRadiusY + delta.y() / 2.0);
            m_center.setY(startCenter.y() + (m_radius_y - startRadiusY));
            break;
        }
        default:
            assert(0);
            break;
    }
}

void CircleShape::zoomInOut(const qreal& factor) {
    m_radius_x = m_radius_x * factor;
    m_radius_y = m_radius_y * factor;
}

void CircleShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
}

HandlePosition CircleShape::hookTest(const QPointF& pt) const {
    // AI generated and adapted code
    if (m_radius_x <= 0.0 || m_radius_y <= 0) 
        return HandlePosition::None;

    // Calculate the 4 cardinal points on the circle boundary
    const QPointF topCenter(m_center.x(), m_center.y() - m_radius_y);
    const QPointF rightCenter(m_center.x() + m_radius_x, m_center.y());
    const QPointF bottomCenter(m_center.x(), m_center.y() + m_radius_y);
    const QPointF leftCenter(m_center.x() - m_radius_x, m_center.y());

    const qreal total_size = handle_size + (hit_padding * 2.0);
    auto makeHitRect = [total_size](const QPointF& center) {
        return QRectF(
            center.x() - total_size / 2.0,
            center.y() - total_size / 2.0,
            total_size,
            total_size
        );
    };

    if (makeHitRect(topCenter).contains(pt))
        return HandlePosition::TopCenter;
    else if (makeHitRect(rightCenter).contains(pt))
        return HandlePosition::RightCenter;
    else if (makeHitRect(bottomCenter).contains(pt))
        return HandlePosition::BottomCenter;
    else if (makeHitRect(leftCenter).contains(pt))
        return HandlePosition::LeftCenter;
    else
        return HandlePosition::None;
}

// ========================== POLYGON SHAPE ==========================
PolygonShape::PolygonShape(const QPen &pen, const QBrush& brush)
    : Shape(pen, brush) {
}

void PolygonShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    if (m_points.isEmpty()) {
        return;
    }
    painter.drawPolygon(m_points);
}

std::unique_ptr<Shape> PolygonShape::clone() const {
    return std::make_unique<PolygonShape>(*this);
}

void PolygonShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());
    assert(shape_data.points.has_value());

    painter.setPen(shape_data.pencil);
    painter.setBrush(shape_data.brush);
    // Draw existing segments using drawPolyline
    painter.drawPolyline(m_points);
    painter.drawLine((*shape_data.points).last(), *shape_data.end);
    // Highlight individual vertices with small circles
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    for (const QPointF &pt : *shape_data.points) {
        painter.drawEllipse(pt, 3, 3);
    }
}

void PolygonShape::drawSelect(QPainter &painter, const bool draw_hooks) const {
    painter.setPen(m_shape_select_pencil);
    painter.setBrush(m_shape_select_brush);
    painter.drawPolygon(m_points);

    QRectF frameRect = m_points.boundingRect();
    const QPointF topCenter(frameRect.center().x(), frameRect.top());
    const QPointF rightCenter(frameRect.right(), frameRect.center().y());
    const QPointF bottomCenter(frameRect.center().x(), frameRect.bottom());
    const QPointF leftCenter(frameRect.left(), frameRect.center().y());

    const QPointF handleMidpoints[] = {
        topCenter,
        rightCenter,
        bottomCenter,
        leftCenter
    };
    if (draw_hooks) {
        painter.setPen(m_hook_pen);
        painter.setBrush(m_hook_brush);
        for (const QPointF& center : handleMidpoints) {
            QRectF handleRect(
                center.x() - handle_size / 2.0,
                center.y() - handle_size / 2.0,
                handle_size,
                handle_size
            );
            painter.drawRect(handleRect);
        }
    }
}

ShapeType PolygonShape::type() const { 
    return ShapeType::Polygon;
}

void PolygonShape::serialize(QDataStream &out) const {
    out << m_points << m_pen << m_brush;
}

void PolygonShape::deserialize(QDataStream &in) {
    in >> m_points >> m_pen >> m_brush;
}

bool PolygonShape::contains(const QPointF &point) const{
    QRectF frameRect = m_points.boundingRect();
    return frameRect.contains(point) 
        || m_points.containsPoint(point, Qt::OddEvenFill);
}

void PolygonShape::addPoint(const QPointF& qpoint) {
    m_points.append(qpoint);
}

QPolygonF PolygonShape::getPoints() {
    return m_points;
}

void PolygonShape::setShapeData(const ShapeData_t& shape_data) {
}

void PolygonShape::moveRelative(const QPointF &delta) {
    m_points.translate(delta);
}

void PolygonShape::resizeShape(const QPointF &delta, const HandlePosition hp) {
    // AI generated and adapted code
    if (m_points.isEmpty())
        return;

    // 1. Calculate enlarge amounts from delta
    qreal topEnlarge    = -delta.y(); // Moving UP (-y) ENLARGES top
    qreal bottomEnlarge =  delta.y(); // Moving DOWN (+y) ENLARGES bottom
    qreal leftEnlarge   = -delta.x(); // Moving LEFT (-x) ENLARGES left
    qreal rightEnlarge  =  delta.x(); // Moving RIGHT (+x) ENLARGES right

    // 2. Get original framing bounding rectangle
    QRectF origRect = m_points.boundingRect();
    if (origRect.width() <= 0 || origRect.height() <= 0)
        return;

    // 3. Construct new target bounding rectangle based on active handle
    QRectF newRect = origRect;

    switch (hp) {
        case HandlePosition::TopCenter:
            newRect.setTop(origRect.top() - topEnlarge);
            break;
        case HandlePosition::BottomCenter:
            newRect.setBottom(origRect.bottom() + bottomEnlarge);
            break;
        case HandlePosition::LeftCenter:
            newRect.setLeft(origRect.left() - leftEnlarge);
            break;
        case HandlePosition::RightCenter:
            newRect.setRight(origRect.right() + rightEnlarge);
            break;
        default:
            return;
    }

    // Guard against flipping or zero size (minimum size threshold)
    constexpr qreal minSize = 10.0;
    if (newRect.width() < minSize || newRect.height() < minSize) {
        return;
    }

    // 4. Remap each vertex of the polygon to the new bounding rectangle
    QPolygonF initialPoly = m_points;
    m_points.clear();
    m_points.reserve(m_points.size());

    for (const QPointF& pt : initialPoly) {
        // Calculate normalized position (0.0 to 1.0) inside old bounding rect
        qreal normX = (pt.x() - origRect.left()) / origRect.width();
        qreal normY = (pt.y() - origRect.top()) / origRect.height();
        // Map normalized position to new bounding rect
        qreal newX = newRect.left() + normX * newRect.width();
        qreal newY = newRect.top() + normY * newRect.height();
        m_points.append(QPointF(newX, newY));
    }
}

void PolygonShape::zoomInOut(const qreal& factor) {
    if (m_points.isEmpty())
        return;
    QPointF center = m_points.boundingRect().center();
    QTransform transform;
    transform.translate(center.x(), center.y()); // Shift center to (0,0)
    transform.scale(factor, factor);             // Scale
    transform.translate(-center.x(), -center.y()); // Shift back
    m_points = transform.map(m_points);
}

void PolygonShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
}

HandlePosition PolygonShape::hookTest(const QPointF& pt) const {
    // AI generated and adapted code
    if (m_points.isEmpty()) 
        return HandlePosition::None;

    QRectF frameRect = m_points.boundingRect();
    const QPointF topCenter(frameRect.center().x(), frameRect.top());
    const QPointF rightCenter(frameRect.right(), frameRect.center().y());
    const QPointF bottomCenter(frameRect.center().x(), frameRect.bottom());
    const QPointF leftCenter(frameRect.left(), frameRect.center().y());

    // Optional padding (in pixels) to make small handles easier to hit/click
    const qreal total_size = handle_size + (hit_padding * 2.0);
    auto makeHitRect = [total_size](const QPointF& center) {
        return QRectF(
            center.x() - total_size / 2.0,
            center.y() - total_size / 2.0,
            total_size,
            total_size
        );
    };

    if (makeHitRect(topCenter).contains(pt))
        return HandlePosition::TopCenter;
    else if (makeHitRect(rightCenter).contains(pt))
        return HandlePosition::RightCenter;
    else if (makeHitRect(bottomCenter).contains(pt))
        return HandlePosition::BottomCenter;
    else if (makeHitRect(leftCenter).contains(pt))
        return HandlePosition::LeftCenter;
    else
        return HandlePosition::None;
}

// ========================== ARC SHAPE ==========================
ArcShape::ArcShape(const QPen &pen, const QBrush& brush)
    : Shape(pen, brush) {
}

void ArcShape::drawHandle(QPainter& painter, const QPointF& pt, const QString& label, bool isDragging) const {
    painter.save();
    double m_handleRadius = 7.0;
    QColor fill = isDragging ? QColor(255, 120, 0) : QColor(30, 144, 255);
    painter.setBrush(fill);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(pt, m_handleRadius, m_handleRadius);
    painter.setPen(Qt::black);
    //painter.drawText(pt + QPointF(10, 5), label);
    painter.restore();
}

std::unique_ptr<Shape> ArcShape::clone() const {
    return std::make_unique<ArcShape>(*this);
}

void ArcShape::drawArc(QPainter &painter, const QPen& pencil, DrawingMode dm) const {
    // AI generated and adapted
    QLineF chordLine(m_startPoint, m_endPoint);
    double d = chordLine.length();

    if (qFuzzyIsNull(d)) {
        drawHandle(painter, m_startPoint, "Start / End", false);
        return;
    }
    // Calculate Midpoint of chord
    QPointF chordMid = chordLine.center();
    // Fixed height = 1/2 of distance
    double h = d / 2.0;
    // Calculate perpendicular normal vector pointing "upward" relative to the line
    QPointF dir = (m_endPoint - m_startPoint) / d;
    QPointF normal(-dir.y(), dir.x()); // 90 degree CCW rotation
    // Apex point of the arc
    QPointF pMid = chordMid + normal * h;
    // Circle Radius: R = h/2 + d^2 / (8*h)
    double R = (h / 2.0) + (d * d) / (8.0 * h); // Equals 0.625 * d
    // Center of Circle (offset from chord midpoint along normal)
    QPointF center = chordMid + normal * (h - R);
    // Angles in degrees (converting top-left origin Y to standard math coordinates)
    double aStart = qRadiansToDegrees(qAtan2(-(m_startPoint.y() - center.y()), m_startPoint.x() - center.x()));
    double aMid   = qRadiansToDegrees(qAtan2(-(pMid.y() - center.y()), pMid.x() - center.x()));
    double aEnd   = qRadiansToDegrees(qAtan2(-(m_endPoint.y() - center.y()), m_endPoint.x() - center.x()));
    // Calculate sweep angle directing through the apex point
    double sweep = aEnd - aStart;
    if (sweep < 0) sweep += 360.0;
    double midSweep = aMid - aStart;
    if (midSweep < 0) midSweep += 360.0;
    if (midSweep > sweep) {
        sweep -= 360.0;
    }
    // 7. Render Arc
    QRectF bounds(center.x() - R, center.y() - R, 2 * R, 2 * R);
    painter.setPen(pencil);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(bounds, qRound(aStart * 16.0), qRound(sweep * 16.0));
    if(dm == DrawingMode::Preview) {
        // Render Chord baseline (dashed guide line)
        painter.setPen(QPen(QColor(Qt::gray), 1, Qt::DashLine));
        painter.drawLine(m_startPoint, m_endPoint);
        // Render Draggable End Handles
        drawHandle(painter, m_startPoint, "Start", false);
        drawHandle(painter, m_endPoint, "End", true);
    }
}

void ArcShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    drawArc(painter, m_pen, DrawingMode::Final);
}

void ArcShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    // working assumption
    assert(shape_data.start.has_value());
    assert(shape_data.end.has_value());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(shape_data.pencil);
    painter.setBrush(shape_data.brush);
    m_startPoint = *shape_data.start;
    m_endPoint = *shape_data.end;
    drawArc(painter, shape_data.pencil, DrawingMode::Preview);
}

void ArcShape::drawSelect(QPainter &painter, const bool draw_hooks) const {
    painter.setPen(m_shape_select_pencil);
    drawArc(painter, m_shape_select_pencil, DrawingMode::Preview);
}

ShapeType ArcShape::type() const { 
    return ShapeType::Arc;
}

void ArcShape::serialize(QDataStream &out) const {
    out << m_pen << m_brush << m_startPoint << m_endPoint;
}

void ArcShape::deserialize(QDataStream &in) {
    in >> m_pen >> m_brush >> m_startPoint >> m_endPoint;
}

bool ArcShape::contains(const QPointF &pt) const {
    // AI generated code !
    double tolerance = 6.0;
    QLineF chordLine(m_startPoint, m_endPoint);
    double d = chordLine.length();

    // Degenerate case: Start and End are the same point
    if (qFuzzyIsNull(d)) {
        return QLineF(pt, m_startPoint).length() <= tolerance;
    }

    // TODO some vars already computed
    // 1. Recompute Circle Center & Radius (matching drawArc)
    QPointF chordMid = chordLine.center();
    double h = d / 2.0;
    QPointF dir = (m_endPoint - m_startPoint) / d;
    QPointF normal(-dir.y(), dir.x()); // 90 degree CCW rotation
    QPointF pMid = chordMid + normal * h;

    double R = (h / 2.0) + (d * d) / (8.0 * h); // Equals 0.625 * d
    // m_centerPoint will be used in zoomInOut
    m_centerPoint = chordMid + normal * (h - R);

    // 2. Distance Check: Is point near the circumference?
    double distToCenter = QLineF(pt, m_centerPoint).length();
    if (std::abs(distToCenter - R) > tolerance) {
        return false; // Point is too far from the circular arc line
    }

    // 3. Angular Check: Is point within the arc's angular sweep?
    double aStart = qRadiansToDegrees(qAtan2(-(m_startPoint.y() - m_centerPoint.y()), m_startPoint.x() - m_centerPoint.x()));
    double aMid   = qRadiansToDegrees(qAtan2(-(pMid.y() - m_centerPoint.y()), pMid.x() - m_centerPoint.x()));
    double aEnd   = qRadiansToDegrees(qAtan2(-(m_endPoint.y() - m_centerPoint.y()), m_endPoint.x() - m_centerPoint.x()));

    // Normalize angles to [0, 360)
    auto normalize = [](double angle) {
        angle = std::fmod(angle, 360.0);
        return (angle < 0) ? angle + 360.0 : angle;
    };

    double sweep = aEnd - aStart;
    if (sweep < 0) sweep += 360.0;

    double midSweep = aMid - aStart;
    if (midSweep < 0) midSweep += 360.0;

    if (midSweep > sweep) {
        sweep -= 360.0;
    }

    // Calculate click angle relative to start angle
    double clickAngle = qRadiansToDegrees(qAtan2(-(pt.y() - m_centerPoint.y()), pt.x() - m_centerPoint.x()));
    double relativeClick = normalize(clickAngle - aStart);

    // If sweep is positive (CCW), relative angle must be between 0 and sweep
    // If sweep is negative (CW), relative angle normalized must be between 360 + sweep and 360
    if (sweep >= 0) {
        return relativeClick <= sweep;
    } else {
        return relativeClick >= (360.0 + sweep);
    }
}

void ArcShape::addPoint(const QPointF& qpoint) {
}

QPolygonF ArcShape::getPoints() {
    /*
        AI generated method
        Turn the Arc shape into a polygon
    */
    QPolygonF poly;
    uint32_t segments = 32;

    QLineF chordLine(m_startPoint, m_endPoint);
    double d = chordLine.length();

    // Degenerate case: single point
    if (qFuzzyIsNull(d)) {
        poly << m_startPoint;
        return poly;
    }

    // 1. Recompute Circle Geometry (matching drawArc)
    QPointF chordMid = chordLine.center();
    double h = d / 2.0;

    QPointF dir = (m_endPoint - m_startPoint) / d;
    QPointF normal(-dir.y(), dir.x()); // 90 degree CCW rotation

    QPointF pMid = chordMid + normal * h;
    double R = (h / 2.0) + (d * d) / (8.0 * h); // Equals 0.625 * d
    QPointF center = chordMid + normal * (h - R);

    // 2. Compute Angles (converted to radians for qCos/qSin)
    double aStartRad = qAtan2(-(m_startPoint.y() - center.y()), m_startPoint.x() - center.x());
    double aMidRad   = qAtan2(-(pMid.y() - center.y()), pMid.x() - center.x());
    double aEndRad   = qAtan2(-(m_endPoint.y() - center.y()), m_endPoint.x() - center.x());

    double aStartDeg = qRadiansToDegrees(aStartRad);
    double aMidDeg   = qRadiansToDegrees(aMidRad);
    double aEndDeg   = qRadiansToDegrees(aEndRad);

    double sweepDeg = aEndDeg - aStartDeg;
    if (sweepDeg < 0) sweepDeg += 360.0;

    double midSweepDeg = aMidDeg - aStartDeg;
    if (midSweepDeg < 0) midSweepDeg += 360.0;

    if (midSweepDeg > sweepDeg) {
        sweepDeg -= 360.0;
    }

    // Convert back to radians for sampling steps
    double startRad = qDegreesToRadians(aStartDeg);
    double sweepRad = qDegreesToRadians(sweepDeg);

    // Ensure at least 2 segments
    segments = std::max<unsigned>(2, segments);

    // 3. Sample Points Along the Arc
    poly.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments; // Interpolation factor [0.0, 1.0]
        double currentAngleRad = startRad + t * sweepRad;

        // Note: Y is inverted (- R * qSin) to match Qt screen coordinates
        double x = center.x() + R * qCos(currentAngleRad);
        double y = center.y() - R * qSin(currentAngleRad);

        poly.append(QPointF(x, y));
    }

    // Explicitly lock exact endpoints to prevent floating point drift
    if (!poly.isEmpty()) {
        poly.first() = m_startPoint;
        poly.last()  = m_endPoint;
    }

    return poly;
}

void ArcShape::setShapeData(const ShapeData_t& shape_data) {
}

void ArcShape::moveRelative(const QPointF &delta) {
    m_startPoint += delta;
    m_endPoint += delta;
}

void ArcShape::resizeShape(const QPointF &delta, const HandlePosition hp) {
    // AI generated and adapted code
}

void ArcShape::zoomInOut(const qreal& factor) {
    // m_centerPoint computed in contains() method
    m_startPoint = m_centerPoint + (m_startPoint - m_centerPoint) * factor;
    // Scale end point relative to centerPoint
    m_endPoint = m_centerPoint + (m_endPoint - m_centerPoint) * factor;
}

void ArcShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
}

HandlePosition ArcShape::hookTest(const QPointF& pt) const {
    // AI generated and adapted code
    /*
    if (m_points.isEmpty()) 
        return HandlePosition::None;

    QRectF frameRect = m_points.boundingRect();
    const QPointF topCenter(frameRect.center().x(), frameRect.top());
    const QPointF rightCenter(frameRect.right(), frameRect.center().y());
    const QPointF bottomCenter(frameRect.center().x(), frameRect.bottom());
    const QPointF leftCenter(frameRect.left(), frameRect.center().y());

    // Optional padding (in pixels) to make small handles easier to hit/click
    const qreal total_size = handle_size + (hit_padding * 2.0);
    auto makeHitRect = [total_size](const QPointF& center) {
        return QRectF(
            center.x() - total_size / 2.0,
            center.y() - total_size / 2.0,
            total_size,
            total_size
        );
    };

    if (makeHitRect(topCenter).contains(pt))
        return HandlePosition::TopCenter;
    else if (makeHitRect(rightCenter).contains(pt))
        return HandlePosition::RightCenter;
    else if (makeHitRect(bottomCenter).contains(pt))
        return HandlePosition::BottomCenter;
    else if (makeHitRect(leftCenter).contains(pt))
        return HandlePosition::LeftCenter;
    else
        return HandlePosition::None; */
   return HandlePosition::None;
}
