#include <QToolTip>
#include "shapes.h"
#include "drawing_canvas.h"

DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent),
    m_isDrawing(false),
    m_selected_shape(nullptr),
    m_pen_width(2),
    m_drawing_color(QColor("#0284c7")),
    m_selected_color(QColor("#e3d01f")),
    m_brush_color(QColor("#1e4db428")),
    m_select_brush(QBrush(QColor("rgba(152, 102, 144, 0.14)"), Qt::DiagCrossPattern)) {
    setBackgroundRole(QPalette::Base); 
    setAutoFillBackground(true);
}

void DrawingCanvas::setMode(const ToolMode& mode) {
    if (m_selected_shape == nullptr) {
        m_mode = mode;
        m_isDrawing = false;
    }
}

void DrawingCanvas::setPenWidth(int width) {
    m_pen_width = width;
}

void DrawingCanvas::setBrushColor(const QColor& brush) {
    m_brush_color = brush;
}

void DrawingCanvas::setPaintColor(const QColor& color) {
   m_drawing_color = color;
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

ShapeType DrawingCanvas::getShapeType(const ToolMode& tm) const {
    if (tm == ToolMode::Line)
        return ShapeType::Line;
    else if (tm == ToolMode::Circle)
        return ShapeType::Circle;
    else if (tm == ToolMode::Rectangle)
        return ShapeType::Rectangle;
    else if (tm == ToolMode::Polygon)
        return ShapeType::Polygon;  
    return ShapeType::None;
}

void DrawingCanvas::paintGrid(QPainter& painter, unsigned grid_width)
{
    painter.fillRect(rect(), Qt::white);
    int w = width();
    int h = height();
    QPen minorPen(QColor("#e6e6e6"), 1);
    QPen majorPen(QColor("#7f9bc6"), 1);
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

void DrawingCanvas::finalizeShape() {
    m_isDrawing = false;
    QPen pencil(QPen(m_drawing_color, m_pen_width, Qt::SolidLine));
    QBrush brush(m_brush_color, Qt::SolidPattern);
    m_shape->setPen(pencil);
    m_shape->setBrush(brush);
    m_shapes.push_back(std::move(m_shape));
}

void DrawingCanvas::paintEvent(QPaintEvent *) {
    // allways create the QPainter object inside the paintEvent() method
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    paintGrid(painter, 16); // Draw grid with 16px spacing

    // A. Draw all saved shapes in the order they were created
    for (const auto &shape : m_shapes)
        shape->draw(painter);

    // Drawing mode
    if (m_isDrawing && m_shape) {
        // Draw temporary preview
        // prepare the shape coordinates and colors for drawing
        ShapeData_t previewData;
        previewData.start = m_start_pos;
        previewData.end = m_current_pos;
        previewData.radius = std::hypot(m_current_pos.x() - m_start_pos.x(), m_current_pos.y() - m_start_pos.y());
        previewData.type = getShapeType(m_mode);
        previewData.points = m_points;
        m_shape->draw(painter, previewData);
    }
}

void DrawingCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_mode == ToolMode::Select) {
            // Select and drag mode
            //---------------------
            if (m_selected_shape)
                // if already selected do nothing
                return;
            for(const auto& shape : m_shapes)
                if (shape->contains(event->position())) {
                    QGuiApplication::setOverrideCursor(Qt::ClosedHandCursor);
                    // retrieve the pen and the brush of 
                    // selected shape, will be used when the
                    // shape will find his new position or zoom factor
                    m_selected_shape = shape.get();
                    m_current_pos = event->position();
                    m_shape_pen = shape->getPen();
                    m_shape_brush = shape->getBrush();
                    // set the pen for the highlighted object
                    QPen pencil(QPen(m_selected_color, m_pen_width, Qt::DashLine));
                    shape->setPen(pencil);
                    shape->setBrush(m_select_brush);
                    m_selected_shape->toolHint(mapToGlobal(event->position().toPoint()),
                        std::move(QString("Use the wheel to zoom-in zoom-out, right click to finalize")));
                    break;
                }
        }
        else {
            // drawing mode
            //-------------
            m_start_pos =  m_current_pos = event->position();
            m_isDrawing = true;
            QPen pencil(QPen(m_selected_color, m_pen_width, Qt::DashLine));
            if (m_mode == ToolMode::Line) {
                QLineF line(m_start_pos, m_current_pos);
                m_shape = std::make_unique<LineShape>(line, pencil);
            }
            else if (m_mode == ToolMode::Circle) {
                qreal radius = std::hypot(m_current_pos.x() - m_start_pos.x(), m_current_pos.y() - m_start_pos.y());
                m_shape = std::make_unique<CircleShape>(m_start_pos, radius, pencil, m_select_brush);
            }
            else if (m_mode == ToolMode::Rectangle) {
                QRectF rect(m_start_pos, m_current_pos);
                m_shape = std::make_unique<RectangleShape>(rect, pencil);
            }
            else if (m_mode == ToolMode::Polygon) {
                if (m_shape == nullptr) {
                    QPen pencil(QPen(m_selected_color, m_pen_width, Qt::DashLine));
                    QBrush brush(m_brush_color, Qt::SolidPattern);
                    m_shape = std::make_unique<PolygonShape>(pencil, brush);
                }
                m_shape->addPoint(event->position());
                m_points.append(event->position());
            }
            else
                qDebug() << "Unknown shape..";
        }
        update();
    }
    else if (event->button() == Qt::RightButton) {
        if (m_shape) {
            // the new shape is ready, set back
            // the chosen pen and clear points in case of Polygon
            finalizeShape();
            if (m_mode == ToolMode::Polygon) {
                // finalize the new added polygon shape
                m_points.clear();
            }
            update();
        }
        // The shape was selected for moving or Zooming
        else if (m_selected_shape) {
            QGuiApplication::restoreOverrideCursor();
            // Restore the brush and color
            m_selected_shape->setPen(m_shape_pen);
            m_selected_shape->setBrush(m_shape_brush);
            m_selected_shape = nullptr;
            m_isDrawing = false;
            update();
        }
    }
}

void DrawingCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDrawing) {
        // tool hint text
        if (m_shape && m_mode == ToolMode::Polygon)
            m_shape->toolHint(mapToGlobal(event->position().toPoint()),
                std::move(QString("Mouse right click to finish the polygon")));
        m_current_pos = event->position();
        ShapeData_t shape_data = {m_shape->type(), m_start_pos, m_current_pos, std::nullopt, m_points};
        m_shape->setShapeData(shape_data);
        update(); // Re-trigger paintEvent for preview
     } else if (m_selected_shape) {
        // Dragging the selected shape
        // Show tool hint text
        m_selected_shape->toolHint(mapToGlobal(event->position().toPoint()),
            std::move(QString("Use the wheel to zoom-in zoom-out, right click when done")));
        QPointF delta = event->position() - m_current_pos;
        m_selected_shape->moveRelative(delta);
        m_current_pos = event->position();
        update();
    }
}

void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event)  {
    if (event->button() == Qt::LeftButton) {
        if (m_selected_shape) {
            m_selected_shape->toolHint(mapToGlobal(event->position().toPoint()),
                std::move(QString("Use the wheel to zoom-in zoom-out, right click when done")));
        }
        else if (m_isDrawing) {
            if (m_mode != ToolMode::Polygon) {
                // Line, Rectangle and Circle
                m_current_pos = event->position();
                finalizeShape();
            }
            else
                // Polygon
                m_shape->toolHint(mapToGlobal(event->position().toPoint()),
                    std::move(QString("Right click to finish the polygon")));
        }
        update();
    }
}

void DrawingCanvas::wheelEvent(QWheelEvent *event) {
     if(m_selected_shape) {
        // 1. Determine scroll direction (positive = scroll up/zoom in, negative = scroll down/zoom out)
        int delta = event->angleDelta().y();
        if (delta == 0) return; // Ignore horizontal scrolling
        // 2. Define the step ratio (e.g., 15% per wheel notch)
        qreal stepFactor = (delta > 0) ? 1.10 : (1.0 / 1.10);
        // 3. Update and clamp the accumulated zoom factor
        qreal minZoom = 0.1;  // 10% minimum
        qreal maxZoom = 20.0; // 2000% maximum
        qreal zoom_factor = 1.0;
        zoom_factor = qBound(minZoom, zoom_factor * stepFactor, maxZoom);
        m_selected_shape->zoomInOut(zoom_factor);
        // Display the tooltip near the cursor
        m_selected_shape->toolHint(event->globalPosition().toPoint(),
            std::move(QString("Use the mouse wheel to zoom, right click when done")));
        // trigger a redraw
        update();
        event->accept();
     }
}

bool DrawingCanvas::saveToFile(const QString &filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);
    // 1. Write total count of shapes
    out << static_cast<quint32>(m_shapes.size());
    // 2. Write each shape's Type ID followed by its serialized data
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

    m_shapes.clear(); // Clear canvas only after confirming file header is valid
    m_shapes.reserve(count); // Optimize vector allocations

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

