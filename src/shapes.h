#ifndef SHAPES_H
#define SHAPES_H

#include <QPainter>
#include <QPen>
#include <QBrush>

constexpr double EPSILON = 1.0; // Tolerance for point-on-line checks

enum class ShapeType : quint32 {
    Line = 1,
    Circle = 2,
    Rectangle = 3,
    Polygon = 4,
    None
};

typedef struct {
    ShapeType type;
    std::optional<QPointF> start;
    std::optional<QPointF> end;
    std::optional<qreal> radius;
    std::optional<QPolygonF> points;
} ShapeData_t;

// -------------------------------------------------------------
// 1. ABSTRACT BASE SHAPE CLASS
// -------------------------------------------------------------
class Shape {
public:
    virtual ~Shape() = default;
    //draw methods
    virtual void draw(QPainter &painter) const = 0;
    virtual void draw(QPainter &painter, const ShapeData_t& shape_data) const = 0;
    // check if the current position overlaps an existing shape
    virtual bool contains(const QPointF &point) const = 0;
    // getter setters
    virtual ShapeType type() const = 0;
    virtual QPen getPen() const = 0;
    virtual QBrush getBrush() const = 0;
    virtual void setPen(const QPen &pen) = 0;
    virtual void setBrush(const QBrush &brush) = 0;
    virtual void setShapeData(const ShapeData_t& shape_data) = 0;
    virtual void addPoint(const QPointF& p) = 0;
    // for selecting and moving a shape
    virtual void moveRelative(const QPointF &delta) = 0;
    virtual void zoomInOut(const qreal& factor) = 0;
    // tool tip
    virtual void toolHint(const QPoint &point, const QString& explanation) = 0;
    // seralize, de-serialize
    virtual void serialize(QDataStream &out) const = 0;
    virtual void deserialize(QDataStream &in) = 0;
};

// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line, Circle...)
// -------------------------------------------------------------
class LineShape : public Shape {
public:
    LineShape() = default;
    LineShape(const QLineF &line, const QPen &pen);
    void draw(QPainter &painter) const override;
    void draw(QPainter &painter, const ShapeData_t& shape_data) const override;
    ShapeType type() const override;
    bool contains(const QPointF &point) const override;
    // getter setters
    QPen getPen() const override;
    QBrush getBrush() const override;
    void setPen(const QPen &pen) override;
    void setBrush(const QBrush &brush) override;
    void setShapeData(const ShapeData_t& shape_data) override;
    void addPoint(const QPointF& p) override;
    void moveRelative(const QPointF &delta) override;
    void zoomInOut(const qreal& factor) override;
    void toolHint(const QPoint &point, const QString& explanation) override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

private:
    //coordinates
    QLineF m_line;
    //pen brushes..
    QPen m_pen;
    QBrush m_brush;
};

class CircleShape : public Shape {
public:
    CircleShape() = default;
    CircleShape(const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush);
    void draw(QPainter &painter) const override;
    void draw(QPainter &painter, const ShapeData_t& shape_data) const override;
    ShapeType type() const override;
    bool contains(const QPointF &point) const override;
    void setPen(const QPen &pen) override;
    void setBrush(const QBrush &brush) override;
    void setShapeData(const ShapeData_t& shape_data) override;
    void addPoint(const QPointF& p) override;
    QPen getPen() const override;
    QBrush getBrush() const override;
    void moveRelative(const QPointF &delta) override;
    void zoomInOut(const qreal& factor) override;
    void toolHint(const QPoint &point, const QString& explanation) override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

private:
    //pen brushes..
    QPen m_pen;
    QBrush m_brush;
    //coordinates and radius
    QPointF m_center;
    qreal m_radius;
};

class RectangleShape : public Shape {
public:
    RectangleShape() = default;
    RectangleShape(const QRectF &rect, const QPen &pen);
    void draw(QPainter &painter) const override;
    void draw(QPainter &painter, const ShapeData_t& shape_data) const override;
    ShapeType type() const override;
    bool contains(const QPointF &point) const override;
    QPen getPen() const override;
    QBrush getBrush() const override;
    void setPen(const QPen &pen) override;
    void setBrush(const QBrush &brush) override;
    void setShapeData(const ShapeData_t& shape_data) override;
    void addPoint(const QPointF& p) override;
    void moveRelative(const QPointF &delta) override;
    void zoomInOut(const qreal& factor) override;
    void toolHint(const QPoint &point, const QString& explanation) override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

private:
    //coordinates
    QRectF m_rectangle;
    //pen brushes..
    QPen m_pen;
    QBrush m_brush;
};

class PolygonShape : public Shape {
public:
    PolygonShape() = default;
    PolygonShape(const QPen &pen, const QBrush& brush);
    void draw(QPainter &painter) const override;
    void draw(QPainter &painter, const ShapeData_t& shape_data) const override;
    void drawPoly(QPainter &painter, bool is_drawing);
    ShapeType type() const override;
    bool contains(const QPointF &point) const override;
    QPen getPen() const override;
    QBrush getBrush() const override;
    void setPen(const QPen &pen) override;
    void setBrush(const QBrush &brush) override;
    void setShapeData(const ShapeData_t& shape_data) override;
    void addPoint(const QPointF& p) override;
    void moveRelative(const QPointF &delta) override;
    void zoomInOut(const qreal& factor) override;
    void toolHint(const QPoint &point, const QString& explanation) override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

private:
    //coordinates
    QPolygonF m_points;
    //pen brushes..
    QPen m_pen;
    QBrush m_brush;
};


#endif