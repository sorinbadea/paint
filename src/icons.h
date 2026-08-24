
#ifndef PAINT_ICONS_H
#define PAINT_ICONS_H

/*
helper file for icons drawing
*/

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

QIcon createLineIcon();
QIcon createCircleIcon();
QIcon createDragIcon(int size = 32, const QColor &dotColor = QColor(10, 10, 10));
QIcon createPencilIcon(const QColor &color);

#endif
