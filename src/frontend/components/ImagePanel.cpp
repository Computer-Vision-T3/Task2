#include "ImagePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

ImagePanel::ImagePanel(const QString& title, bool isInput, QWidget *parent)
    : QWidget(parent), isInputPanel(isInput) {
    
    setObjectName("imagePanel");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // --- Header ---
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setObjectName("imagePanelHeader");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 8);

    titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("panelTitle");
    
    QLabel* badge = new QLabel(isInput ? "INPUT" : "OUTPUT", this);
    badge->setObjectName(isInput ? "panelBadgeInput" : "panelBadge");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(badge);
    
    resolutionLabel = new QLabel("", this);
    resolutionLabel->setStyleSheet("color: #C4BDB4; font-size: 10px; font-weight: 600;");
    headerLayout->addStretch();
    headerLayout->addWidget(resolutionLabel);

    if (isInput) {
        QPushButton* loadBtn = new QPushButton("Load Image", this);
        loadBtn->setObjectName("loadBtn");
        loadBtn->setCursor(Qt::PointingHandCursor);
        connect(loadBtn, &QPushButton::clicked, this, &ImagePanel::handleLoad);
        headerLayout->addWidget(loadBtn);
    }

    // --- Image Display Area ---
    imageLabel = new QLabel(isInput ? "No Image Loaded" : "Waiting for processing...", this);
    imageLabel->setObjectName("imageDisplay");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(300, 300);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setSizePolicy(sizePolicy);

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(imageLabel, 1);
}

cv::Mat ImagePanel::getImage() const {
    return currentImage.clone();
}

void ImagePanel::displayImage(const cv::Mat& img) {
    if (img.empty()) {
        clear();
        return;
    }
    
    currentImage = img.clone();
    QImage qimg = matToQImage(currentImage);
    
    // Smoothly scale the image to fit the panel without stretching
    QPixmap pixmap = QPixmap::fromImage(qimg).scaled(
        imageLabel->size(), 
        Qt::KeepAspectRatio, 
        Qt::SmoothTransformation
    );
    
    imageLabel->setPixmap(pixmap);
    updateResolutionLabel();
}

void ImagePanel::clear() {
    currentImage.release();
    imageLabel->clear();
    imageLabel->setText(isInputPanel ? "No Image Loaded" : "Waiting for processing...");
    resolutionLabel->setText("");
}

void ImagePanel::handleLoad() {
    QString filter = "Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)";
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", filter);
    
    if (!fileName.isEmpty()) {
        cv::Mat img = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
        if (img.empty()) {
            QMessageBox::warning(this, "Error", "Failed to open image.");
            return;
        }
        displayImage(img);
    }
}

QImage ImagePanel::matToQImage(const cv::Mat& mat) {
    if (mat.type() == CV_8UC1) {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    } else if (mat.type() == CV_8UC3) {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_BGR888);
        return image.copy();
    } else if (mat.type() == CV_8UC4) {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        return image.copy();
    }
    return QImage();
}

void ImagePanel::updateResolutionLabel() {
    if (!currentImage.empty()) {
        resolutionLabel->setText(QString("%1 × %2 px").arg(currentImage.cols).arg(currentImage.rows));
    } else {
        resolutionLabel->setText("");
    }
}