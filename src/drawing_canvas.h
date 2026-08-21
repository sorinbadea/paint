#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H
#include "shapes.h"
#include <QWidget>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QPointF>
#include <QFile>
#include <vector>
#include <memory>
#include <cmath>

constexpr int pen_width = 2;
// -------------------------------------------------------------
// 3. CANVAS WIDGET
// -------------------------------------------------------------
enum class ToolMode { Line, Circle, Select };

class DrawingCanvas : public QWidget {
    Q_OBJECT

public:
    explicit DrawingCanvas(QWidget *parent);
    void setMode(ToolMode mode);
    void undoLast();
    void clearAll();
    // serialize deserialize from/to file
    bool saveToFile(const QString &filePath) const;
    bool loadFromFile(const QString &filePath);
    void setPenWidth(int width);
    
protected:
    void paintGrid(QPainter& painter, unsigned grid_width);
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override ;
    void paintShape(const ShapeData_t &shapeData, const QPen& pen, const QBrush& brush, QPainter& painter);

private:
    ToolMode m_mode;
    bool m_isDrawing;
    int m_pen_width;
    QPointF m_startPos;
    QPointF m_currentPos;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    Shape* m_selectedShape;
    QColor m_drawing_color;
    QColor m_preview_color;
    QColor m_selected_color;
    QBrush m_drawing_qbrush;
    QBrush m_preview_qbrush;
    QBrush m_select_brush;
    QAction m_select_action;
    QPen m_shape_pen;
};

#endif // DRAWINGCANVAS_H