#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H
#include "shapes.h"
#include <QWidget>
#include <QMouseEvent>
#include <QPointF>
#include <QFile>
#include <vector>
#include <memory>
#include <cmath>

constexpr int pen_width = 2;
// -------------------------------------------------------------
// 3. CANVAS WIDGET
// -------------------------------------------------------------
enum class ToolMode { Line, 
                    Circle, 
                    Rectangle,
                    Polygon,
                    Select,
                    None
                };

class DrawingCanvas : public QWidget {
    Q_OBJECT

public:
    explicit DrawingCanvas(QWidget *parent);
    void undoLast();
    void clearAll();
    // serialize deserialize from/to file
    bool saveToFile(const QString &filePath) const;
    bool loadFromFile(const QString &filePath);
    //setters
    void setMode(const ToolMode& mode);
    void setPenWidth(int width);
    void setPaintColor(const QColor& color);
    void setBrushColor(const QColor& brush);
    void keepShape();
    void removeShape();
    Shape* isShapeSelected() const;
    
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void paintGrid(QPainter& painter, unsigned grid_width);
    ShapeType getShapeType(const ToolMode& tm) const;
    void finalizeShape();

    ToolMode m_mode;
    bool m_isDrawing;
    int m_pen_width;
    QPointF m_start_pos;
    QPointF m_current_pos;
    std::list<std::unique_ptr<Shape>> m_shapes;
    std::unique_ptr<Shape> m_shape;
    Shape* m_selected_shape;
    QColor m_drawing_color;
    QColor m_selected_color;
    QColor m_brush_color;
    QBrush m_select_brush;
    //store the pen and the brush of the selected shape
    QPen m_shape_pen;
    QBrush m_shape_brush;
    QPolygonF m_points;
};
#endif // DRAWINGCANVAS_H