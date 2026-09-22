
#ifndef PAINT_ICONS_H
#define PAINT_ICONS_H

/*
helper file for icons drawing
*/

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

constexpr unsigned ICON_SIZE = 30;

QIcon   createLineIcon(unsigned size = ICON_SIZE);
QIcon   createCircleIcon(unsigned size = ICON_SIZE);
QIcon   createRectangleIcon(unsigned size = ICON_SIZE);
QIcon   createSelectIcon(unsigned size = ICON_SIZE, const QColor &dotColor = QColor(10, 10, 10));
QIcon   createPencilIcon(const QColor &color);
QIcon   createBrushIcon(const QColor &color);
QIcon   createPolygonIcon();
QIcon   createGroupIcon(unsigned size = ICON_SIZE, const QColor &dotColor = QColor(10, 10, 10));
QIcon   createWidthIcon(int width, const QColor& pen_color);
QIcon   createArcToolIcon(const QColor& iconColor = Qt::red, const QSize& iconSize = QSize(32, 32)); 

#endif
