#include "shapes.h"
#include "drawing_canvas.h"
#include "icons.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
        ,m_width_action(nullptr) {
        m_canvas = std::make_unique<DrawingCanvas>(this);
        setCentralWidget(m_canvas.get());
        setWindowTitle("Paint-brush (Qt6)");
        resize(1000, 600);
        createMenus();
        // right click menu
        this->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QWidget::customContextMenuRequested, 
            this, &MainWindow::showContextMenu);

    }
    ~MainWindow() {
    }

private:
    QAction* penWidthPickMenu(QToolBar* toolbar, 
                          const char* text, 
                          const std::function<QIcon(int)>& icon_creator,
                          int initialWidth, 
                          std::function<void(int)> onWidthChanged) {
        QAction* penWidthAction = new QAction(QObject::tr(text), toolbar);

        // Use custom icon creator if provided, otherwise generate sample line preview
        auto getIcon = [icon_creator](int w) -> QIcon {
            return icon_creator ? icon_creator(w) : createLinePreviewIcon(w);
        };
        penWidthAction->setIcon(getIcon(initialWidth));
        QMenu* menu = new QMenu(toolbar);
        menu->setStyleSheet(
            "QMenu::icon {"
            "    width: 48px;"
            "    height: 20px;"
            "}"
            "QMenu::item {"
            "    padding: 6px 20px 6px 10px;"
            "}"
        );

        // Helper lambda to update state and trigger callback
        auto applyWidth = [getIcon, penWidthAction, onWidthChanged](int newWidth) {
            penWidthAction->setIcon(getIcon(newWidth));
            if (onWidthChanged) {
                onWidthChanged(newWidth);
            }
        };
        // Populate preset menu actions with icons
        for (int w : preset_widths) {
            QString itemText = QString("%1 px").arg(w);
            QIcon widthIcon = createPenWidthIcon(w, QSize(48, 20));
            QAction* itemAction = menu->addAction(widthIcon, itemText);
            // Attach the preview icon
            itemAction->setIcon(getIcon(w));
            itemAction->setIconVisibleInMenu(true);
            QObject::connect(itemAction, &QAction::triggered, toolbar, [applyWidth, w]() {
                applyWidth(w);
            });
        }
        // Attach menu to toolbar button
        penWidthAction->setMenu(menu);
        toolbar->addAction(penWidthAction);
        // Configure the toolbar button
        if (QToolButton* btn = qobject_cast<QToolButton*>(toolbar->widgetForAction(penWidthAction))) {
            btn->setPopupMode(QToolButton::InstantPopup);
            btn->setIconSize(QSize(48, 20)); // Ensure the toolbar button displays the wide icon
        }
        return penWidthAction;
    }

    void showContextMenu(const QPoint &pos) {
        /*
            Context menu popping-up on mouse right click
        */
        QMenu contextMenu(tr("Context Menu"), this);
        QFont font = contextMenu.font();
        font.setPointSize(13);
        contextMenu.setFont(font);
    
        Shape* shape = m_canvas->isShapeSelected();

        if (shape != nullptr) {
            // the sape is selected
            QAction *copy_action = contextMenu.addAction("Clone");
            connect(copy_action, &QAction::triggered, this, [this]() {m_canvas->cloneShape();});

            QAction *action_zoom_in = contextMenu.addAction("Zoom In");
            connect(action_zoom_in, &QAction::triggered, this, [this]() {m_canvas->setShapeZoomFactor(1.1);});

            QAction *action_zoom_out = contextMenu.addAction("Zoom Out");
            connect(action_zoom_out, &QAction::triggered, this, [this]() {m_canvas->setShapeZoomFactor(0.9);});

            QAction *action_remove = contextMenu.addAction("Delete");
            connect(action_remove, &QAction::triggered, this, [this]() {m_canvas->removeShape();});

            contextMenu.addSeparator();

            QAction* penColorAction = colorPick(&contextMenu, "Pen Color", createPencilIcon, Qt::white, [this](const QColor& c) {
                drawing_properties_t data;
                data.line_color = c;
                m_canvas->setDrawingProperties(data);
            });
            // Add action to context menu
            contextMenu.addAction(penColorAction);

            // add a sub-menu for different line styles
            connectSubmenu(contextMenu, font);

            if (shape->type() == ShapeType::Circle
                || shape->type() == ShapeType::Rectangle
                || shape->type() == ShapeType::Polygon) {
                // only for thos shapes makes sens to fill them
                QAction* brushColorAction = colorPick(&contextMenu, "Brush Color", createPencilIcon, Qt::white, [this](const QColor& c) {
                    drawing_properties_t data;
                    data.brush_color = c;
                    m_canvas->setDrawingProperties(data);
                });
                // Add action to context menu
                contextMenu.addAction(brushColorAction);

                QAction* transparentBrushAction = contextMenu.addAction("Transparent");               
                // Add action to context menu
                connect(transparentBrushAction, &QAction::triggered, this, [this]() {
                    drawing_properties_t data;
                    data.brush_color = Qt::NoBrush;
                    m_canvas->setDrawingProperties(data);
                });
            }
        }
        else if (m_canvas->isGrouping()) {
            QAction *action_zoom_in = contextMenu.addAction("Zoom In");
            connect(action_zoom_in, &QAction::triggered, this, [this]() {m_canvas->setGroupZoomFactor(1.1);});

            QAction *action_zoom_out = contextMenu.addAction("Zoom Out");
            connect(action_zoom_out, &QAction::triggered, this, [this]() {m_canvas->setGroupZoomFactor(0.9);});

            QAction *action_remove = contextMenu.addAction("Remove");
            connect(action_remove, &QAction::triggered, this, [this]() {m_canvas->removeGroup();});
        }
        // last step of Polygon drawing
        else if (m_canvas->getToolMode() == ToolMode::Polygon){
            QAction *action_keep = contextMenu.addAction("Done");
            connect(action_keep, &QAction::triggered, this, [this]() {m_canvas->restoreShape();});
        }
        contextMenu.exec(this->mapToGlobal(pos));
    }

    void addPenMenuToEdit(QMenu *editMenu) {
        QMenu *penSubMenu = editMenu->addMenu(tr("&Pen Width"));

        // Set QSS on penSubMenu to prevent icons from being clipped/squished
        penSubMenu->setStyleSheet(
            "QMenu::icon {"
            "    width: 48px;"
            "    height: 20px;"
            "}"
            "QMenu::item {"
            "    padding: 6px 20px 6px 10px;"
            "}"
        );
        auto *actionGroup = new QActionGroup(penSubMenu);
        actionGroup->setExclusive(true);

        for (const auto& w : preset_widths) {
            QString text = QString("%1 px").arg(w);
            // Generate line width preview icon
            QIcon widthIcon = createPenWidthIcon(w, QSize(48, 20));
            QAction *penWidthAction = penSubMenu->addAction(widthIcon, text);
            
            // Force icon visibility on macOS
            penWidthAction->setIconVisibleInMenu(true);
            penWidthAction->setCheckable(true);
            penWidthAction->setData(w); // Store numeric pixel width in action data
            actionGroup->addAction(penWidthAction);
            // Default to 2px checked
            if (w == 2) {
                penWidthAction->setChecked(true);
            }
            // Trigger width change on click
            connect(penWidthAction, &QAction::triggered, this, [this, w]() {
                drawing_properties_t data;
                data.pen_width = w;
                m_canvas->setDrawingProperties(data);
            });
        }
    }
    
    QAction* colorPick(QWidget* parent, const char *text, const std::function<QIcon(QColor)>& icon_creator, QColor initialColor, std::function<void(const QColor&)> onColorChanged) {
        // 1. Create the action using the passed parent
        QAction* colorAction = new QAction(QObject::tr(text), parent);
        if (icon_creator) {
            colorAction->setIcon(icon_creator(initialColor));
        }
        if (QToolBar* toolbar = qobject_cast<QToolBar*>(parent)) {
            toolbar->addAction(colorAction);
        }
        // Connect the trigger signal to open the color dialog
        QObject::connect(colorAction, &QAction::triggered, parent, [parent, icon_creator, colorAction, initialColor, onColorChanged]() mutable {
            QColor chosen = QColorDialog::getColor(initialColor, parent, QObject::tr("Select Color"));
            if (chosen.isValid()) {
                initialColor = chosen; // Update local state for next time the dialog opens
                if (icon_creator) {
                    colorAction->setIcon(icon_creator(chosen));
                }
                if (onColorChanged) {
                    onColorChanged(chosen); // Execute callback
                }
            }
        });
        return colorAction;
    }

    void connectSubmenu(QMenu& contextMenu, const QFont& font) {
        //set-up the Pen Style sub menu
        QMenu* subMenu = contextMenu.addMenu(tr("Pen Style"));
        subMenu->setStyleSheet("QMenu::icon { width: 48px; height: 20px; }");
        subMenu->setFont(font);
        // Supported line types
        const std::vector<Qt::PenStyle> styles = {
            Qt::SolidLine,
            Qt::DashLine,
            Qt::DotLine,
            Qt::DashDotLine,
            Qt::DashDotDotLine
        };
        // Populate sub-menu with QLine previews
        for (Qt::PenStyle style : styles) {
            // Create an action with the human-readable text
            QString label = penStyleToText(style);
            QAction* act = subMenu->addAction(label);
            // Attach the QLineF preview icon for this pen style
            act->setIcon(createPenStylePreviewIcon(style, 3, QSize(48, 20), Qt::black));
            // Mark currently active style as checked
            act->setCheckable(true);
            // act->setChecked(m_pen_style == style);
            act->setIconVisibleInMenu(true);
            // Handle selection
            connect(act, &QAction::triggered, this, [this, style]() {
                m_pen_style = style;
                update(); // Repaint canvas with updated style
            });
        }
    }

    void createMenus() {
        // File Menu
        // New option
        //------------
        assert(m_canvas);
    
        QMenu *fileMenu = menuBar()->addMenu("&File");
        QAction *newAction = new QAction("&New", this);
        newAction->setShortcut(QKeySequence::New); // Ctrl+N
        connect(newAction, &QAction::triggered, this, [this]() {
            qDebug() << "File -> New selected";
        });
        fileMenu->addAction(newAction);

        //Open option
        //-----------
        QAction *openAction = new QAction("&Open", this);
        openAction->setShortcut(QKeySequence::Open); // Ctrl+Q
        connect(openAction, &QAction::triggered, this, [this]() {
            QString filter = "Vector Graphic Files (*.vec);;All Files (*)";
            QString filePath = QFileDialog::getOpenFileName(
            this, 
            "File open", 
            QDir::currentPath(),
            "Files .vec (*.*)"
        );
        if (filePath.isEmpty()) {
            return;
        }
        if (!m_canvas->loadFromFile(filePath)) {
            QMessageBox::warning(this, tr("OPen Error"), tr("Failed to open the file."));
        }
        });
        fileMenu->addAction(openAction);

        // Save option
        //-------------
        QAction *saveAction = new QAction("&Save", this);
        saveAction->setShortcut(QKeySequence::Save); // Ctrl+Q
        connect(saveAction, &QAction::triggered, this, [this]() {
            QString filter = "Vector Graphic Files (*.vec);;All Files (*)";
            QString filePath = QFileDialog::getSaveFileName(
                this,
                tr("Salvează desen vectorial"),
                QString(),
                filter
            );
            if (filePath.isEmpty()) {
                return;
            }
            if (!m_canvas->saveToFile(filePath)) {
                QMessageBox::warning(this, tr("Save Error"), tr("Failed to save the file."));
            }
        });
        fileMenu->addAction(saveAction);

        fileMenu->addSeparator();

        // Exit option
        //-------------
        QAction *exitAction = new QAction("E&xit", this);
        exitAction->setShortcut(QKeySequence::Quit); // Ctrl+Q
        connect(exitAction, &QAction::triggered, this, [this]() {
            QApplication::quit();
        });
        fileMenu->addAction(exitAction);

        // Tools Menu
        // Line option
        //-------------
        QMenu *toolsMenu = menuBar()->addMenu("&Tools");
        QAction *lineAction = new QAction("&Line Mode", this);
        connect(lineAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Line);
        });
        toolsMenu->addAction(lineAction);

        // Circle option
        //--------------
        QAction *circleAction = new QAction("&Circle Mode", this);
        connect(circleAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Circle);
        });
        toolsMenu->addAction(circleAction);

        // Rectangle option
        //-----------------
        QAction *rectangleAction = new QAction("&Rectangle Mode", this);
        connect(rectangleAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Rectangle);
        });
        toolsMenu->addAction(rectangleAction);

        // Polygon option
        //---------------
        QAction *polygonAction = new QAction("&Polygon Mode", this);
        connect(polygonAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Polygon);
        });
        toolsMenu->addAction(polygonAction);

        // Arc option
        //------------
        QAction *arcAction = new QAction("&Arc Mode", this);
        connect(arcAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Arc);
        });
        toolsMenu->addAction(arcAction);

        // Edit Menu
        //------------
        QMenu *editMenu = menuBar()->addMenu("&Edit");
        // Undo option
        //-------------
        QAction *undoAction = new QAction("&Undo", this);
        undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
        connect(undoAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::undoLast);
        editMenu->addAction(undoAction);

        // Add the pen width menu
        addPenMenuToEdit(editMenu);

        // Clear all option
        //-----------------
        QAction *clearAction = new QAction("&Clear All", this);
        connect(clearAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::clearAll);
        editMenu->addAction(clearAction);

        // Zoom In option
        //----------------
        QAction *zoomInAction = new QAction("&Zoom In", this);
        connect(zoomInAction, &QAction::triggered, this, [this]() {
            m_canvas->zommInOut(1.2);
        });
        editMenu->addAction(zoomInAction);

        // Zoom Out option
        //----------------
        QAction *zoomOutAction = new QAction("&Zoom Out", this);
        connect(zoomOutAction, &QAction::triggered, this, [this]() {
            m_canvas->zommInOut(0.8);
        });
        editMenu->addAction(zoomOutAction);

        // Help Menu
        QMenu *helpMenu = menuBar()->addMenu("&Help");
        // About option
        //--------------
        QAction *aboutAction = new QAction("&About", this);
        connect(aboutAction, &QAction::triggered, this, [this]() {
            QMessageBox::about(this, "About", "Qt Paint 0.1");
        });
        helpMenu->addAction(aboutAction);

        // Create the Toolbars
        // Left toolbar, Pen and Brush pickers
        //-------------------------------------
        left_toolbar = std::make_unique<QToolBar>("Pencils", this);
        addToolBar(Qt::LeftToolBarArea, left_toolbar.get());
        // Drawing color picker
        colorPick(left_toolbar.get(), "Pen Color", createPencilIcon, Qt::white, [this](const QColor& c) {
            drawing_properties_t data;
            data.line_color = c;
            m_canvas->setDrawingProperties(data);
            m_width_action->setIcon(createWidthIcon(m_width, c));
        });

        // Brush color picker
        colorPick(left_toolbar.get(), "Brush Color", createBrushIcon, Qt::white, [this](const QColor& c) {
            drawing_properties_t data;
            data.brush_color = c;
            m_canvas->setDrawingProperties(data);
        });

        // Pen width picker
        auto drawing_color = m_canvas->getDrawingColor();
        m_width_action = penWidthPickMenu(
            left_toolbar.get(), 
            "Pen Width", 
            // Capture drawing_color by value so icon_creator matches std::function<QIcon(int)>
            [drawing_color](int w) { 
                return createWidthIcon(w, drawing_color); 
            }, 
            2,  // initialWidth
            // Callback signature only receives (int width)
            [this](int width) {
                drawing_properties_t data;
                data.pen_width = width;
                m_canvas->setDrawingProperties(data);
                m_width = width;
            }
        );

        // top tool bar, Line, Circle, Rectangle, Polygon and Select actions
        //------------------------------------------------------------------
        top_toolbar = std::make_unique<QToolBar>("Shape Tools", this);
        QActionGroup *toolGroup = new QActionGroup(this);
        toolGroup->setExclusive(true);
        addToolBar(Qt::TopToolBarArea, top_toolbar.get());
        top_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        // 1. Add Line Action with custom icon
        QAction *lineToolBarAction = new QAction(createLineIcon(), "Line", this);
        lineToolBarAction->setCheckable(true); // Useful for toggle mode selection
        top_toolbar->addAction(lineToolBarAction);
        toolGroup->addAction(lineToolBarAction);
        connect(lineToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Line);
        });

        // 2. Add Circle Action with custom icon
        QAction *circleToolBarAction = new QAction(createCircleIcon(), "Circle", this);
        circleToolBarAction->setCheckable(true);
        top_toolbar->addAction(circleToolBarAction);
        toolGroup->addAction(circleToolBarAction);
        connect(circleToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Circle);
        });

        // 3. Add Rectangle Action with custom icon
        QAction *rectangleToolBarAction = new QAction(createRectangleIcon(), "Rectangle", this);
        rectangleToolBarAction->setCheckable(true);
        top_toolbar->addAction(rectangleToolBarAction);
        toolGroup->addAction(rectangleToolBarAction);
        connect(rectangleToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Rectangle);
        });

        // 4. Add Polygon Action with custom icon
        QAction *polygonToolBarAction = new QAction(createPolygonIcon(), "Polygon", this);
        polygonToolBarAction->setCheckable(true);
        top_toolbar->addAction(polygonToolBarAction);
        toolGroup->addAction(polygonToolBarAction);
        connect(polygonToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Polygon);
        });

        // 5. Add Polygon Action with custom icon
        QAction *arcToolBarAction = new QAction(createArcToolIcon(), "Arc", this);
        arcToolBarAction->setCheckable(true);
        top_toolbar->addAction(arcToolBarAction);
        toolGroup->addAction(arcToolBarAction);
        connect(arcToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Arc);
        });

        // 6. Add Select Action with custom icon
        QAction *selectToolBarAction = new QAction(createSelectIcon(), "Select", this);
        selectToolBarAction->setCheckable(true);
        top_toolbar->addAction(selectToolBarAction);
        toolGroup->addAction(selectToolBarAction);
        connect(selectToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Select);
        });

        // 7. Add Group shapes option
        QAction *groupToolBarAction = new QAction(createGroupIcon(), "Group", this);
        groupToolBarAction->setCheckable(true);
        top_toolbar->addAction(groupToolBarAction);
        toolGroup->addAction(groupToolBarAction);
        connect(groupToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Group);
        });
    }

    // private data
    int m_width;
    Qt::PenStyle m_pen_style;
    std::unique_ptr<DrawingCanvas> m_canvas;
    std::unique_ptr<QToolBar> top_toolbar;
    std::unique_ptr<QToolBar> left_toolbar;
    QAction* m_width_action;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFont font = app.font();
    font.setPointSize(11); 
    app.setFont(font);
    MainWindow window;
    window.show();
    return app.exec();
}