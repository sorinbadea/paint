#include "main.h"

void MainWindow::createMenus() {
    //----------------- File Menu -----------------
    // New option
    //------------
    assert(m_canvas);

    m_file_menu = menuBar()->addMenu("&File");
    QAction *newAction = new QAction("&New", this);
    newAction->setShortcut(QKeySequence::New); // Ctrl+N
    connect(newAction, &QAction::triggered, this, [this]() {
        qDebug() << "File -> New selected";
    });
    m_file_menu->addAction(newAction);

    //Open option
    //-----------
    QAction *openAction = new QAction("&Open", this);
    openAction->setShortcut(QKeySequence::Open); // Ctrl+Q
    connect(openAction, &QAction::triggered, this, [this]() {
        QString filter = "Vector Graphic Files (*.vec);;All Files (*)";
        m_file_path = QFileDialog::getOpenFileName(
        this, 
        "File open", 
        QDir::currentPath(),
        "Files .vec (*.*)"
    );
    if (m_file_path.isEmpty()) {
        return;
    }
    if (!m_canvas->loadFromFile(m_file_path)) {
        QMessageBox::warning(this, tr("Open Error"), tr("Failed to open the file."));
    }
    });
    m_file_menu->addAction(openAction);

    // Save option
    //------------
    QAction *saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save); // Ctrl S
    connect(saveAction, &QAction::triggered, this, [this, saveAction]() {
        if (m_file_path.isEmpty()) {
            // file does not exists
            triggerSaveAsOption(saveAction);
        }
        else {
            // file exists
            if (!m_canvas->saveToFile(m_file_path))
                QMessageBox::warning(this, tr("Save Error"), tr("Failed to save the file."));
        }
    });
    m_file_menu->addAction(saveAction);

    // Save as option
    //---------------
    QAction *saveAsAction = new QAction("Save as", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    triggerSaveAsOption(saveAsAction);
    m_file_menu->addAction(saveAsAction);

    m_file_menu->addSeparator();

    // Exit option
    //-------------
    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit); // Ctrl+Q
    exitAction->setMenuRole(QAction::NoRole);    // Mac OS stuff
    connect(exitAction, &QAction::triggered, this, [this]() {
        QApplication::quit();
    });
    m_file_menu->addAction(exitAction);

    //------------------ Edit Menu --------------
    m_edit_menu = menuBar()->addMenu("&Edit");
    // Undo option
    //-------------
    QAction *undoAction = new QAction("&Undo shape", this);
    undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
    connect(undoAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::undoLast);
    m_edit_menu->addAction(undoAction);

    // Add the pen width menu
    penWidthPickMenu(m_edit_menu);

    // Clear all option
    //-----------------
    QAction *clearAction = new QAction("&Clear All", this);
    connect(clearAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::clearAll);
    m_edit_menu->addAction(clearAction);

    // Zoom In option
    //----------------
    QAction *zoomInAction = new QAction("&Zoom In", this);
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        m_canvas->zommInOut(1.2);
    });
    m_edit_menu->addAction(zoomInAction);

    // Zoom Out option
    //----------------
    QAction *zoomOutAction = new QAction("&Zoom Out", this);
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        m_canvas->zommInOut(0.8);
    });
    m_edit_menu->addAction(zoomOutAction);

    //------------------- Tools Menu --------------------
    // Line option
    //-------------
    m_tools_menu = menuBar()->addMenu("&Tools");
    QAction *lineAction = new QAction("&Line Mode", this);
    connect(lineAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Line);
    });
    m_tools_menu->addAction(lineAction);

    // Circle option
    //--------------
    QAction *circleAction = new QAction("&Circle Mode", this);
    connect(circleAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Circle);
    });
    m_tools_menu->addAction(circleAction);

    // Rectangle option
    //-----------------
    QAction *rectangleAction = new QAction("&Rectangle Mode", this);
    connect(rectangleAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Rectangle);
    });
    m_tools_menu->addAction(rectangleAction);

    // Polygon option
    //---------------
    QAction *polygonAction = new QAction("&Polygon Mode", this);
    connect(polygonAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Polygon);
    });
    m_tools_menu->addAction(polygonAction);

    // Arc option
    //------------
    QAction *arcAction = new QAction("&Arc Mode", this);
    connect(arcAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Arc);
    });
    m_tools_menu->addAction(arcAction);

    // Arc option
    //------------
    QAction *textAction = new QAction("&Text mode", this);
    connect(textAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Text);
    });
    m_tools_menu->addAction(textAction);

    // Help Menu
    m_help_menu = menuBar()->addMenu("&Help");
    // About option
    //--------------
    QAction *aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About", "Qt Paint 0.1");
    });
    m_help_menu->addAction(aboutAction);
}

void MainWindow::createToolbars() {
    // Create the Toolbars
    // Left toolbar, Pen and Brush pickers
    //-------------------------------------
    assert(m_canvas);
    left_toolbar = std::make_unique<QToolBar>("Pencils", this);
    addToolBar(Qt::LeftToolBarArea, left_toolbar.get());
    // Drawing color picker
    colorPick(left_toolbar.get(), "Pen Color", createPencilIcon, Qt::white, [this](const QColor& c) {
        drawing_properties_t data;
        data.line_color = c;
        m_canvas->setDrawingProperties(data);
        m_pen_width_action->setIcon(createLinePreviewIcon(Qt::SolidLine, m_pen_width, QSize(48, 20), c));
    });

    // Brush color picker
    colorPick(left_toolbar.get(), "Brush Color", createBrushIcon, Qt::white, [this](const QColor& c) {
        drawing_properties_t data;
        data.brush_color = c;
        m_canvas->setDrawingProperties(data);
    });

    // Pen width picker
    auto drawing_color = m_canvas->getDrawingColor();
    m_pen_width_action = penWidthPickMenu(
        left_toolbar.get(), 
        "Pen Width", 
        // Capture drawing_color by value so icon_creator matches std::function<QIcon(int)>
        [drawing_color](int w) { 
            return createLinePreviewIcon(Qt::SolidLine, w,  QSize(48, 20), Qt::black); 
        }, 
        2,  // initialWidth
        // Callback signature only receives (int width)
        [this](int width) {
            drawing_properties_t data;
            data.pen_width = width;
            m_canvas->setDrawingProperties(data);
            m_pen_width = width;
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

    // 5. Add Arc Action with custom icon
    QAction *arcToolBarAction = new QAction(createArcToolIcon(), "Arc", this);
    arcToolBarAction->setCheckable(true);
    top_toolbar->addAction(arcToolBarAction);
    toolGroup->addAction(arcToolBarAction);
    connect(arcToolBarAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Arc);
    });

    // 6. Add Text shape option
    QAction *textIconAction = new QAction(createAbIcon(), "Text", this);
    textIconAction->setCheckable(true);
    top_toolbar->addAction(textIconAction);
    toolGroup->addAction(textIconAction);
    connect(textIconAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Text);
    });

    // 7. Add Select Action with custom icon
    QAction *selectToolBarAction = new QAction(createSelectIcon(), "Select", this);
    selectToolBarAction->setCheckable(true);
    top_toolbar->addAction(selectToolBarAction);
    toolGroup->addAction(selectToolBarAction);
    connect(selectToolBarAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Select);
    });

    // 8. Add Group shapes option
    QAction *groupToolBarAction = new QAction(createGroupIcon(), "Group", this);
    groupToolBarAction->setCheckable(true);
    top_toolbar->addAction(groupToolBarAction);
    toolGroup->addAction(groupToolBarAction);
    connect(groupToolBarAction, &QAction::triggered, this, [this]() {
        m_canvas->setMode(ToolMode::Group);
    });
}

void MainWindow::showContextMenu(const QPoint &pos) {
    /*
        Context menu popping-up on mouse right click
    */
    assert(m_canvas);
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

        if (shape->type() == ShapeType::Text) {
            QAction* textColorAction = colorPick(&contextMenu, "Text Color", createPencilIcon, Qt::white, [this](const QColor& c) {
                drawing_properties_t data;
                data.line_color = c;
                m_canvas->setDrawingProperties(data);
            });
            contextMenu.addAction(textColorAction);

            // Add the normal text menu
            drawing_properties_t data;
            QAction *action_normal_text = contextMenu.addAction("Normal");
            data.bold_text = false;
            connect(action_normal_text, &QAction::triggered, this, [this, data]() {m_canvas->setDrawingProperties(data);});

            // Add bold text menu
            QAction *action_bold_text = contextMenu.addAction("Bold");
            data.bold_text = true;
            connect(action_bold_text, &QAction::triggered, this, [this, data]() {m_canvas->setDrawingProperties(data);});
        }
        else {

            QAction* penColorAction = colorPick(&contextMenu, "Pen Color", createPencilIcon, Qt::white, [this](const QColor& c) {
                drawing_properties_t data;
                data.line_color = c;
                m_canvas->setDrawingProperties(data);
            });
            // Add action to context menu
            contextMenu.addAction(penColorAction);

            // add a sub-menu for different line styles
            connectPenStyleSubmenu(contextMenu, font);
            connectPenWidthSubmenu(contextMenu, font);
            if (shape->type() == ShapeType::Line) {
                connectLineEndPointSubmenu(contextMenu, font);
                connectLineBasePointSubmenu(contextMenu, font);
            }

            if (shape->type() == ShapeType::Circle
                || shape->type() == ShapeType::Rectangle
                || shape->type() == ShapeType::Polygon) {
                // add brush color picker for those shapes
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
    }
    else if (m_canvas->isGrouping()) {

        QAction *action_clone = contextMenu.addAction("Clone");
        connect(action_clone, &QAction::triggered, this, [this]() {m_canvas->cloneGroup();});

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
