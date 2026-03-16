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
#include <QComboBox>

// ── TASK 2 BACKEND IMPORTS ────────────────────────────────────
// INSTRUCTION: Uncomment these lines ONLY when your team actually 
// creates the files in their backend folders.

// #include "../../backend/Module1_CannyEdge/CannyDetector.h"
// #include "../../backend/Module2_HoughLines/HoughLines.h"
// #include "../../backend/Module3_HoughCircles/HoughCircles.h"
// #include "../../backend/Module4_HoughEllipses/HoughEllipses.h"
// #include "../../backend/Module5_ActiveContours/GreedySnake.h"
// #include "../../backend/Module5_ActiveContours/ShapeAnalytics.h"

AppController::AppController(MainWindow* window, QObject *parent)
    : QObject(parent), mainWindow(window) {
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::taskChanged,   this, &AppController::handleTaskChange);
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::applyRequested, this, &AppController::handleApply);
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::clearRequested, this, &AppController::handleClear);
    connect(mainWindow->getTopTaskBar(), &TopTaskBar::saveRequested,  this, &AppController::handleSave);
}

void AppController::handleTaskChange(int taskIndex) {
    mainWindow->updateLayoutForTask(taskIndex);
}

void AppController::handleApply() {
    int taskIndex = mainWindow->getTopTaskBar()->getSelectedOperation();
    auto& inputs  = mainWindow->getInputPanels();
    auto& outputs = mainWindow->getOutputPanels();

    if (inputs.isEmpty() || inputs[0]->getImage().empty()) {
        mainWindow->setStatusMessage("No image loaded!", false);
        return;
    }

    mainWindow->getTopTaskBar()->setProcessing(true);
    cv::Mat currentImg = inputs[0]->getImage();
    ParameterBox* pBox = mainWindow->getTopTaskBar()->getParameterBox();

    try {
        // ── MODULE 1: CANNY EDGE DETECTOR (FROM SCRATCH) ─────────
        if (taskIndex == 1) {
            int lowT = 50, highT = 150;
            if (auto* s = pBox->findChild<QSpinBox*>("cannyLow")) lowT = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("cannyHigh")) highT = s->value();

            cv::Mat result;

            // --- WHEN READY, UNCOMMENT THIS: ---
            // result = CannyDetector::apply(currentImg, lowT, highT);
            
            // --- PLACEHOLDER (REMOVE WHEN READY): ---
            result = currentImg.clone();
            QMessageBox::information(mainWindow, "Module 1", "The Canny Edge Detector is currently under construction by Member 1.");
            // ----------------------------------------

            if (!outputs.isEmpty()) outputs[0]->displayImage(result);
        }

        // ── MODULE 2: HOUGH LINES ────────────────────────────────
        else if (taskIndex == 2) {
            int thresh = 100;
            if (auto* s = pBox->findChild<QSpinBox*>("houghLineThresh")) thresh = s->value();

            cv::Mat result;

            // --- WHEN READY, UNCOMMENT THIS: ---
            // result = HoughLines::detect(currentImg, thresh);

            // --- PLACEHOLDER (REMOVE WHEN READY): ---
            result = currentImg.clone();
            QMessageBox::information(mainWindow, "Module 2", "Hough Lines is currently under construction by Member 2.");
            // ----------------------------------------

            if (!outputs.isEmpty()) outputs[0]->displayImage(result);
        }

        // ── MODULE 3: HOUGH CIRCLES ──────────────────────────────
        else if (taskIndex == 3) {
            int minR = 10, maxR = 50, thresh = 100;
            if (auto* s = pBox->findChild<QSpinBox*>("circleMinR")) minR = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("circleMaxR")) maxR = s->value();
            if (auto* s = pBox->findChild<QSpinBox*>("circleThresh")) thresh = s->value();

            cv::Mat result;

            // --- WHEN READY, UNCOMMENT THIS: ---
            // result = HoughCircles::detect(currentImg, minR, maxR, thresh);

            // --- PLACEHOLDER (REMOVE WHEN READY): ---
            result = currentImg.clone();
            QMessageBox::information(mainWindow, "Module 3", "Hough Circles is currently under construction by Member 3.");
            // ----------------------------------------

            if (!outputs.isEmpty()) outputs[0]->displayImage(result);
        }

        // ── MODULE 4: HOUGH ELLIPSES ─────────────────────────────
        else if (taskIndex == 4) {
            cv::Mat result;

            // --- WHEN READY, UNCOMMENT THIS: ---
            // result = HoughEllipses::detect(currentImg);

            // --- PLACEHOLDER (REMOVE WHEN READY): ---
            result = currentImg.clone();
            QMessageBox::information(mainWindow, "Module 4", "Hough Ellipses is currently under construction by Member 4.");
            // ----------------------------------------

            if (!outputs.isEmpty()) outputs[0]->displayImage(result);
        }

        // ── MODULE 5: ACTIVE CONTOURS & ANALYTICS ────────────────
        else if (taskIndex == 5) {
            float alpha = 1.0f, beta = 1.0f, gamma = 1.0f;
            int iter = 100;

            if (auto* s = pBox->findChild<QSlider*>("snakeAlpha")) alpha = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSlider*>("snakeBeta"))  beta  = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSlider*>("snakeGamma")) gamma = s->value() / 10.0f;
            if (auto* s = pBox->findChild<QSpinBox*>("snakeIter")) iter  = s->value();
            
            cv::Mat resultImg = currentImg.clone();
            QString analyticsHtml;

            // --- WHEN READY, UNCOMMENT THIS: ---
            // std::vector<cv::Point> finalContour = GreedySnake::evolve(resultImg, alpha, beta, gamma, iter);
            // analyticsHtml = ShapeAnalytics::generateReport(finalContour);
            
            // --- PLACEHOLDER (REMOVE WHEN READY): ---
            QMessageBox::information(mainWindow, "Module 5", "Active Contours (Snakes) is currently under construction by Member 5.");
            analyticsHtml = R"(
                <div style='background:#FFFFFF; border:1px solid #E6E0F7; border-radius:12px; padding:14px; font-family:sans-serif;'>
                    <h3 style='color:#2C2825; margin:0 0 4px;'>Analytics (Placeholder)</h3>
                    <p style='color:#7A7268; font-size:12px;'>Waiting for Member 5's backend logic to populate this data...</p>
                </div>
            )";
            // ----------------------------------------

            if (!outputs.isEmpty()) outputs[0]->displayImage(resultImg);
            if (mainWindow->getInfoSidebar()) {
                mainWindow->getInfoSidebar()->setHtml(analyticsHtml);
            }
        }
        
        mainWindow->setStatusMessage("Done ✓", true);

    } catch (const std::exception& e) {
        mainWindow->setStatusMessage("Processing Failed", false);
        QMessageBox::critical(mainWindow, "Backend Error", 
            QString("An error occurred in the image processing module:\n\n%1").arg(e.what()));
    } catch (...) {
        mainWindow->setStatusMessage("Processing Failed", false);
    }

    mainWindow->getTopTaskBar()->setProcessing(false);
}

void AppController::handleClear() {
    for (auto* panel : mainWindow->getOutputPanels()) {
        panel->clear();
    }
    mainWindow->setStatusMessage("Outputs Cleared", true);
}

void AppController::handleSave() {
    auto& outputs = mainWindow->getOutputPanels();
    if (outputs.isEmpty() || outputs[0]->getImage().empty()) {
        mainWindow->setStatusMessage("Nothing to save", false);
        return;
    }

    QString filter = "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;BMP Image (*.bmp)";
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        mainWindow, "Save Processed Image", "", filter, &selectedFilter
    );

    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).suffix().isEmpty()) {
            if (selectedFilter.contains("*.jpg") || selectedFilter.contains("*.jpeg")) {
                fileName += ".jpg";
            } else if (selectedFilter.contains("*.bmp")) {
                fileName += ".bmp";
            } else {
                fileName += ".png";
            }
        }

        try {
            bool ok = cv::imwrite(fileName.toStdString(), outputs[0]->getImage());
            if (ok) {
                mainWindow->setStatusMessage("Saved ✓", true);
            } else {
                mainWindow->setStatusMessage("Save failed", false);
                QMessageBox::critical(mainWindow, "Save Error", "Could not write the file. Check your file path and permissions.");
            }
        } catch (const cv::Exception& e) {
            mainWindow->setStatusMessage("Save failed", false);
            QMessageBox::critical(mainWindow, "Save Error", QString("OpenCV encountered an error while saving:\n%1").arg(e.what()));
        }
    }
}