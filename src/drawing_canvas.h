#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H
#include "shapes.h"
#include "position_manager.h"
#include <QToolTip>
#include <QWidget>
#include <QMouseEvent>
#include <QFile>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

constexpr int default_pen_width = 2;
constexpr unsigned grid_width = 16;
constexpr qreal clone_x_offset = 10.0;
constexpr qreal clone_y_offset = 10.0;
const std::vector<int> preset_widths = {1, 2, 3, 4, 5, 7, 9, 11, 12};
constexpr qreal text_width = 100.0;
constexpr qreal text_height = 30.0;

// -------------------------------------------------------------
// 3. CANVAS WIDGET
// -------------------------------------------------------------
enum class ToolMode { Line, Circle,  Rectangle, Polygon, Arc, Text, Select, Group, None};

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
    void setDrawingProperties(const drawing_properties_t& data);
    void setShapeZoomFactor(const qreal& zoom_factor);
    void setGroupZoomFactor(const qreal& zoom_factor);
    // getters
    Shape* isShapeSelected() const;
    bool isGrouping() const;
    ToolMode getToolMode() const;
    const QColor& getDrawingColor() const;
    const unsigned& getPenWidth() const;
    // context menu actions
    void cloneShape();   // Copy/Paste
    void cloneGroup();   // Clone all grouped shapes
    void restoreShape(); // Restore shape properties
    void removeShape();  // Remove selected shape
    void removeGroup();  // Remove grouped shapes
    void zommInOut(const qreal& factor);
    
protected:
    // methods called by QT
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // paint grid
    void paintGrid(QPainter& painter, unsigned grid_width);

    // - add the new shape on the Shape's list, restore brush and pen;
    // - handle the case of finalizing the grouping rectangle;
    void finalizeShape(std::optional<QPointF> const&  point);

    // returns true if one of the 4 surounding hooks is clicked
    bool hookSelected(HandlePosition h) const;

    // check if a Shape is below the cursor
    Shape* getSelectedShape(const QPointF& point);

    // implement drawing mode
    void drawingMode(const QMouseEvent *event);

    // console log
    void logging(const std::string& message);

    // return true if the shape beeing draw is
    // Line, Rectangle, Circle, Arc or grouping (draw in one shot, except Polygon)
    bool oneShotShape() const;

    // return true for drawing shapes
    bool drawingShape() const;

    // open a QLineEdit widget
    void openTextLineEditor(TextShape* shape, const QPointF& position);

    // mouse events handling by the selected shape
    void handleMousePressEvent(const QPointF&);
    void handleMouseMoveEvent(const QPointF&);

    // Attributes
    // drawing mode, Circle, Line, Selection..
    ToolMode m_mode;

    // indicates if a new shape is drawing or grouping operation
    bool m_isDrawing;
    bool m_grouping;

    // Hook handle position, CenterUp, CenterDown, MiddleRight, etc...
    HandlePosition m_handle;

    /*
        The shapes beeing grouped
    */
    std::list<Shape*> m_group_shapes;

    // store the position of the selected shape in the list of shapes
    std::list<std::unique_ptr<Shape>>::iterator m_select_shape_it;
    std::list<std::unique_ptr<Shape>>::iterator m_original_next_it;

    // list of existing shapes
    std::list<std::unique_ptr<Shape>> m_shapes;

    /*
        new shape beeing drawn, including grouping rectangle
    */
    std::unique_ptr<Shape> m_shape;

    /* 
        points to the selected shape,
        can be zoomed-in zoomed-out, moved, resized, cloned or deleted
    */
    Shape*       m_selected_shape;

    // drawing color, brush and pen style and width
    unsigned     m_pen_width;
    QColor       m_drawing_color;
    QColor       m_selected_color;
    QColor       m_brush_color;
    QBrush       m_select_brush;
    QBrush       m_grouping_brush;
    Qt::PenStyle m_pen_style;

    // zoom factor, <1 zoom in, >1 zoom out
    qreal        m_zoom_factor;

    // Text editor
    QLineEdit*   m_active_text;
    std::unique_ptr<Shape> m_currentEditingShape;

    // Position manager, store drawing start/stop position, start resize position
    PositionManager m_position_mgr;
};

double computeRotationAngle(QPointF startVector, QPointF endVector);

#endif // DRAWINGCANVAS_H

