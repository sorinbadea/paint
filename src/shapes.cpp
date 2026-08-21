// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line & Circle)
// -------------------------------------------------------------
#include "shapes.h"

// ========================== LINE SHAPE ==========================
LineShape::LineShape(const QLineF &line, const QPen &pen)
    : m_line(line), m_pen(pen) {}

void LineShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.drawLine(m_line);
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

void LineShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void LineShape::moveRelative(const QPointF &delta) {
    m_line.setP1(m_line.p1() + delta);
    m_line.setP2(m_line.p2() + delta);
}

// ========================== CIRCLE SHAPE ==========================
CircleShape::CircleShape(const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush)
        : m_center(center), m_radius(radius), m_pen(pen), m_brush(brush) {}

void CircleShape::draw(QPainter &painter) const {
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    painter.drawEllipse(m_center, m_radius, m_radius);
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

void CircleShape::setPen(const QPen &pen) {
    m_pen = pen;
}

void CircleShape::moveRelative(const QPointF &delta) {
    m_center = m_center + delta;
}
