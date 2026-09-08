// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line & Circle)
// -------------------------------------------------------------
#include "shapes.h"
#include <QToolTip>

// ========================== LINE SHAPE ==========================
LineShape::LineShape(const QLineF &line, const QPen &pen, const QBrush& brush)
    : m_line(line), m_pen(pen), m_brush(brush) {}

void LineShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.drawLine(m_line);
}

void LineShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    QLineF line(*shape_data.start, *shape_data.end);
    painter.setPen(m_pen);
    painter.drawLine(line);
}

void LineShape::drawSelect(QPainter &painter) const {
    painter.setPen(m_pen);
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

const QPen& LineShape::getPen() const {
    return m_pen;
}

const QBrush& LineShape::getBrush() const {
    return m_brush;
}

void LineShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void LineShape::setBrush(const QBrush &brush) {
    m_brush = brush;
}

void LineShape::addPoint(const QPointF& p) {
}

QPolygonF LineShape::getPoints() {
    return QPolygonF({m_line.p1(), m_line.p2()});
}

void LineShape::setShapeData(const ShapeData_t& shape_data) {
    if (shape_data.start && shape_data.end) {
        m_line.setP1(shape_data.start.value());
        m_line.setP1(shape_data.end.value());
    }
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
    : m_rectangle(rectangle), m_pen(pen), m_brush(brush) {
        m_rectangle = m_rectangle.normalized();
    }

void RectangleShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    painter.drawRect(m_rectangle);
}

void RectangleShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    QRectF rect(*shape_data.start, *shape_data.end);
    rect = rect.normalized();
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    painter.drawRect(rect);
}

QRectF RectangleShape::getHandleRect(const QPointF& center) const {
    return QRectF(center.x() - handle_size / 2.0,
        center.y() - handle_size / 2.0,
        handle_size,
        handle_size);
}

void RectangleShape::drawSelect(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
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
    // draw hooking points around the shape
    for (const QPointF& handle : handleCenters) {
        painter.drawRect(getHandleRect(handle));
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

const QPen& RectangleShape::getPen() const {
    return m_pen;
}

const QBrush& RectangleShape::getBrush() const {
    return m_brush;
}

void RectangleShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void RectangleShape::setBrush(const QBrush &brush) {
    m_brush = brush;
}

void RectangleShape::addPoint(const QPointF& p) {
}

QPolygonF RectangleShape::getPoints() {
    return QPolygonF(m_rectangle);
}

void RectangleShape::setShapeData(const ShapeData_t& shape_data) {
    if (shape_data.start && shape_data.end) {
        m_rectangle.setTopLeft(shape_data.start.value());
        m_rectangle.setBottomRight(shape_data.end.value());
        m_rectangle = m_rectangle.normalized();
    }
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
    if (isNear(pt, topCenter, tolerance))    return HandlePosition::TopCenter;
    if (isNear(pt, bottomCenter, tolerance)) return HandlePosition::BottomCenter;
    if (isNear(pt, leftCenter, tolerance))   return HandlePosition::LeftCenter;
    if (isNear(pt, rightCenter, tolerance))  return HandlePosition::RightCenter;

    // 2. Inside Area & Outside
    if (m_rectangle.contains(pt)) return HandlePosition::Inside;

    return HandlePosition::None;
}

// ========================== CIRCLE SHAPE ==========================
CircleShape::CircleShape(
    const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush)
        : m_center(center),
        m_radius_x(radius),
        m_radius_y(radius),
        m_pen(pen),
        m_brush(brush) {}

void CircleShape::draw(QPainter &painter) const {
    if (m_radius_x >0 && m_radius_y >0) {
        painter.setPen(m_pen);
        painter.setBrush(m_brush);
        painter.drawEllipse(m_center, m_radius_x, m_radius_y);
    }
}

void CircleShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    if (shape_data.radius > 0) {
        painter.setPen(m_pen);
        painter.setBrush(m_brush);
        painter.drawEllipse(*shape_data.start, *shape_data.radius, *shape_data.radius);
    }
}

void CircleShape::drawSelect(QPainter &painter) const {
    if (m_radius_x <= 0.0 || m_radius_y <= 0)
        return;

    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Draw the circle outline / fill
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
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

    // 3. Draw small handle rectangles centered on each point
    painter.setPen(QPen(Qt::darkBlue, 1));
    painter.setBrush(Qt::white);

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

const QPen& CircleShape::getPen() const {
    return m_pen;
}

const QBrush& CircleShape::getBrush() const {
    return m_brush;
}

void CircleShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void CircleShape::setBrush(const QBrush &brush) {
    m_brush = brush;
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
    if (shape_data.start && shape_data.end) {
        m_radius_x = std::hypot(shape_data.end.value().x()
        - shape_data.start.value().x(), shape_data.end.value().y() - shape_data.start.value().y());
        m_radius_y = m_radius_x;
    }
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
    return HandlePosition::None;
}

// ========================== POLYGON SHAPE ==========================
PolygonShape::PolygonShape(const QPen &pen, const QBrush& brush)
    : m_pen(pen), m_brush(brush) {
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
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    // 1. Draw existing segments using drawPolyline
    painter.drawPolyline(m_points);
    painter.drawLine((*shape_data.points).last(), *shape_data.end);
    // 3. Highlight individual vertices with small circles
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    for (const QPointF &pt : *shape_data.points) {
        painter.drawEllipse(pt, 3, 3);
    }
}

void PolygonShape::drawSelect(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);

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
    
    for (const QPointF& center : handleMidpoints) {
        QRectF handleRect(
            center.x() - handle_size / 2.0,
            center.y() - handle_size / 2.0,
            handle_size,
            handle_size
        );
        painter.drawRect(handleRect);
    }
    painter.drawPolygon(m_points);
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

const QPen& PolygonShape::getPen() const {
    return m_pen;
}

const QBrush& PolygonShape::getBrush() const {
    return m_brush;
}

void PolygonShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void PolygonShape::setBrush(const QBrush &brush) {
    m_brush = brush;
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

    return HandlePosition::None;
}
