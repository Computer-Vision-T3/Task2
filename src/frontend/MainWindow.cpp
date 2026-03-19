#include "MainWindow.h"
#include "controllers/AppController.h"
#include "backend/Module2_HoughLines/HoughLines.h" // Added your module include
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QScrollArea>
#include <QGridLayout>
#include <QStyle>

// ── Task 2 metadata ──────────────────────────────────────────────────────────
struct TaskMeta {
    QString icon;
    QString shortName;
    QString group;
};

static const QList<TaskMeta> TASKS = {
    { "◦", "Canny (Scratch)",  "Edge Detection" },
    { "◦", "Hough Lines",      "Hough Transform" },
    { "◦", "Hough Circles",    "Hough Transform" },
    { "◦", "Hough Ellipses",   "Hough Transform" },
    { "◦", "Active Contour",   "Snakes & Analytics" }
};

// ── Constructor ──────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setObjectName("MainWindow");

    QWidget* root = new QWidget(this);
    root->setObjectName("rootWidget");
    root->setStyleSheet("background-color: #F5F3EF;");
    setCentralWidget(root);

    QHBoxLayout* rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Left navigation rail ─────────────────────────────
    buildLeftRail();
    rootLayout->addWidget(leftRail);

    // ── Main content area ────────────────────────────────
    mainArea = new QWidget(this);
    mainArea->setStyleSheet("background-color: #F5F3EF;");
    QVBoxLayout* mainAreaLayout = new QVBoxLayout(mainArea);
    mainAreaLayout->setContentsMargins(0, 0, 0, 0);
    mainAreaLayout->setSpacing(0);

    // Header bar
    buildHeaderBar();
    mainAreaLayout->addWidget(headerBar);

    // Thin separator
    QFrame* headerSep = new QFrame(mainArea);
    headerSep->setObjectName("divider");
    headerSep->setFrameShape(QFrame::HLine);
    headerSep->setFixedHeight(1);
    mainAreaLayout->addWidget(headerSep);

    // Parameter strip (task bar)
    taskBar = new TopTaskBar(mainArea);
    mainAreaLayout->addWidget(taskBar);

    // Thin separator
    QFrame* paramSep = new QFrame(mainArea);
    paramSep->setObjectName("divider");
    paramSep->setFrameShape(QFrame::HLine);
    paramSep->setFixedHeight(1);
    mainAreaLayout->addWidget(paramSep);

    // Canvas + info sidebar
    buildCanvasArea();
    mainAreaLayout->addWidget(canvasArea, 1);

    rootLayout->addWidget(mainArea, 1);

    // ── Connect controller ───────────────────────────────
    appController = new AppController(this);

    // ── Initial layout ───────────────────────────────────
    rebuildPanels(1, 1, {"Source Image"}, {"Binary Edge Map"});
    setActiveTask(1);

    // Connect task change to sync sidebar selection
    connect(taskBar, &TopTaskBar::taskChanged, this, [this](int idx) {
        setActiveTask(idx);
        updateLayoutForTask(idx);
    });
}

MainWindow::~MainWindow() {}

// ── Left Rail ────────────────────────────────────────────────────────────────
void MainWindow::buildLeftRail() {
    leftRail = new QWidget(this);
    leftRail->setObjectName("leftRail");
    leftRail->setFixedWidth(210);

    QVBoxLayout* railLayout = new QVBoxLayout(leftRail);
    railLayout->setContentsMargins(0, 0, 0, 0);
    railLayout->setSpacing(0);

    // ── Logo area ────────────────────────────────────────
    QWidget* logoArea = new QWidget(leftRail);
    logoArea->setFixedHeight(62);
    logoArea->setStyleSheet(
        "background-color: #FDFCFA;"
        "border-bottom: 1px solid #E8E2DA;"
    );
    QVBoxLayout* logoLayout = new QVBoxLayout(logoArea);
    logoLayout->setContentsMargins(18, 0, 18, 0);
    logoLayout->setSpacing(1);
    logoLayout->setAlignment(Qt::AlignVCenter);

    QLabel* logoTop = new QLabel("VISION", logoArea);
    logoTop->setStyleSheet("font-size: 18px; font-weight: 900; letter-spacing: -0.02em; color: #2C2825;");

    QLabel* logoBottom = new QLabel("STUDIO", logoArea);
    logoBottom->setStyleSheet("font-size: 10px; font-weight: 700; letter-spacing: 0.28em; color: #5B4FCF; margin-top: -4px;");

    logoLayout->addWidget(logoTop);
    logoLayout->addWidget(logoBottom);
    railLayout->addWidget(logoArea);

    // ── Task list ────────────────────────────────────────
    QScrollArea* scroll = new QScrollArea(leftRail);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget* taskList = new QWidget();
    taskList->setObjectName("taskListContainer");
    QVBoxLayout* taskLayout = new QVBoxLayout(taskList);
    taskLayout->setContentsMargins(10, 14, 10, 14);
    taskLayout->setSpacing(2);

    QString currentGroup = "";
    for (int i = 0; i < TASKS.size(); ++i) {
        const TaskMeta& meta = TASKS[i];

        // Group header
        if (meta.group != currentGroup) {
            if (!currentGroup.isEmpty()) {
                QFrame* groupSep = new QFrame(taskList);
                groupSep->setFixedHeight(8);
                groupSep->setStyleSheet("background: transparent;");
                taskLayout->addWidget(groupSep);
            }
            currentGroup = meta.group;

            QLabel* groupLabel = new QLabel(meta.group.toUpper(), taskList);
            groupLabel->setObjectName("sectionLabel");
            groupLabel->setContentsMargins(4, 0, 0, 4);
            taskLayout->addWidget(groupLabel);
        }

        // Task button
        QPushButton* btn = new QPushButton(taskList);
        btn->setObjectName("taskItem");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setFixedHeight(36);

        QString numChip = QString("%1").arg(i + 1);
        btn->setText(QString("  %1   %2").arg(numChip).arg(meta.shortName));
        btn->setProperty("active", false);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            QComboBox* sel = taskBar->findChild<QComboBox*>("operationSelector");
            if (sel) sel->setCurrentIndex(i);
        });

        taskButtons.append(btn);
        taskLayout->addWidget(btn);
    }

    taskLayout->addStretch();
    scroll->setWidget(taskList);
    railLayout->addWidget(scroll, 1);

    // ── Bottom rail info ─────────────────────────────────
    QWidget* railFooter = new QWidget(leftRail);
    railFooter->setFixedHeight(52);
    railFooter->setStyleSheet("background-color: #FDFCFA; border-top: 1px solid #E8E2DA;");
    QVBoxLayout* rfLayout = new QVBoxLayout(railFooter);
    rfLayout->setContentsMargins(18, 0, 18, 0);
    rfLayout->setAlignment(Qt::AlignVCenter);

    QLabel* rfLabel = new QLabel("OpenCV · Qt5 · C++17", railFooter);
    rfLabel->setStyleSheet("font-size: 10px; color: #C4BDB4; font-weight: 500;");

    QLabel* rfVersion = new QLabel("Task 2: Hough & Contours", railFooter);
    rfVersion->setStyleSheet("font-size: 10px; color: #D4CEC6; font-weight: 400;");

    rfLayout->addWidget(rfLabel);
    rfLayout->addWidget(rfVersion);
    railLayout->addWidget(railFooter);
}

// ── Header Bar ───────────────────────────────────────────────────────────────
void MainWindow::buildHeaderBar() {
    headerBar = new QWidget(this);
    headerBar->setObjectName("headerBar");
    headerBar->setFixedHeight(56);

    QHBoxLayout* hbl = new QHBoxLayout(headerBar);
    hbl->setContentsMargins(20, 0, 20, 0);
    hbl->setSpacing(12);

    QLabel* breadcrumb = new QLabel("Workspace", headerBar);
    breadcrumb->setStyleSheet("font-size: 13px; font-weight: 600; color: #2C2825;");

    QLabel* breadDiv = new QLabel("›", headerBar);
    breadDiv->setStyleSheet("font-size: 14px; color: #C4BDB4;");

    QLabel* taskNameLabel = new QLabel("Canny (Scratch)", headerBar);
    taskNameLabel->setObjectName("activeBreadcrumb");
    taskNameLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #5B4FCF;");

    hbl->addWidget(breadcrumb);
    hbl->addWidget(breadDiv);
    hbl->addWidget(taskNameLabel);
    hbl->addStretch();

    QLabel* shortcutHint = new QLabel("↵ Apply   ⌫ Clear", headerBar);
    shortcutHint->setStyleSheet("font-size: 10px; color: #C4BDB4; letter-spacing: 0.05em;");
    hbl->addWidget(shortcutHint);

    QLabel* accentDot = new QLabel(headerBar);
    accentDot->setFixedSize(8, 8);
    accentDot->setStyleSheet("background-color: #5B4FCF; border-radius: 4px;");
    hbl->addWidget(accentDot);
}

// ── Canvas Area ──────────────────────────────────────────────────────────────
void MainWindow::buildCanvasArea() {
    canvasArea = new QWidget(this);
    canvasArea->setObjectName("canvasArea");

    QHBoxLayout* canvasLayout = new QHBoxLayout(canvasArea);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    contentSplitter = new QSplitter(Qt::Horizontal, canvasArea);
    contentSplitter->setHandleWidth(1);
    contentSplitter->setChildrenCollapsible(false);

    // Left: input panels
    QWidget* inputContainer = new QWidget(canvasArea);
    inputContainer->setStyleSheet("background-color: #F5F3EF;");
    leftSplitter = new QSplitter(Qt::Vertical, inputContainer);
    leftSplitter->setHandleWidth(6);
    leftSplitter->setChildrenCollapsible(false);

    QVBoxLayout* leftContLayout = new QVBoxLayout(inputContainer);
    leftContLayout->setContentsMargins(12, 12, 6, 12);
    leftContLayout->setSpacing(0);
    leftContLayout->addWidget(leftSplitter);

    // Center: thin divider label
    QWidget* dividerWidget = new QWidget(canvasArea);
    dividerWidget->setFixedWidth(1);
    dividerWidget->setStyleSheet("background-color: #E8E2DA;");

    // Right: output panels
    QWidget* outputContainer = new QWidget(canvasArea);
    outputContainer->setStyleSheet("background-color: #F5F3EF;");
    rightSplitter = new QSplitter(Qt::Vertical, outputContainer);
    rightSplitter->setHandleWidth(6);
    rightSplitter->setChildrenCollapsible(false);

    QVBoxLayout* rightContLayout = new QVBoxLayout(outputContainer);
    rightContLayout->setContentsMargins(6, 12, 12, 12);
    rightContLayout->setSpacing(0);
    rightContLayout->addWidget(rightSplitter);

    contentSplitter->addWidget(inputContainer);
    contentSplitter->addWidget(dividerWidget);
    contentSplitter->addWidget(outputContainer);
    contentSplitter->setStretchFactor(0, 2);
    contentSplitter->setStretchFactor(1, 0);
    contentSplitter->setStretchFactor(2, 3);

    // Info sidebar (for Active Contours)
    infoSidebar = new QTextBrowser(canvasArea);
    infoSidebar->setFixedWidth(280);
    infoSidebar->hide();

    canvasLayout->addWidget(contentSplitter, 1);
    canvasLayout->addWidget(infoSidebar);
}

// ── Active Task Highlight ─────────────────────────────────────────────────────
void MainWindow::setActiveTask(int taskIndex) {
    for (int i = 0; i < taskButtons.size(); ++i) {
        bool active = (i == taskIndex - 1);
        taskButtons[i]->setProperty("active", active);
        taskButtons[i]->style()->unpolish(taskButtons[i]);
        taskButtons[i]->style()->polish(taskButtons[i]);
    }

    QLabel* bc = headerBar->findChild<QLabel*>("activeBreadcrumb");
    if (bc && taskIndex >= 1 && taskIndex <= TASKS.size()) {
        bc->setText(TASKS[taskIndex - 1].shortName);
    }
}

// ── Layout Updates ────────────────────────────────────────────────────────────
void MainWindow::updateLayoutForTask(int taskIndex) {
    if (infoSidebar) infoSidebar->hide();

    cv::Mat savedInput;
    if (!inputPanels.isEmpty() && !inputPanels[0]->getImage().empty()) {
        savedInput = inputPanels[0]->getImage().clone();
    }

    // Adapt layout for Task 2 specifications
    if (taskIndex == 1) { // Canny Edge
        rebuildPanels(1, 1, {"Source Image"}, {"Binary Edge Map"});
    } else if (taskIndex == 2) { // Member 2 Task: Hough Lines
        rebuildPanels(1, 1, {"Source Image"}, {"Lines Detection Overlay"});
    } else if (taskIndex == 3 || taskIndex == 4) { // Other Hough transforms
        rebuildPanels(1, 1, {"Source Image"}, {"Detection Overlay"});
    } else if (taskIndex == 5) { // Active contours task triggers the Analytics Sidebar
        rebuildPanels(1, 1, {"Source Image"}, {"Evolved Contour"});
        infoSidebar->show();
        infoSidebar->setHtml(R"(
            <style>
                body { font-family: 'DM Sans', sans-serif; color: #2C2825; margin: 0; padding: 0; }
                .card { background: #FFFFFF; border: 1px solid #E6E0F7; border-radius: 12px; padding: 14px 14px; margin-bottom: 10px; }
                h3 { font-size: 15px; font-weight: 800; color: #2C2825; margin: 0 0 4px; }
                p { font-size: 12px; color: #7A7268; line-height: 1.6; margin: 8px 0 0; }
                .badge { display: inline-block; background: #EDE8FF; color: #5B4FCF; border-radius: 6px; padding: 3px 9px; font-size: 11px; font-weight: 800; }
            </style>
            <div class='card'>
                <h3>Shape Analytics</h3>
                <p>Click <b>Apply</b> to calculate metrics.</p>
            </div>
            <div class='card'>
                <p class='badge'>Perimeter: --</p><br>
                <p class='badge'>Area: --</p>
            </div>
            <div class='card'>
                <h3>Chain Code</h3>
                <p style="word-wrap: break-word;">Waiting for contour...</p>
            </div>
        )");
    }

    if (!savedInput.empty() && !inputPanels.isEmpty()) {
        inputPanels[0]->displayImage(savedInput);
    }
}

void MainWindow::rebuildPanels(int numInputs, int numOutputs,
                                QStringList inTitles, QStringList outTitles) {
    for (ImagePanel* p : inputPanels) {
        p->hide();
        p->setParent(nullptr);
        p->deleteLater();
    }
    inputPanels.clear();

    for (ImagePanel* p : outputPanels) {
        p->hide();
        p->setParent(nullptr);
        p->deleteLater();
    }
    outputPanels.clear();

    for (int i = 0; i < numInputs; ++i) {
        QString title = (i < inTitles.size()) ? inTitles[i] : QString("Input %1").arg(i + 1);
        ImagePanel* panel = new ImagePanel(title, true, leftSplitter);
        leftSplitter->addWidget(panel);
        panel->show();
        inputPanels.append(panel);
    }

    for (int i = 0; i < numOutputs; ++i) {
        QString title = (i < outTitles.size()) ? outTitles[i] : QString("Output %1").arg(i + 1);
        ImagePanel* panel = new ImagePanel(title, false, rightSplitter);
        rightSplitter->addWidget(panel);
        panel->show();
        outputPanels.append(panel);
    }
}

void MainWindow::setStatusMessage(const QString& msg, bool success) {
    taskBar->setStatus(msg, success);
}
