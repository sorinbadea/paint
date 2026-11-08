#include "drawing_canvas.h"

void DrawingCanvas::handleMousePressEvent(const QPointF& mousePos) {
    assert(m_selected_shape);
    m_handle = m_selected_shape->hookTest(mousePos);
    // qDebug() << "Hook test:" << m_handle;
    if (m_handle != HandlePosition::None) {
        // Clicked a resize handle
        m_position_mgr.setResizeStartPos(mousePos);
    } 
    else if (m_selected_shape->contains(mousePos)) {
        // Clicked inside a shape (prepare to drag)
        m_position_mgr.setStartPos(mousePos);
        m_position_mgr.setCurrentPos(mousePos);
    } 
    else {
        // Click outside the active shape -> Unselect
        restoreShape();
    }
}

void DrawingCanvas::handleMouseMoveEvent(const QPointF& position) {
    assert(m_selected_shape);
    // Check for shape hooks click
    if (hookSelected(m_handle)) {
        // one of the hooks clicked
        QPointF delta = position -  m_position_mgr.getResizeStartPos();
        // ask for Shape coordinates re-computation
        // TODO integrate with resizeShape(), avoid consider TextShape!
        TextShape* text_shape = dynamic_cast<TextShape*>(m_selected_shape);
        if (text_shape) {
            QPointF center = text_shape->m_rect.center();
            // Define vectors relative to the shape's center pivot
            QPointF startPoint =   m_position_mgr.getResizeStartPos() - center;
            QPointF currentPoint = position - center;
            double deltaDegrees = computeRotationAngle(startPoint, currentPoint);
            text_shape->setRotationAngle(deltaDegrees);
        }
        else {
            // other shape, Line, Circle, Rectangle
            m_selected_shape->resizeShape(delta, m_handle);
        }
        m_position_mgr.setResizeStartPos(position);
    }
    else {
        // Drag the selected shape
        // Show tool hint for Zoom-in Zoom-out
        m_selected_shape->toolHint(mapToGlobal(position.toPoint()),
            std::move(QString("Use the wheel to zoom-in zoom-out, click outside when done")));
        QPointF delta = position - m_position_mgr.getCurrentPos();
        m_selected_shape->moveRelative(delta);
        m_position_mgr.setCurrentPos(position);
    }
}
