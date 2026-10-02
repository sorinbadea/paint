#include "shapes.h"
#include "drawing_canvas.h"

constexpr ShapeType getShapeType(ToolMode tm) noexcept {
    switch (tm) {
        case ToolMode::Line:
            return ShapeType::Line;
        case ToolMode::Circle:
            return ShapeType::Circle;
        case ToolMode::Rectangle:
            return ShapeType::Rectangle;
        case ToolMode::Polygon:
            return ShapeType::Polygon;
        case ToolMode::Arc:
            return ShapeType::Arc;
        default:
            return ShapeType::None;
    }
}

DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent),
    m_isDrawing(false),
    m_grouping(false),
    m_mode(ToolMode::None),
    m_selected_shape(nullptr),
    m_pen_width(2),
    m_drawing_color(Qt::blue),
    m_selected_color(Qt::darkYellow),
    m_brush_color(Qt::cyan),
    m_grouping_brush(QColor(100, 10, 0, 32)), // semi transparent brush
    m_select_brush(QBrush(Qt::darkGray, Qt::DiagCrossPattern)),
    m_pen_style(Qt::SolidLine) {
    setBackgroundRole(QPalette::Base); 
    setAutoFillBackground(true);
}

// ----- called from main.cpp as user actions -----
void DrawingCanvas::setMode(const ToolMode& mode) {
    if (m_selected_shape == nullptr) {
        m_mode = mode;
        m_isDrawing = false;
    }
}

void DrawingCanvas::setDrawingProperties(const drawing_properties_t& data) {
    m_brush_color   = data.brush_color.value_or(m_brush_color);
    m_drawing_color = data.line_color.value_or(m_drawing_color);
    m_pen_style     = data.pen_style.value_or(m_pen_style);
    m_pen_width     = data.pen_width.value_or(m_pen_width);
    if(m_selected_shape) {
        // called when a shape is selected
        m_selected_shape->setDrawingProperties(data);
        restoreShape();
    }
}

void DrawingCanvas::undoLast() {
    if (!m_shapes.empty()) {
        m_shapes.pop_back(); // Remove last drawn shape
        update();
    }
}

void DrawingCanvas::clearAll() {
    m_shapes.clear();
    m_isDrawing = false;
    update();
}

void DrawingCanvas::setShapeZoomFactor(const qreal& zoom_factor) {
    assert(m_selected_shape);
    m_selected_shape->zoomInOut(zoom_factor);
    update();
}

void DrawingCanvas::setGroupZoomFactor(const qreal& zoom_factor) {
    for(const auto& shape : m_group_shapes) {
        shape->zoomInOut(zoom_factor);
    }
    update();
}

void DrawingCanvas::zommInOut(const qreal& zoom_factor) {
    for(const auto& shape : m_shapes) {
        shape->zoomInOut(zoom_factor);
    }
    update();
}
// ---- end of user requested actions -----

void DrawingCanvas::paintGrid(QPainter& painter, unsigned grid_width)
{
    painter.fillRect(rect(), Qt::white);
    int w = width();
    int h = height();
    QPen minorPen(Qt::lightGray, 1);
    QPen majorPen(Qt::gray, 2);
    // Draw Vertical Lines
    for (int x = 0, lineCount = 0; x < w; x += grid_width, ++lineCount) {
        painter.setPen((lineCount % 5 == 0) ? majorPen : minorPen);
        painter.drawLine(x, 0, x, h);
    }
    // Draw Horizontal Lines
    for (int y = 0, lineCount = 0; y < h; y += grid_width, ++lineCount) {
        painter.setPen((lineCount % 5 == 0) ? majorPen : minorPen);
        painter.drawLine(0, y, w, y);
    }
}

void DrawingCanvas::finalizeShape(std::optional<QPointF> const&  point) {
    /*
       - reset the drawing flag, set the pencil and brush for the new shape;
       - add the shape to existing shapes
       - handle the grouping case
    */
    logging(std::move(std::string{"finalize shape"}));
    assert(m_shape);
    // reset drawing flag
    m_isDrawing = false;

    if (m_grouping) {
        // build the rectangle to group several shapes
        QRectF rectangle(m_start_pos, m_current_pos);

        // compute the shapes beeing grouped
        for(const auto& shape : m_shapes) {
            QPolygonF polygon = shape->getPoints();
            if (polygon.intersects(rectangle)) {
                // add a shape to the group
                m_group_shapes.push_back(shape.get());
            }
        }
        if (point.has_value())
            m_shape->toolHint(mapToGlobal(point.value().toPoint()),
                std::move(QString("Use the mouse wheel for zoom in zoom out")));
    }
    else {
        // store the new shape
        m_shapes.push_back(std::move(m_shape));
    }
}

void DrawingCanvas::restoreShape() {
    if (m_selected_shape) {
        /* 
          The shape was selected for zooming or moving
          restore brushes and pencil, restore mouse cursor
        */
        QGuiApplication::restoreOverrideCursor();
        // move back the selected shape at his previous position in the list
        if ( m_shapes.size() > 1 && m_select_shape_it != m_shapes.end()) {
            auto last_it = std::prev(m_shapes.end());
            std::swap(*m_select_shape_it, *last_it);
            qDebug() << "swapping shapes..";
        }
        // Restore the brush and color of the selected shape
        m_selected_shape = nullptr;
        m_isDrawing = false;
        update();
    }
    else if (m_shape && m_mode == ToolMode::Polygon) {
        /*
          This section is available as a callback when
          drawing a Polygon, Right click, than click on "Done";
          The Polygon shape is new, never selected so far
        */
        finalizeShape(std::nullopt);
        update();
    }
}

void DrawingCanvas::removeShape() {
    assert(m_selected_shape);
    m_shapes.erase(
        std::remove_if(m_shapes.begin(), m_shapes.end(),
            [this](const std::unique_ptr<Shape>& shape) {
                return shape.get() == m_selected_shape;
            }),
        m_shapes.end()
    );
    QGuiApplication::restoreOverrideCursor();
    m_selected_shape = nullptr;
    update();
}

 void DrawingCanvas::removeGroup() {
    m_shapes.erase(
        std::remove_if(m_shapes.begin(), m_shapes.end(),
            [this](const std::unique_ptr<Shape>& shape) {
                auto it = std::find(m_group_shapes.begin(), m_group_shapes.end(), shape.get());
                return it != m_group_shapes.end();
            }),
        m_shapes.end()
    );
    QGuiApplication::restoreOverrideCursor();
    m_group_shapes.clear();
    m_grouping = false;
    update();
 }

void DrawingCanvas::cloneShape() {
    assert(m_shape == nullptr);
    assert(m_selected_shape != nullptr);
    m_shape = m_selected_shape->clone();
    QPointF delta = QPointF(clone_x_offset, clone_y_offset);
    m_shape->moveRelative(delta);
    m_shapes.push_front(std::move(m_shape));
    update();
}

Shape* DrawingCanvas::isShapeSelected() const {
    return m_selected_shape;
}

bool DrawingCanvas::isGrouping() const {
    return m_group_shapes.size() > 0;
}

ToolMode DrawingCanvas::getToolMode() const {
    return m_mode;
}

const QColor& DrawingCanvas::getDrawingColor() const {
    return m_drawing_color;
}

void DrawingCanvas::paintEvent(QPaintEvent *) {
    // allways create the QPainter object inside the paintEvent() method
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    paintGrid(painter, 16); // Draw grid with 16px spacing

    // draw what we have so far
    for (const auto &shape : m_shapes) {
        if (shape.get() == m_selected_shape)
            // draw the selected Shape with hooks on the center
            shape->drawSelect(painter, w_hooks);
        else {
            auto it = std::find(m_group_shapes.begin(), m_group_shapes.end(), shape.get()); 
            // the shape is grouped, draw it specially but wo hooks
            if (it != m_group_shapes.end())
                shape->drawSelect(painter, wo_hooks);
            else
                // normal draw
                shape->draw(painter);
        }
    }
    // in case of grouping draw the rectangle group
    if(m_shape && m_grouping)
        m_shape->draw(painter);

    // Drawing mode
    if (m_isDrawing && m_shape) {
        // draw a preview, prepare the shape coordinates and colors for drawing     
        ShapeData_t previewData;
        previewData.type = getShapeType(m_mode);
        previewData.start = m_start_pos;
        previewData.end = m_current_pos;
        previewData.radius = std::hypot(m_current_pos.x() - m_start_pos.x(), m_current_pos.y() - m_start_pos.y());
        previewData.points = m_shape->getPoints();
        QPen pencil(QPen(m_selected_color, 2, Qt::DashLine));
        previewData.pencil = pencil;
        previewData.brush = m_select_brush;
        m_shape->draw(painter, previewData);
    }
}

void DrawingCanvas::mousePressEvent(QMouseEvent *event) {

    if (!event || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event); // Call base class if right-click or middle-click
        return;
    }
    logging("Mouse press event");
    const QPointF mousePos = event->position();

    // Group Dragging
    if (!m_group_shapes.empty()) {
        if (!QGuiApplication::overrideCursor()) {
            QGuiApplication::setOverrideCursor(Qt::ClosedHandCursor);
        }
        m_current_pos = mousePos;
        update();
        event->accept();
        return;
    }

    // Shape Manipulation (Resize or Move)
    if (m_selected_shape) {
        m_handle = m_selected_shape->hookTest(mousePos);
        qDebug() << "Hook test:" << m_handle;

        if (m_handle != HandlePosition::None) {
            // Clicked a resize handle
            m_start_zoom_pos = mousePos;
        } 
        else if (m_selected_shape->contains(mousePos)) {
            // Clicked inside the active shape (prepare to drag)
            m_start_pos = m_current_pos = mousePos;
        } 
        else {
            // Clicked outside the active shape -> Deselect
            restoreShape();
        }
        update();
        event->accept();
        return;
    }
    // Shape Selection / Picking
    m_selected_shape = getSelectedShape(mousePos);
    if (m_selected_shape) {
        QGuiApplication::setOverrideCursor(Qt::ClosedHandCursor);
        m_current_pos = mousePos;

        const QPoint globalPos = mapToGlobal(mousePos.toPoint());
        m_selected_shape->toolHint(
            globalPos,
            QStringLiteral("Use the wheel to zoom-in zoom-out, click right for more actions")
        );
    } 
    // 4. Drawing Mode (No shape selected)
    else {
        drawingMode(event);
    }
    update();
    event->accept();
}

void DrawingCanvas::mouseMoveEvent(QMouseEvent *event) {

    logging(std::string{"Mouse move event"});
    if (m_isDrawing) {
        // Drawing mode
        if (!m_shape)
            return;

        if (m_mode == ToolMode::Polygon)
            // tool hint text in case of Polygon
            m_shape->toolHint(mapToGlobal(event->position().toPoint()),
                std::move(QString("Mouse right click to finish the shape")));

        m_current_pos = event->position();
        QPen pencil(QPen(m_selected_color, m_pen_width, Qt::DashLine));

        ShapeData_t shape_data = {
            m_shape->type(),
            m_start_pos, m_current_pos,
            std::nullopt,
            m_shape->getPoints(),
            m_select_brush, pencil };
        m_shape->setShapeData(shape_data);
        update(); // Re-trigger paintEvent for preview
        event->accept();

     } else if (m_selected_shape != nullptr) {
        // Check for shape hooks click
        if (hookSelected(m_handle)) {
            // one of the four hooks clicked
            QPointF delta = (event->position() - m_start_zoom_pos);
            // ask for Shape coordinates re-computation
            m_selected_shape->resizeShape(delta, m_handle);
            m_start_zoom_pos = event->position();
        }
        else {
            // Drag the selected shape
            // Show tool hint for Zoom-in Zoom-out
            m_selected_shape->toolHint(mapToGlobal(event->position().toPoint()),
                std::move(QString("Use the wheel to zoom-in zoom-out, click outside when done")));
            QPointF delta = event->position() - m_current_pos;
            m_selected_shape->moveRelative(delta);
            m_current_pos = event->position();
        }
        update();
        event->accept();
    }
    else if (m_grouping) {        
        // grouping case
        QPointF delta = event->position() - m_current_pos;
        for(const auto& shape : m_group_shapes) {
            shape->moveRelative(delta);
        }
        assert(m_shape != nullptr);
        // move the rectangle (m_shape) grouping the shapes
        m_shape->moveRelative(delta);
        m_current_pos = event->position();
        update();
        event->accept();
    }
}

void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event)  {

    if (!event || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event); // Call base class if right-click or middle-click
        return;
    }
    logging(std::string{"Mouse release event"});
    // one shape is selected
    if (m_selected_shape != nullptr) {
        m_selected_shape->toolHint(mapToGlobal(event->position().toPoint()),
            std::move(QString("Use the wheel to zoom-in zoom-out, click outside when done")));
    }
    else if (m_isDrawing && m_shape) {
        // Drawing case
        if (   m_mode == ToolMode::Line 
            || m_mode == ToolMode::Circle
            || m_mode == ToolMode::Rectangle
            || m_mode == ToolMode::Arc
            || m_mode == ToolMode::Group) {
            // Line, Rectangle, Circle, Arc and the grouping rectangle
            m_current_pos = event->position();
            // check if the Shape is big enough
            QPointF delta = m_current_pos - m_start_pos;
            qreal distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();
            if (distanceSquared >= (QApplication::startDragDistance() * QApplication::startDragDistance())) {
                // Store the new shape, including the grouping one
                finalizeShape(event->position());
            }
            else
                // ! the shape is too small
                m_shape.reset();
        }
    else if (m_mode == ToolMode::Polygon)
        // Polygon or Arc
        m_shape->toolHint(mapToGlobal(event->position().toPoint()),
            std::move(QString("Right click to finish the shape")));
    }
    else if (m_grouping) {
        assert(m_shape);
        if (!m_shape->contains(event->position())) {
            /*
                click outside the grouping rectangle
                clear the list containing the grouped shapes and flags
            */
            m_grouping = false;
            m_shape.reset();
            m_group_shapes.clear();
            QGuiApplication::restoreOverrideCursor();
        } else
            m_shape->toolHint(mapToGlobal(event->position().toPoint()),
                std::move(QString("Use the wheel to zoom in zoom out")));
    }
    update();
    event->accept();
}

void DrawingCanvas::wheelEvent(QWheelEvent *event) {
    // Determine scroll direction (positive = scroll up/zoom in, negative = scroll down/zoom out)
    int delta = event->angleDelta().y();
    if (delta == 0) return; // Ignore horizontal scrolling
    // Define the step ratio (e.g., 10% per wheel notch)
    qreal stepFactor = (delta > 0) ? 1.10 : (1.0 / 1.10);
    // 3. Update and clamp the accumulated zoom factor
    qreal minZoom = 0.1;  // 10% minimum
    qreal maxZoom = 10.0; // 2000% maximum
    qreal zoom_factor = 1.0;
    zoom_factor = qBound(minZoom, zoom_factor * stepFactor, maxZoom);
    if(m_selected_shape) {
        m_selected_shape->zoomInOut(zoom_factor);
        // qDebug() << "zoom factor " << zoom_factor;
        // Display the tooltip near the cursor
        m_selected_shape->toolHint(event->globalPosition().toPoint(),
            std::move(QString("Use the mouse wheel to zoom, click outside when done")));
        // trigger a redraw
        update();
        event->accept();
     }
     // zoom the group of shapes
     else if (m_group_shapes.size() > 0) {
        for(const auto& shape : m_group_shapes)
            shape->zoomInOut(zoom_factor);
        update();
        event->accept();
     }
}

void DrawingCanvas::drawingMode(const QMouseEvent *event) {
    if (!event) return;

    if (m_mode == ToolMode::Group && m_shapes.empty()) {
        return;
    }

    m_start_pos = m_current_pos = event->position();
    m_isDrawing = true;
    m_grouping = false;

    const QPen pencil(m_drawing_color, m_pen_width, m_pen_style);
    const QBrush brush(m_brush_color, Qt::SolidPattern);

    switch (m_mode) {
    case ToolMode::Line: {
        const QLineF line(m_start_pos, m_current_pos);
        m_shape = std::make_unique<LineShape>(line, pencil, brush);
        break;
    }
    case ToolMode::Circle: {
        // At press time, radius is 0 because start_pos == current_pos
        m_shape = std::make_unique<CircleShape>(m_start_pos, 0.0, pencil, brush);
        break;
    }
    case ToolMode::Rectangle: {
        const QRectF rect(m_start_pos, m_current_pos);
        m_shape = std::make_unique<RectangleShape>(rect, pencil, brush);
        break;
    }
    case ToolMode::Polygon: {
        if (!m_shape) {
            m_shape = std::make_unique<PolygonShape>(pencil, brush);
        }
        m_shape->addPoint(m_start_pos);
        break;
    }
    case ToolMode::Arc: {
        m_shape = std::make_unique<ArcShape>(pencil, brush);
        break;
    }
    case ToolMode::Group: {
        const QRectF rect(m_start_pos, m_current_pos);
        const QPen groupPen(Qt::darkGray, 2, Qt::DashLine);
        
        m_shape = std::make_unique<RectangleShape>(rect, groupPen, m_grouping_brush);
        m_grouping = true;
        qDebug() << "Grouping selection started.";
        break;
    }
    default:
        m_isDrawing = false;
        qWarning() << "Mouse press event: unknown drawing mode" << static_cast<int>(m_mode);
        break;
    }
}

Shape* DrawingCanvas::getSelectedShape(const QPointF& point) {

    if (m_shapes.empty())
        return nullptr;

    if (m_shapes.size() == 1 && m_shapes.front()->contains(point)) {
        m_select_shape_it = m_shapes.begin();
        return m_shapes.front().get();
    }

    auto lastIt = std::prev(m_shapes.end());
    m_select_shape_it = m_shapes.end();
    for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it) {
        Shape* shape = it->get();
        if (shape->contains(point)) {
            /* 
            - Identify a shape under the cursor;
            - bring in front the selected shape;
            */
            if (it != lastIt) {
                m_select_shape_it = it;
                std::swap(*it, *lastIt);
            }
            return shape;
        }
    }
    return nullptr;
}

bool DrawingCanvas::saveToFile(const QString &filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);
    // Write total count of shapes
    out << static_cast<quint32>(m_shapes.size());
    // Write each shape's Type ID followed by its serialized data
    for (const auto &shape : m_shapes) {
        out << static_cast<quint32>(shape->type());
        shape->serialize(out);
    }
    return true;
}

bool DrawingCanvas::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);
    quint32 count = 0;
    in >> count;
    // Check if reading the header count failed
    if (in.status() != QDataStream::Ok) {
        return false;
    }
    // clear shape list
    m_shapes.clear(); 

    for (quint32 i = 0; i < count; ++i) {
        quint32 rawType = 0;
        in >> rawType;
        if (in.status() != QDataStream::Ok) {
            break; // Stop if stream is truncated or corrupted
        }
        std::unique_ptr<Shape> shape;
        switch (static_cast<ShapeType>(rawType)) {
            case ShapeType::Line:
                shape = std::make_unique<LineShape>();
                break;
            case ShapeType::Circle:
                shape = std::make_unique<CircleShape>();
                break;
            case ShapeType::Rectangle:
                shape = std::make_unique<RectangleShape>();
                break;
            case ShapeType::Polygon:
                shape = std::make_unique<PolygonShape>();
                break;
            case ShapeType::Arc:
                shape = std::make_unique<ArcShape>();
                break;
            default:
                // Unknown shape type encountered in file
                return false;
        }
        if (shape) {
            shape->deserialize(in);
            // Verify serialization of individual shape succeeded
            if (in.status() != QDataStream::Ok) {
                m_shapes.clear();
                return false;
            }
            m_shapes.push_back(std::move(shape));
        }
    }
    update(); // Refresh widget canvas
    return true;
}

bool DrawingCanvas::hookSelected(const HandlePosition handle) const {
    return (handle == HandlePosition::TopCenter ||
            handle == HandlePosition::RightCenter ||
            handle == HandlePosition::BottomCenter ||
            handle == HandlePosition::LeftCenter ||
            handle == HandlePosition::StartPoint ||
            handle == HandlePosition::EndPoint);
}

void DrawingCanvas::logging(const std::string& message) {
    qDebug() << message << "- drawing" << m_isDrawing << "- grouping" << m_grouping <<
        (m_shape == nullptr ? "shape not defined" : "new shape defined") << "-" <<
        (m_selected_shape != nullptr ? "shape selected" : "shape not selected ");
}
