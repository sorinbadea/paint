// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line & Circle)
// -------------------------------------------------------------
#include "shapes.h"
#include <QToolTip>

// ========================== LINE SHAPE ==========================
LineShape::LineShape(const QLineF &line, const QPen &pen)
    : m_line(line), m_pen(pen) {}

void LineShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.drawLine(m_line);
}

void LineShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    QLineF line(*shape_data.start, *shape_data.end);
    painter.setPen(m_pen);
    painter.drawLine(line);
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

QPen LineShape::getPen() const {
    return m_pen;
}

QBrush LineShape::getBrush() const {
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

// ========================== RECTANGLE SHAPE ==========================
RectangleShape::RectangleShape(const QRectF &rectangle, const QPen &pen)
    : m_rectangle(rectangle), m_pen(pen) {
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

QPen RectangleShape::getPen() const {
    return m_pen;
}

QBrush RectangleShape::getBrush() const {
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

// ========================== CIRCLE SHAPE ==========================
CircleShape::CircleShape(const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush)
        : m_center(center), m_radius(radius), m_pen(pen), m_brush(brush) {}

void CircleShape::draw(QPainter &painter) const {
    if (m_radius >0) {
        painter.setPen(m_pen);
        painter.setBrush(m_brush);
        painter.drawEllipse(m_center, m_radius, m_radius);
    }
}

void CircleShape::draw(QPainter &painter, const ShapeData_t& shape_data) const {
    if (shape_data.radius > 0) {
        painter.setPen(m_pen);
        painter.setBrush(m_brush);
        painter.drawEllipse(*shape_data.start, *shape_data.radius, *shape_data.radius);
    }
}

ShapeType CircleShape::type() const { 
    return ShapeType::Circle;
}

void CircleShape::serialize(QDataStream &out) const {
    out << m_center << m_radius << m_pen << m_brush;
}

void CircleShape::deserialize(QDataStream &in) {
    in >> m_center >> m_radius >> m_pen >> m_brush;
}

bool CircleShape::contains(const QPointF &point) const{
    // verify if the point is within the circle by checking 
    // the distance from the center
    qreal distance = QLineF(m_center, point).length();
    return distance <= m_radius;
}

QPen CircleShape::getPen() const {
    return m_pen;
}

QBrush CircleShape::getBrush() const {
    return m_brush;
}

void CircleShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void CircleShape::setBrush(const QBrush &brush) {
    m_brush = brush;
}

void CircleShape::addPoint(const QPointF& p) {
}

void CircleShape::setShapeData(const ShapeData_t& shape_data) {
    if (shape_data.start && shape_data.end) {
        m_radius = std::hypot(shape_data.end.value().x()
        - shape_data.start.value().x(), shape_data.end.value().y() - shape_data.start.value().y());
    }
}

void CircleShape::moveRelative(const QPointF &delta) {
    m_center = m_center + delta;
}

void CircleShape::zoomInOut(const qreal& factor) {
    m_radius = m_radius * factor;
}

void CircleShape::toolHint(const QPoint &point, const QString& explanation) {
    // Display the tooltip near the cursor
    // Parameters: pos, text, widget parent, rect boundary, duration in ms
    QToolTip::showText(point + QPoint(10, 10), explanation, nullptr, QRect(), 2000);
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
    return m_points.containsPoint(point, Qt::OddEvenFill);
}

QPen PolygonShape::getPen() const {
    return m_pen;
}

QBrush PolygonShape::getBrush() const {
    return m_brush;
}

void PolygonShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void PolygonShape::setBrush(const QBrush &brush) {
    m_brush = brush;
}

void PolygonShape::addPoint(const QPointF& qpoint) {
    // qDebug() << "adding points";
    m_points.append(qpoint);
}

void PolygonShape::setShapeData(const ShapeData_t& shape_data) {
    /*if (shape_data.start && shape_data.end) {
        m_rectangle.setTopLeft(shape_data.start.value());
        m_rectangle.setBottomRight(shape_data.end.value());
        m_rectangle = m_rectangle.normalized();
    }*/
}

void PolygonShape::moveRelative(const QPointF &delta) {
    m_points.translate(delta);
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
