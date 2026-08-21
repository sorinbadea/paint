#ifndef SHAPES_H
#define SHAPES_H

#include <QPainter>
#include <QPen>
#include <QBrush>

constexpr double EPSILON = 1.0; // Tolerance for point-on-line checks

enum class ShapeType : quint32 {
    Line = 1,
    Circle = 2
};

typedef struct {
    ShapeType type;
    std::optional<QPointF> start;
    std::optional<QPointF> end;
    std::optional<qreal> radius;
} ShapeData_t;

// -------------------------------------------------------------
// 1. ABSTRACT BASE SHAPE CLASS
// -------------------------------------------------------------
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw(QPainter &painter) const = 0;
    virtual ShapeType type() const = 0;
    virtual void serialize(QDataStream &out) const = 0;
    virtual void deserialize(QDataStream &in) = 0;
    virtual bool contains(const QPointF &point) const = 0;
    virtual QPen getPen() const = 0;
    virtual void setPen(const QPen &pen) = 0;
    virtual void moveRelative(const QPointF &delta) = 0;
};

// -------------------------------------------------------------
// 2. CONCRETE SHAPES (Line, Circle...)
// -------------------------------------------------------------
class LineShape : public Shape {
public:
    LineShape() = default;
    LineShape(const QLineF &line, const QPen &pen);
    void draw(QPainter &painter) const override;
    ShapeType type() const override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
    bool contains(const QPointF &point) const override;
    QPen getPen() const override;
    void setPen(const QPen &pen) override;
    void moveRelative(const QPointF &delta) override;

private:
    //coordinates
    QLineF m_line;
    //pen brushes..
    QPen m_pen;
};

class CircleShape : public Shape {
public:
    CircleShape() = default;
    CircleShape(const QPointF &center, qreal radius, const QPen &pen, const QBrush &brush);
    void draw(QPainter &painter) const override;
    ShapeType type() const override;
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
    bool contains(const QPointF &point) const override;
    void setPen(const QPen &pen) override;
    QPen getPen() const override;
    void moveRelative(const QPointF &delta) override;

private:
    //pen brushes..
    QPen m_pen;
    QBrush m_brush;
    //coordinates and radius
    QPointF m_center;
    qreal m_radius;
};

#endif