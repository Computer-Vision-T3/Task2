#include "AppController.h"
#include "../MainWindow.h"
#include "../components/TopTaskBar.h"
#include "../components/ImagePanel.h"
#include "../components/ParameterBox.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSlider>
#include <QSpinBox>

// ── BACKEND IMPORTS ──────────────────────────────────────────
#include "../../backend/Module1_CannyEdge/CannyDetector.h"
#include "../../backend/Module2_HoughLines/HoughLines.h"
#include "../../backend/Module3_HoughCircles/HoughCircles.h"
#include "../../backend/Module4_HoughEllipses/HoughEllipses.h" 
#include "../../backend/Module5_ActiveContours/GreedySnake.h"
#include "../../backend/Module5_ActiveContours/ShapeAnalytics.h"

AppController::AppController(MainWindow* window, QObject *parent)
    : QObject(parent), mainWindow(window) {
    
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::taskChanged,   this, &AppController::handleTaskChange);
    
    // Normal Apply button (passes -1 automatically)
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::applyRequested, this, [this](){ this->handleApply(); });
    
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::clearRequested, this, &AppController::handleClear);
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::saveRequested,  this, &AppController::handleSave);

    // Connect the "+ Add" buttons to the apply logic
    connect(mainWindow->getTopTaskBar()->getParameterBox(), &ParameterBox::quickActionRequested, 
            this, &AppController::handleApply);
}

void AppController::handleTaskChange(int taskIndex) {
    mainWindow->updateLayoutForTask(taskIndex);
}

// ... (existing includes)

void AppController::handleApply(int taskIndexOverride) {
    int taskIndex = (taskIndexOverride != -1) ? taskIndexOverride : mainWindow->getTopTaskBar()->getSelectedOperation();
    
    auto& inputs  = mainWindow->getInputPanels();
    auto& outputs = mainWindow->getOutputPanels();

    if (inputs.isEmpty() || inputs[0]->getImage().empty()) {
        mainWindow->setStatusMessage("Load an image first!", false);
        return;
    }

    mainWindow->getTopTaskBar()->setProcessing(true);
    cv::Mat originalImg = inputs[0]->getImage();
    
    // Determine the background canvas
    cv::Mat currentCanvas;
    if (!outputs.isEmpty() && !outputs[0]->getImage().empty()) {
        currentCanvas = outputs[0]->getImage().clone();
    } else {
        currentCanvas = originalImg.clone();
    }

    ParameterBox* pBox = mainWindow->getTopTaskBar()->getParameterBox();
    cv::Mat result;

    try {
        // --- MODULE 1: CANNY ---
        if (taskIndex == 1) {
            int low = 50, high = 150;
            if (auto* s = pBox->findChild<QSpinBox*>("cannyLow")) low = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("cannyHigh")) high = s->value();
            result = CannyDetector::apply(originalImg, low, high);
        }
        // --- MODULE 2: HOUGH LINES ---
        else if (taskIndex == 2) {
            int thresh = 100; // Default
            // Only read from UI if the widget actually exists (safe check)
            if (auto* s = pBox->findChild<QSpinBox*>("houghLineThresh")) thresh = s->value();
            result = HoughLines::detect(originalImg, currentCanvas, thresh);
        }
        // --- MODULE 3: HOUGH CIRCLES ---
        else if (taskIndex == 3) {
            int minR = 10, maxR = 80, thresh = 30; // Defaults
            if (auto* s = pBox->findChild<QSpinBox*>("circleMinR")) minR = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("circleMaxR")) maxR = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("circleThresh")) thresh = s->value();
            result = HoughCircles::detect(originalImg, currentCanvas, minR, maxR, thresh);
        }
        // --- MODULE 4: HOUGH ELLIPSES ---
        else if (taskIndex == 4) {
            int minMaj = 20, votes = 8; // Defaults
            if (auto* s = pBox->findChild<QSpinBox*>("ellipMinMaj")) minMaj = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("ellipVotes")) votes = s->value();
            // Call the ellipse detect function
            result = HoughEllipses::detect(originalImg, currentCanvas, minMaj, 0, votes);
        }
        // --- MODULE 5: ACTIVE CONTOURS ---
        else if (taskIndex == 5) {
            float alpha = 1.0f, beta = 1.0f, gamma = 1.0f;
            int iter = 100;

            if (auto* s = pBox->findChild<QSlider*>("snakeAlpha")) alpha = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSlider*>("snakeBeta"))  beta  = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSlider*>("snakeGamma")) gamma = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSpinBox*>("snakeIter")) iter  = s->value();
            
            result = currentCanvas.clone();
            
            // Grab the user's manual points from the UI
            std::vector<cv::Point> initialPoints = inputs[0]->getClickedPoints();

            // Evolve the snake and generate the HTML report
            std::vector<cv::Point> finalContour = GreedySnake::evolve(result, alpha, beta, gamma, iter, initialPoints);
            QString analyticsHtml = ShapeAnalytics::generateReport(finalContour);

            // Inject the calculated area/perimeter back into the UI sidebar
            if (mainWindow->getInfoSidebar()) {
                mainWindow->getInfoSidebar()->setHtml(analyticsHtml);
            }
        }
        if (!result.empty() && !outputs.isEmpty()) {
            outputs[0]->displayImage(result);
            mainWindow->setStatusMessage("Applied successfully ✓", true);
        }

    } catch (const std::exception& e) {
        mainWindow->setStatusMessage("Error in logic", false);
    }

    mainWindow->getTopTaskBar()->setProcessing(false);
}
void AppController::handleClear() {
    for (auto* panel : mainWindow->getOutputPanels()) panel->clear();
    mainWindow->setStatusMessage("Outputs Cleared", true);
}

void AppController::handleSave() {
    auto& outputs = mainWindow->getOutputPanels();
    if (outputs.isEmpty() || outputs[0]->getImage().empty()) return;
    QString fileName = QFileDialog::getSaveFileName(mainWindow, "Save Result", "", "PNG (*.png);;JPG (*.jpg)");
    if (!fileName.isEmpty()) {
        cv::imwrite(fileName.toStdString(), outputs[0]->getImage());
        mainWindow->setStatusMessage("Saved ✓", true);
    }
}
