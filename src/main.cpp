#include "main.h"

MainWindow::MainWindow(QWidget *parent)
    : m_file_path(""), QMainWindow(parent),
    m_pen_width_action(nullptr),
    m_canvas(std::make_unique<DrawingCanvas>(this)) {
        setCentralWidget(m_canvas.get());
        setWindowTitle("Paint-brush (Qt6)");
        resize(1000, 600);
        createMenus();
        createToolbars();
        // enable right click menu
        this->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

MainWindow::~MainWindow() {
}

QAction* MainWindow::penWidthPickMenu(QToolBar* toolbar, const char* text, const std::function<QIcon(int)>& icon_creator, int initialWidth, std::function<void(int)> onWidthChanged) {
    QAction* penWidthAction = new QAction(QObject::tr(text), toolbar);
    assert(penWidthAction);
    // Use custom icon creator if provided, otherwise generate sample line preview
    auto getIcon = [icon_creator](int w) -> QIcon {
        return icon_creator ? icon_creator(w) : createLinePreviewIcon(Qt::SolidLine, w, QSize(48, 20), Qt::black);
    };
    penWidthAction->setIcon(getIcon(initialWidth));
    QMenu* menu = new QMenu(toolbar);
    assert(menu);
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
    auto current_pen_width = m_canvas->getPenWidth();
    // Populate preset menu actions with icons
    for (int w : preset_widths) {
        QString itemText = QString("%1 px").arg(w);
        QIcon widthIcon = createLinePreviewIcon(Qt::SolidLine, w, QSize(48, 20), Qt::black);
        QAction* itemAction = menu->addAction(widthIcon, itemText);
        // Attach the preview icon
        itemAction->setIcon(getIcon(w));
        itemAction->setIconVisibleInMenu(true);
        if (w == current_pen_width) {
            itemAction->setChecked(true);
        }
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

void MainWindow::penWidthPickMenu(QMenu *editMenu) {
    QMenu *penSubMenu = editMenu->addMenu(tr("&Pen Width"));
    assert(penSubMenu);
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

    auto current_pen_width = m_canvas->getPenWidth();
    for (const auto& w : preset_widths) {
        QString text = QString("%1 px").arg(w);
        // Generate line width preview icon
        QIcon widthIcon = createLinePreviewIcon(Qt::SolidLine, w, QSize(48, 20), Qt::black);
        QAction *penWidthAction = penSubMenu->addAction(widthIcon, text);
        
        // Force icon visibility on macOS
        penWidthAction->setIconVisibleInMenu(true);
        penWidthAction->setCheckable(true);
        penWidthAction->setData(w); // Store numeric pixel width in action data
        actionGroup->addAction(penWidthAction);
        if (w == current_pen_width) {
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
    
QAction* MainWindow::colorPick(QWidget* parent, const char *text, const std::function<QIcon(QColor)>& icon_creator, QColor initialColor, std::function<void(const QColor&)> onColorChanged) {
    // Create the action using the passed parent
    QAction* colorAction = new QAction(QObject::tr(text), parent);
    assert(colorAction);
    if (icon_creator) {
        colorAction->setIcon(icon_creator(initialColor));
    }
    if (QToolBar* toolbar = qobject_cast<QToolBar*>(parent)) {
        assert(toolbar);
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

void MainWindow::connectPenStyleSubmenu(QMenu& contextMenu, const QFont& font) {
    //set-up the Pen Style sub menu
    QMenu* subMenu = contextMenu.addMenu(tr("Pen Style"));
    assert(subMenu);
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
        act->setIcon(createLinePreviewIcon(style, 3, QSize(48, 20), Qt::black));
        // Mark currently active style as checked
        act->setCheckable(true);
        // act->setChecked(m_pen_style == style);
        act->setIconVisibleInMenu(true);
        // Handle selection
        connect(act, &QAction::triggered, this, [this, style]() {
            m_pen_style = style;
            drawing_properties_t data;
            data.pen_style = style;
            m_canvas->setDrawingProperties(data);
            update(); // Repaint canvas with updated style
        });
    }
}

void MainWindow::connectPenWidthSubmenu(QMenu& contextMenu, const QFont& font) {
    //set-up the Pen Style sub menu
    QMenu* subMenu = contextMenu.addMenu(tr("Pen Width"));
    assert(subMenu);
    subMenu->setStyleSheet("QMenu::icon { width: 48px; height: 20px; }");
    subMenu->setFont(font);
    for (int width : preset_widths) {
        // Attach the QLineF preview icon for this pen style
        QAction* act = subMenu->addAction(QString::number(width) + "px");
        assert(act);
        act->setIcon(createLinePreviewIcon(Qt::SolidLine, width, QSize(48, 20), Qt::black));
        // Mark currently active style as checked
        act->setCheckable(true);
        act->setIconVisibleInMenu(true);
        // Handle selection
        connect(act, &QAction::triggered, this, [this, width]() {
            drawing_properties_t data;
            data.pen_width = width;
            m_canvas->setDrawingProperties(data);
            update(); // Repaint canvas with updated style
        });
    }
}

void MainWindow::connectLineEndPointSubmenu(QMenu& contextMenu, const QFont& font) {
    //set-up the Pen Style sub menu
    QMenu* subMenu = contextMenu.addMenu(tr("End Point"));
    assert(subMenu);
    subMenu->setStyleSheet("QMenu::icon { width: 48px; height: 20px; }");
    subMenu->setFont(font);

    QAction* actNone = subMenu->addAction("None");
    assert(actNone);
    //act->setIcon(createLinePreviewIcon(Qt::SolidLine, width, QSize(48, 20), Qt::black));
    // Mark currently active style as checked
    actNone->setCheckable(true);
    actNone->setIconVisibleInMenu(true);
    // Handle selection
    connect(actNone, &QAction::triggered, this, [this]() {
        drawing_properties_t data;
        data.endpoint = std::nullopt;
        m_canvas->setDrawingProperties(data);
        update(); // Repaint canvas with updated style
    });

    // Set the solid arrow tip
    QAction* solidArrowAction = subMenu->addAction("Solid Arrow");
    solidArrowAction->setIcon(createArrowPreviewIcon(QSize(56, 56), Qt::black, ArrowEndPoint::SolidArrow));
    // Mark currently active style as checked
    solidArrowAction->setCheckable(true);
    solidArrowAction->setIconVisibleInMenu(true);
    // Handle selection
    connect(solidArrowAction, &QAction::triggered, this, [this]() {
        drawing_properties_t data;
        data.endpoint.emplace().endPoint = ArrowEndPoint::SolidArrow;
        m_canvas->setDrawingProperties(data);
        update(); // Repaint canvas with updated style
    });

    // Set the empty arrow tip
    QAction* emptyArrowAction = subMenu->addAction("Empty Arrow");
    emptyArrowAction->setIcon(createArrowPreviewIcon(QSize(56, 56), Qt::black, ArrowEndPoint::EmptyArrow));
    // Mark currently active style as checked
    emptyArrowAction->setCheckable(true);
    emptyArrowAction->setIconVisibleInMenu(true);
    // Handle selection
    connect(emptyArrowAction, &QAction::triggered, this, [this]() {
        drawing_properties_t data;
        data.endpoint.emplace().endPoint = ArrowEndPoint::EmptyArrow;
        m_canvas->setDrawingProperties(data);
        update(); // Repaint canvas with updated style
    });
}

void MainWindow::connectLineBasePointSubmenu(QMenu& contextMenu, const QFont& font) {
    //set-up the Pen Style sub menu
    QMenu* subMenu = contextMenu.addMenu(tr("Base Point"));
    assert(subMenu);
    subMenu->setStyleSheet("QMenu::icon { width: 48px; height: 20px; }");
    subMenu->setFont(font);

    QAction* actNone = subMenu->addAction("None");
    //act->setIcon(createLinePreviewIcon(Qt::SolidLine, width, QSize(48, 20), Qt::black));
    // Mark currently active style as checked
    actNone->setCheckable(true);
    actNone->setIconVisibleInMenu(true);
    // Handle selection
    connect(actNone, &QAction::triggered, this, [this]() {
        drawing_properties_t data;
        data.endpoint = std::nullopt;
        m_canvas->setDrawingProperties(data);
        update(); // Repaint canvas with updated style
    });

    // Attach the QLineF preview icon for this pen style
    QAction* actBase = subMenu->addAction("Rectangle");
    actBase->setIcon(createRectanglePreviewIcon(QSize(48, 20), Qt::black));
    // Mark currently active style as checked
    actBase->setCheckable(true);
    actBase->setIconVisibleInMenu(true);
    // Handle selection
    connect(actBase, &QAction::triggered, this, [this]() {
        drawing_properties_t data;
        data.endpoint.emplace().basePoint = ArrowBasePoint::SolidRectangle;
        m_canvas->setDrawingProperties(data);
        update(); // Repaint canvas with updated style
    });
}

void MainWindow::triggerSaveAsOption(const QAction* qAction) {
    connect(qAction, &QAction::triggered, this, [this]() {
        QString filter = "Vector Graphic Files (*.vec);;All Files (*)";
        m_file_path = QFileDialog::getSaveFileName(
            this,
            tr("Draw saving.."),
            QString(),
            filter
        );
        if (m_file_path.isEmpty()) {
            return;
        }
        if (!m_canvas->saveToFile(m_file_path)) {
            QMessageBox::warning(this, tr("Save Error"), tr("Failed to save the file."));
        }
    });
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFont font = app.font();
    font.setPointSize(12); 
    app.setFont(font);
    MainWindow window;
    window.show();
    return app.exec();
}
