#include "TopTaskBar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

TopTaskBar::TopTaskBar(QWidget *parent) : QWidget(parent) {
    setObjectName("paramStrip");
    
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(24);

    // --- Left Section: Operations Dropdown ---
    QWidget* opWidget = new QWidget(this);
    QVBoxLayout* opLayout = new QVBoxLayout(opWidget);
    opLayout->setContentsMargins(0, 0, 0, 0);
    opLayout->setSpacing(6);

    QLabel* opLabel = new QLabel("Active Module", opWidget);
    opLabel->setObjectName("paramLabel");

    opSelector = new QComboBox(opWidget);
    opSelector->setObjectName("operationSelector");
    opSelector->setMinimumWidth(220);
    // These match the Task Meta defined in MainWindow.cpp
    opSelector->addItems({
        "1. Canny Edge (Scratch)",
        "2. Hough Lines",
        "3. Hough Circles",
        "4. Hough Ellipses",
        "5. Active Contours (Snake)"
    });

    opLayout->addWidget(opLabel);
    opLayout->addWidget(opSelector);
    opLayout->addStretch();
    
    mainLayout->addWidget(opWidget);

    // --- Middle Section: Dynamic Parameters ---
    paramBox = new ParameterBox(this);
    mainLayout->addWidget(paramBox, 1); // Takes up expanding space

    // --- Right Section: Action Buttons & Status ---
    QWidget* actionWidget = new QWidget(this);
    QVBoxLayout* actionLayout = new QVBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(8);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    saveBtn = new QPushButton("Save", actionWidget);
    saveBtn->setObjectName("ghostBtn");
    
    clearBtn = new QPushButton("Clear", actionWidget);
    clearBtn->setObjectName("secondaryBtn");
    
    applyBtn = new QPushButton("Apply Filter", actionWidget);
    applyBtn->setObjectName("primaryBtn");

    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(applyBtn);

    statusLabel = new QLabel("Ready", actionWidget);
    statusLabel->setObjectName("statusIndicator");
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    actionLayout->addLayout(btnLayout);
    actionLayout->addWidget(statusLabel);
    
    mainLayout->addWidget(actionWidget);

    // --- Signal Connections ---
    connect(opSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TopTaskBar::onSelectionChanged);
    connect(applyBtn, &QPushButton::clicked, this, &TopTaskBar::applyRequested);
    connect(clearBtn, &QPushButton::clicked, this, &TopTaskBar::clearRequested);
    connect(saveBtn, &QPushButton::clicked, this, &TopTaskBar::saveRequested);

    // Initialize first task params
    onSelectionChanged(0);
}

int TopTaskBar::getSelectedOperation() const {
    // Return 1-based index (1 = Canny, 2 = Hough Lines, etc.)
    return opSelector->currentIndex() + 1;
}

void TopTaskBar::onSelectionChanged(int index) {
    paramBox->updateForTask(index + 1);
    emit taskChanged(index + 1);
    setStatus("Ready", true);
}

void TopTaskBar::setProcessing(bool isProcessing) {
    applyBtn->setEnabled(!isProcessing);
    clearBtn->setEnabled(!isProcessing);
    saveBtn->setEnabled(!isProcessing);
    opSelector->setEnabled(!isProcessing);
    paramBox->setEnabled(!isProcessing);

    if (isProcessing) {
        applyBtn->setText("Processing...");
        setStatus("Computing...", true);
        statusLabel->setStyleSheet("color: #5B4FCF; background-color: #EDE8FF;");
    } else {
        applyBtn->setText("Apply Filter");
    }
}

void TopTaskBar::setStatus(const QString& message, bool success) {
    statusLabel->setText(message);
    if (success) {
        if (message == "Ready") {
            statusLabel->setStyleSheet("color: #7A7268; background-color: transparent;");
        } else {
            // Green success
            statusLabel->setStyleSheet("color: #2D9B6F; background-color: #E8F5F0;");
        }
    } else {
        // Red error
        statusLabel->setStyleSheet("color: #D32F2F; background-color: #FFEBEE;");
    }
}