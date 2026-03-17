#include "ImagePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>

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
    
    // Scale the image to fit the panel without stretching
    QPixmap pixmap = QPixmap::fromImage(qimg).scaled(
        imageLabel->size(), 
        Qt::KeepAspectRatio, 
        Qt::SmoothTransformation
    );
    
    // If the user has clicked points, draw them onto the scaled Pixmap
    if (!clickedPoints.empty() && isInputPanel) {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(QColor(0, 120, 255))); 
        painter.setPen(QPen(QColor(255, 255, 255), 2)); 
        
        int pw = pixmap.width();
        int ph = pixmap.height();
        double scaleX = (double)pw / currentImage.cols;
        double scaleY = (double)ph / currentImage.rows;
        
        int dotSize = std::max(4, currentImage.cols / 150); 

        for (const auto& pt : clickedPoints) {
            int uiX = pt.x * scaleX;
            int uiY = pt.y * scaleY;
            painter.drawEllipse(QPoint(uiX, uiY), dotSize, dotSize);
        }
        painter.end();
    }
    
    imageLabel->setPixmap(pixmap);
    updateResolutionLabel();
}

void ImagePanel::clear() {
    clickedPoints.clear();
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
        clickedPoints.clear(); // Wipe points if a new image is loaded
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

void ImagePanel::mousePressEvent(QMouseEvent *event) {
    // Only capture clicks on the Input panel when an image exists
    if (!isInputPanel || currentImage.empty()) return;

    // Map click from the panel's coordinates to the imageLabel's coordinates
    QPoint pos = imageLabel->mapFrom(this, event->pos());

    QPixmap pixmap = imageLabel->pixmap(Qt::ReturnByValue);
    if (pixmap.isNull()) return;

    int pw = pixmap.width();
    int ph = pixmap.height();
    int lw = imageLabel->width();
    int lh = imageLabel->height();

    // Calculate the black/blank padding generated by KeepAspectRatio
    int offsetX = (lw - pw) / 2;
    int offsetY = (lh - ph) / 2;

    // Ignore clicks outside the actual image area
    if (pos.x() < offsetX || pos.x() >= offsetX + pw ||
        pos.y() < offsetY || pos.y() >= offsetY + ph) {
        return; 
    }

    // Convert UI display coordinates back to original image matrix coordinates
    double scaleX = (double)currentImage.cols / pw;
    double scaleY = (double)currentImage.rows / ph;

    int imgX = (pos.x() - offsetX) * scaleX;
    int imgY = (pos.y() - offsetY) * scaleY;

    // Save the point
    clickedPoints.push_back(cv::Point(imgX, imgY));

    // Force UI refresh to draw the newly added point
    displayImage(currentImage);
}