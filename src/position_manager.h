#ifndef POSITION_MANAGER_H
#define POSITION_MANAGER_H

#include <QPointF>

class PositionManager {

    QPointF m_current_pos;
    QPointF m_start_pos;
    QPointF m_resize_start_pos;

    public:
    PositionManager() : m_current_pos(0.0, 0.0), m_start_pos(0.0, 0.0), m_resize_start_pos(0.0, 0.0) {
    }
    //Setters
    void setStartPos(const QPointF& position) {
        m_start_pos = position;
    }
    void setCurrentPos(const QPointF& position) {
        m_current_pos = position;
    }
    void setResizeStartPos(const QPointF& position) {
        m_resize_start_pos = position;
    }
    //Getters
    const QPointF& getStartPos() const {
        return m_start_pos;
    }
    const QPointF getCurrentPos() const {
        return m_current_pos;
    }
    const QPointF getResizeStartPos() const {
        return m_resize_start_pos;
    }
};

#endif
