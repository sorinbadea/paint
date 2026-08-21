#include "shapes.h"
#include "drawing_canvas.h"

#include <QFileDialog>
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

// Helper function to dynamically draw a simple Line icon
QIcon createLineIcon() {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
    // Draw diagonal line
    painter.drawLine(3, 23, 23, 3);
    return QIcon(pixmap);
}

// Helper function to dynamically draw a simple Circle icon
QIcon createCircleIcon() {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red, 3));
    painter.setBrush(Qt::NoBrush);
    // Draw circle in center
    painter.drawEllipse(5, 5, 22, 22);
    return QIcon(pixmap);
}

QIcon createDragIcon(int size = 32, const QColor &dotColor = QColor(100, 100, 100)) {
    // 1. Create a transparent pixmap buffer
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(dotColor);

    // 2. Define grid parameters (2 columns x 3 rows of grip dots)
    int dotRadius = size / 12; // Radius of each dot
    if (dotRadius < 2) dotRadius = 2;

    int spacingY = size / 4;  // Vertical distance between dots
    int col1_X = size / 3;    // Left column X
    int col2_X = (size * 2) / 3; // Right column X
    int startY = size / 4;    // Top row Y

    // 3. Draw the 6 dots forming a vertical drag grip handle
    for (int i = 0; i < 3; ++i) {
        int y = startY + (i * spacingY);
        
        // Left column dot
        painter.drawEllipse(QPoint(col1_X, y), dotRadius, dotRadius);
        // Right column dot
        painter.drawEllipse(QPoint(col2_X, y), dotRadius, dotRadius);
    }
    return QIcon(pixmap);
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        m_canvas = std::make_unique<DrawingCanvas>(this);
        setCentralWidget(m_canvas.get());
        setWindowTitle("Paint brush (Qt6)");
        resize(800, 600);
        createMenus();
    }

private:
    QIcon createPenWidthIcon(int width) {
        QPixmap pixmap(40, 16);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        
        QPen pen(Qt::black, width, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(2, 8, 38, 8); // Draw stroke preview

        return QIcon(pixmap);
    }
    void addPenMenuToEdit(QMenu *editMenu) {
        // 1. Create a "Pen Width" sub-menu inside Edit
        QMenu *penSubMenu = editMenu->addMenu("&Pen Width");

        // Sample width values
        const QVector<int> widths = {1, 2, 3, 5, 8, 12};

        // --- Option A: Standard QAction list with visual icons ---
        auto *actionGroup = new QActionGroup(penSubMenu);
        actionGroup->setExclusive(true);

        for (int w : widths) {
            QString text = QString("%1 px").arg(w);
            QAction *widthAction = penSubMenu->addAction(createPenWidthIcon(w), text);
            widthAction->setCheckable(true);
            widthAction->setData(w); // Store numeric pixel width in action data

            actionGroup->addAction(widthAction);

            // Default to 2px checked
            if (w == 2) {
                widthAction->setChecked(true);
            }

            // Trigger width change on click
            connect(widthAction, &QAction::triggered, this, [this, w]() {
                m_canvas->setPenWidth(w);
            });
        }
    }

    void createMenus() {
        // File Menu
        // New option
        //------------
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
            "Fișiere .vec (*.*)"
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

        // Edit Menu
        //------------
        QMenu *editMenu = menuBar()->addMenu("&Edit");
        // Undo option
        //-------------
        QAction *undoAction = new QAction("&Undo", this);
        undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
        connect(undoAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::undoLast);
        editMenu->addAction(undoAction);

        // Add the pen width selection menu
        addPenMenuToEdit(editMenu);

        // Clear option
        //-------------
        QAction *clearAction = new QAction("&Clear All", this);
        connect(clearAction, &QAction::triggered, m_canvas.get(), &DrawingCanvas::clearAll);
        editMenu->addAction(clearAction);

        // Help Menu
        QMenu *helpMenu = menuBar()->addMenu("&Help");
        // About option
        //--------------
        QAction *aboutAction = new QAction("&About", this);
        connect(aboutAction, &QAction::triggered, this, [this]() {
            QMessageBox::about(this, "About", "Qt Menu Bar Example");
        });
        helpMenu->addAction(aboutAction);

        // Add toolbar
        // 1. Create the Toolbar
        QToolBar *toolbar = addToolBar("Shape Tools");
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        // 2. Add Line Action with custom icon
        QAction *lineToolBarAction = new QAction(createLineIcon(), "Line", this);
        lineToolBarAction->setCheckable(true); // Useful for toggle mode selection
        toolbar->addAction(lineToolBarAction);
        connect(lineToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Line);
        });

        // 3. Add Circle Action with custom icon
        QAction *circleToolBarAction = new QAction(createCircleIcon(), "Circle", this);
        circleToolBarAction->setCheckable(true);
        toolbar->addAction(circleToolBarAction);
        connect(circleToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Circle);
        });

        // 3. Add Select Action with custom icon
        QAction *selectToolBarAction = new QAction(createDragIcon(), "Select", this);
        selectToolBarAction->setCheckable(true);
        toolbar->addAction(selectToolBarAction);
        connect(selectToolBarAction, &QAction::triggered, this, [this]() {
            m_canvas->setMode(ToolMode::Select);
        });

    }

private:
    //DrawingCanvas *m_canvas;
    std::unique_ptr<DrawingCanvas> m_canvas;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFont font = app.font();
    font.setPointSize(10); 
    app.setFont(font);
    MainWindow window;
    window.show();
    return app.exec();
}