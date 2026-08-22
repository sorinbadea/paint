
#include "shapes.h"
#include "drawing_canvas.h"

DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent),
    m_isDrawing(false),
    m_selectedShape(nullptr),
    m_pen_width(2),
    m_drawing_color(QColor("#0284c7")),
    m_preview_color(QColor("#dc2626")),
    m_selected_color(QColor("#b9b90b")),
    m_drawing_qbrush(QBrush(QColor("#1e4db428"))),
    m_select_brush(QBrush(QColor("#facc1566"))),
    m_preview_qbrush(QBrush(QColor("#dc26261e"))) {
    setBackgroundRole(QPalette::Base); 
    setAutoFillBackground(true);
}

void DrawingCanvas::setMode(ToolMode mode) {
    m_mode = mode;
    m_isDrawing = false;
}

void DrawingCanvas::setPenWidth(int width) {
    m_pen_width = width;
}

void DrawingCanvas::setColor(const QColor& color) {
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

void DrawingCanvas::paintGrid(QPainter& painter, unsigned grid_width)
{
    painter.fillRect(rect(), Qt::white);
    int w = width();
    int h = height();
    QPen minorPen(QColor(230, 230, 230), 1);
    QPen majorPen(QColor(180, 200, 230), 2);
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

void DrawingCanvas::paintShape(const ShapeData_t &shapeData, const QPen& pen, const QBrush& brush, QPainter& painter) {
    // Line
    if (shapeData.type == ShapeType::Line && shapeData.start && shapeData.end) {
        QLineF line(*shapeData.start, *shapeData.end);
        painter.setPen(pen);
        painter.drawLine(line);
    //Circle
    } else if (shapeData.type == ShapeType::Circle && shapeData.start && shapeData.radius) {
        painter.setPen(pen);
        if (shapeData.radius > 0) {
            painter.setBrush(brush);
            painter.drawEllipse(*shapeData.start, *shapeData.radius, *shapeData.radius);
        }
    } else {
        qDebug() << "Invalid shape data for painting.";
    }
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
    // B. Draw temporary preview while dragging the mouse
    if (m_isDrawing) {
        QPen pencil(QPen(m_preview_color, m_pen_width, Qt::DashLine));
        ShapeData_t previewData;
        previewData.start = m_startPos;
        previewData.end = m_currentPos;
        previewData.radius = std::hypot(m_currentPos.x() - m_startPos.x(), m_currentPos.y() - m_startPos.y());
        previewData.type = (m_mode == ToolMode::Line) ? ShapeType::Line : ShapeType::Circle;
        paintShape(previewData, pencil, m_preview_qbrush, painter);
    }
}

void DrawingCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_mode == ToolMode::Select) {
            // Mode select and drag  
            for(const auto& shape : m_shapes) {   
                if (shape->contains(event->position())) {
                    QGuiApplication::setOverrideCursor(Qt::ClosedHandCursor);
                    qDebug() << "Selected shape of type: " << static_cast<int>(shape->type());
                    m_selectedShape = shape.get();
                    m_currentPos = event->position();
                    m_shape_pen = shape->getPen();
                    //set the pen for the highlighted objects
                    QPen pencil(QPen(m_selected_color, m_pen_width, Qt::DashLine));
                    shape->setPen(pencil);
                    break;
                }
            }
        }
        else {
            // drawing..
            m_last_mode = m_mode;
            m_startPos = event->position();
            m_currentPos = m_startPos;
            m_isDrawing = true;
        }
        update();
    }
}

void DrawingCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDrawing) {
        m_currentPos = event->position();
        update(); // Re-trigger paintEvent for preview
     } else if(m_selectedShape) {
        // Dragging selected shape
        QPointF delta = event->position() - m_currentPos;
        m_selectedShape->moveRelative(delta);
        m_currentPos = event->position();
        update();
    }
}

void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event)  {

    if(m_selectedShape) {
        QGuiApplication::restoreOverrideCursor();
        // Highlight selected shape
        m_selectedShape->setPen(m_shape_pen);
        m_selectedShape = nullptr;
        m_mode = m_last_mode;
        update();
    }
    else if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;
        m_currentPos = event->position();
        QPen pencil(QPen(m_drawing_color, m_pen_width, Qt::SolidLine));
        // Line drawing
        if (m_mode == ToolMode::Line) {
            QLineF line(m_startPos, m_currentPos);
            if (line.length() > 2.0)
                m_shapes.push_back(std::make_unique<LineShape>(line, pencil));
        // Circle drawing
        } else if (m_mode == ToolMode::Circle) {
            qreal radius = std::hypot(m_currentPos.x() - m_startPos.x(),
                                        m_currentPos.y() - m_startPos.y());
            if (radius > 2.0)
                m_shapes.push_back(std::make_unique<CircleShape>(m_startPos, radius, pencil, m_drawing_qbrush));
        }
        update();
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
    m_shapes.clear(); // Clear current canvas before loading

    quint32 count;
    in >> count;

    for (quint32 i = 0; i < count; ++i) {
        quint32 rawType;
        in >> rawType;
        std::unique_ptr<Shape> shape;
        // Factory instantiation based on Type ID
        if (static_cast<ShapeType>(rawType) == ShapeType::Line) {
            shape = std::make_unique<LineShape>();
        } else if (static_cast<ShapeType>(rawType) == ShapeType::Circle) {
            shape = std::make_unique<CircleShape>();
        }
        if (shape) {
            shape->deserialize(in);
            m_shapes.push_back(std::move(shape));
        }
    }
    update(); // Refresh screen with loaded shapes
    return true;
}

