#include "ParameterBox.h"
#include <QVBoxLayout>

ParameterBox::ParameterBox(QWidget *parent) : QWidget(parent) {
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
}

void ParameterBox::clearLayout() {
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

void ParameterBox::updateForTask(int taskIndex) {
    clearLayout();

    // --- MODULE 1: CANNY EDGE ---
    if (taskIndex == 1) {
        addSpinBox("cannyLow", "Low Threshold", 0, 255, 50, 0, 0);
        addSpinBox("cannyHigh", "High Threshold", 0, 255, 150, 0, 1);
    }
    // --- MODULE 2: HOUGH LINES ---
    else if (taskIndex == 2) {
        addSpinBox("houghLineThresh", "Voting Threshold", 1, 500, 100, 0, 0);
    }
    // --- MODULE 3: HOUGH CIRCLES ---
    else if (taskIndex == 3) {
        addSpinBox("circleMinR", "Min Radius", 1, 500, 10, 0, 0);
        addSpinBox("circleMaxR", "Max Radius", 1, 500, 50, 0, 1);
        addSpinBox("circleThresh", "Voting Threshold", 1, 500, 100, 0, 2);
    }
    // --- MODULE 4: HOUGH ELLIPSES ---
    else if (taskIndex == 4) {
        QLabel* info = new QLabel("Advanced Transform (Auto-Optimized Parameters)", this);
        info->setStyleSheet("color: #A09890; font-style: italic;");
        layout->addWidget(info, 0, 0);
    }
    // --- MODULE 5: ACTIVE CONTOURS ---
    else if (taskIndex == 5) {
        addSlider("snakeAlpha", "Alpha (Elasticity)", 0, 50, 10, 0, 0, true); // 10 = 1.0
        addSlider("snakeBeta", "Beta (Curvature)", 0, 50, 10, 0, 1, true);
        addSlider("snakeGamma", "Gamma (Image Energy)", 0, 50, 10, 0, 2, true);
        addSpinBox("snakeIter", "Iterations", 1, 5000, 100, 0, 3);
    }
    else {
        QLabel* info = new QLabel("No parameters required.", this);
        info->setStyleSheet("color: #C4BDB4; font-style: italic;");
        layout->addWidget(info, 0, 0);
    }
}

void ParameterBox::addSpinBox(const QString& id, const QString& labelText, int min, int max, int defaultVal, int row, int col) {
    QWidget* container = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(0,0,0,0);
    vbox->setSpacing(4);

    QLabel* label = new QLabel(labelText, container);
    label->setObjectName("paramLabel");

    QSpinBox* spinBox = new QSpinBox(container);
    spinBox->setObjectName(id);
    spinBox->setRange(min, max);
    spinBox->setValue(defaultVal);

    vbox->addWidget(label);
    vbox->addWidget(spinBox);
    layout->addWidget(container, row, col);
}

void ParameterBox::addSlider(const QString& id, const QString& labelText, int min, int max, int defaultVal, int row, int col, bool isFloat) {
    QWidget* container = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(0,0,0,0);
    vbox->setSpacing(4);

    QWidget* labelRow = new QWidget(container);
    QHBoxLayout* hbox = new QHBoxLayout(labelRow);
    hbox->setContentsMargins(0,0,0,0);

    QLabel* label = new QLabel(labelText, labelRow);
    label->setObjectName("paramLabel");

    QLabel* valLabel = new QLabel(isFloat ? QString::number(defaultVal / 10.0, 'f', 1) : QString::number(defaultVal), labelRow);
    valLabel->setObjectName("valueLabel");
    valLabel->setAlignment(Qt::AlignRight);

    hbox->addWidget(label);
    hbox->addWidget(valLabel);

    QSlider* slider = new QSlider(Qt::Horizontal, container);
    slider->setObjectName(id);
    slider->setRange(min, max);
    slider->setValue(defaultVal);

    connect(slider, &QSlider::valueChanged, [valLabel, isFloat](int val){
        valLabel->setText(isFloat ? QString::number(val / 10.0, 'f', 1) : QString::number(val));
    });

    vbox->addWidget(labelRow);
    vbox->addWidget(slider);
    layout->addWidget(container, row, col);
}