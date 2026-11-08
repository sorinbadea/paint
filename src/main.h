#ifndef PAINT_MAIN_H
#define PAINT_MAIN_H

#include "drawing_canvas.h"
#include "icons.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // attributes
    QString m_file_path;
    Qt::PenStyle m_pen_style;
    int m_pen_width;
    QMenu* m_file_menu;
    QMenu* m_edit_menu;
    QMenu* m_tools_menu;
    QMenu* m_help_menu;
    std::unique_ptr<DrawingCanvas> m_canvas;
    std::unique_ptr<QToolBar> top_toolbar;
    std::unique_ptr<QToolBar> left_toolbar;
    QAction* m_pen_width_action;

    // methods
    /* 
       create the top menus, File, Edit, Tools and Help
    */
    void createMenus();

    /*
       Create the top toolbar below the menu
    */
    void createToolbars();

    /*
      Add the pen width chices to the toolbar
    */
    QAction* penWidthPickMenu(
        QToolBar*, const char*, const std::function<QIcon(int)>&,
        int, std::function<void(int)>);

    /*
        Shows a contextual menu when selecting a shape followed by a right click
    */
    void showContextMenu(const QPoint &pos);

    /*
      Add the pen width chices to the Edit Menu
    */
    void penWidthPickMenu(QMenu *editMenu);

    /*
       color picker
    */
    QAction* colorPick(
        QWidget*, const char *, const std::function<QIcon(QColor)>&,
        QColor, std::function<void(const QColor&)>);
    /*
       connect to the contextual menu the pen style sub menu
    */
    void connectPenStyleSubmenu(QMenu&, const QFont&);

    /*
       connect to the contextual menu the pen width sub menu
    */
    void connectPenWidthSubmenu(QMenu&, const QFont&);

    /*
       connect to the contextual menu the line end point submenu
    */
    void connectLineEndPointSubmenu(QMenu&, const QFont&);

    /*
       connect to the contextual menu the line base point submenu
    */
    void connectLineBasePointSubmenu(QMenu&, const QFont&);

    /*
       serialize existing shapes into a .vec file
    */
    void triggerSaveAsOption(const QAction*);
};

#endif