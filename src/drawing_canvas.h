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
constexpr qreal clone_x_offset = 10.0;
constexpr qreal clone_y_offset = 10.0;

// -------------------------------------------------------------
// 3. CANVAS WIDGET
// -------------------------------------------------------------
enum class ToolMode { Line, 
                    Circle, 
                    Rectangle,
                    Polygon,
                    Select,
                    Group,
                    None
                };
constexpr ShapeType getShapeType(ToolMode tm) noexcept;

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
    void setZoomFactor(const qreal& zoom_factor);
    // getter
    Shape* isShapeSelected() const;
    // context menu actions
    void cloneShape();   // Copy/Paste
    void restoreShape(); // Restore shape properties
    void removeShape();  // Remove shape 
    void zommInOut(const qreal& factor);
    
protected:
    // methods called by QT
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // paint grid
    void paintGrid(QPainter& painter, unsigned grid_width);

    // - add the new shape on the Shape's list, restore brush and pen;
    // - handle the case of finalizing the grouping rectangle;
    void finalizeShape();

    // returns true if one of the 4 surounding hooks is clicked
    inline bool hookSelected(HandlePosition h) const;

    // Attributes
    // drawing mode, Circle, Line, Selection..
    ToolMode m_mode;

    // indicates if a new shape is drawing or grouping operation
    bool m_isDrawing;
    bool m_grouping;

    // position
    QPointF m_start_pos;
    QPointF m_current_pos;
    QPointF m_start_zoom_pos;
    HandlePosition m_handle;

    /*
        The shapes beeing grouped
    */
    std::list<Shape*> m_group_shapes;

    // list of existing shapes, used when paintEvent is called
    std::list<std::unique_ptr<Shape>> m_shapes;

    /*
        new shape beeing drawn, including grouping rectangle
    */
    std::unique_ptr<Shape> m_shape;

    /* 
        points to the selected shape,
        can be zoomed-in zoomed-out, moved or removed
    */
    Shape* m_selected_shape;

    // drawing color, brush and pen width
    int m_pen_width;
    QColor m_drawing_color;
    QColor m_selected_color;
    QColor m_brush_color;
    QBrush m_select_brush;
    QBrush m_grouping_brush;

    /*
        store the pen and brush either for the selected shape
        or for a group of selected shapes
    */
    struct PenBrush {
        QPen pen;
        QBrush brush;
    };
    std::map<Shape*, struct PenBrush> m_saved_pen_brush;

    // zoom factor
    qreal m_zoom_factor;
};
#endif // DRAWINGCANVAS_H