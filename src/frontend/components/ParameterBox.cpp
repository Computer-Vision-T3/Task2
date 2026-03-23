#include "ParameterBox.h"
#include <QVBoxLayout>
#include <QPushButton>

ParameterBox::ParameterBox(QWidget *parent) : QWidget(parent) {
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
}

void ParameterBox::clearLayout() {
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
}

void ParameterBox::updateForTask(int taskIndex) {
    clearLayout();

    if (taskIndex == 1) { // CANNY
        addSpinBox("cannyLow", "Low Threshold", 0, 255, 50, 0, 0);
        addSpinBox("cannyHigh", "High Threshold", 0, 255, 150, 0, 1);
    }
    else if (taskIndex == 2) { // HOUGH LINES
        addSpinBox("houghLineThresh", "Voting Threshold", 1, 500, 100, 0, 0);
        // Quick Action Buttons
        addButton("+ Add Circles", 3, 0, 1);
        addButton("+ Add Ellipses", 4, 0, 2);
    }
    else if (taskIndex == 3) { // HOUGH CIRCLES
        addSpinBox("circleMinR", "Min Radius", 1, 500, 10, 0, 0);
        addSpinBox("circleMaxR", "Max Radius", 1, 500, 80, 0, 1); // Increased default
        addSpinBox("circleThresh", "Voting Threshold", 1, 500, 30, 0, 2); // Lowered default
        // Quick Action Buttons
        addButton("+ Add Lines", 2, 0, 3);
        addButton("+ Add Ellipses", 4, 0, 4);
    }
    else if (taskIndex == 4) { // HOUGH ELLIPSES
        addSpinBox("ellipMinMaj", "Min Major Axis", 1, 500, 20, 0, 0);
        addSpinBox("ellipVotes", "Min Votes", 1, 500, 8, 0, 1);
        addButton("+ Add Lines", 2, 0, 2);
        addButton("+ Add Circles", 3, 0, 3);
    }
    else if (taskIndex == 5) {
        addSlider("snakeAlpha", "Alpha (Elasticity)", 0, 50, 10, 0, 0, true);
        addSlider("snakeBeta", "Beta (Curvature)", 0, 50, 10, 0, 1, true);
        addSlider("snakeGamma", "Gamma (Image Energy)", 0, 50, 10, 0, 2, true);
        addSpinBox("snakeIter", "Iterations", 1, 5000, 100, 0, 3);
    }
}

void ParameterBox::addButton(const QString& text, int targetTask, int row, int col) {
    QPushButton* btn = new QPushButton(text, this);
    btn->setObjectName("loadBtn"); // Using your themed styling
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedWidth(130);
    
    // When clicked, emit the signal with the target task index
    connect(btn, &QPushButton::clicked, [this, targetTask]() {
        emit quickActionRequested(targetTask);
    });
    
    layout->addWidget(btn, row, col, Qt::AlignBottom);
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
