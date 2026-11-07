
#ifndef PAINT_ICONS_H
#define PAINT_ICONS_H

/*
helper file for icons drawing
*/
#include "shapes.h"
#include <QPainterPath>
#include <QFileDialog>
#include <QPainter>
#include <QColorDialog>
#include <QActionGroup>
#include <QToolBar>
#include <QLabel>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QMessageBox>
#include <QToolButton>
#include <QInputDialog>

constexpr unsigned ICON_SIZE = 28;
constexpr unsigned ICON_SELECT_SIZE = 20;
constexpr qreal tool_bar_pen_width = 2.5;

QIcon   createLineIcon(unsigned size = ICON_SIZE);
QIcon   createCircleIcon(unsigned size = ICON_SIZE);
QIcon   createRectangleIcon(unsigned size = ICON_SIZE);
QIcon   createSelectIcon(unsigned size = ICON_SELECT_SIZE, const QColor &arrow_color = QColor(10, 10, 10));
QIcon   createPencilIcon(const QColor &color);
QIcon   createBrushIcon(const QColor &color);
QIcon   createPolygonIcon();
QIcon   createGroupIcon(unsigned size = ICON_SIZE, const QColor &dotColor = QColor(10, 10, 10));
QIcon   createArcToolIcon(const QColor& iconColor = Qt::black, const QSize& iconSize = QSize(32, 32)); 
QIcon   createLinePreviewIcon(Qt::PenStyle style, int penWidth, const QSize& iconSize = QSize(48, 32), const QColor& color = Qt::black);
QIcon   createArrowPreviewIcon(const QSize& iconSize, const QColor& color, const ArrowEndPoint& arrow_style);
QIcon   createRectanglePreviewIcon(const QSize& iconSize, const QColor& color);
QIcon   createAbIcon(const unsigned size = ICON_SIZE);
QString penStyleToText(Qt::PenStyle style);

#endif
