#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QList>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTimer>
#include "components/TopTaskBar.h"
#include "components/ImagePanel.h"

class AppController;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Accessors for AppController
    TopTaskBar* getTopTaskBar()               { return taskBar; }
    QList<ImagePanel*>& getInputPanels()      { return inputPanels; }
    QList<ImagePanel*>& getOutputPanels()     { return outputPanels; }
    QTextBrowser* getInfoSidebar()            { return infoSidebar; }

    void updateLayoutForTask(int taskIndex);
    void setStatusMessage(const QString& msg, bool success = true);

private:
    // UI regions
    QWidget* leftRail;           // Sidebar navigation
    QWidget* mainArea;           // Right: header + canvas
    QWidget* headerBar;          // Top app bar
    TopTaskBar* taskBar;         // Parameter strip
    QWidget* canvasArea;         // Image panels grid
    QSplitter* mainSplitter;     // Left rail | content
    QSplitter* contentSplitter;  // Canvas | info sidebar
    QSplitter* leftSplitter;     // Input panels (vertical)
    QSplitter* rightSplitter;    // Output panels (vertical)
    QTextBrowser* infoSidebar;

    // Sidebar nav buttons
    QList<QPushButton*> taskButtons;
    QLabel* logoLabel;
    QLabel* versionLabel;

    // Panel management
    QList<ImagePanel*> inputPanels;
    QList<ImagePanel*> outputPanels;

    AppController* appController;

    void buildLeftRail();
    void buildHeaderBar();
    void buildCanvasArea();
    void setActiveTask(int taskIndex);
    void rebuildPanels(int numInputs, int numOutputs,
                       QStringList inTitles, QStringList outTitles);
};

#endif // MAINWINDOW_H