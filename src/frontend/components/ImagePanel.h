#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <opencv2/opencv.hpp>

class ImagePanel : public QWidget {
    Q_OBJECT

public:
    explicit ImagePanel(const QString& title, bool isInput = false, QWidget *parent = nullptr);
    
    cv::Mat getImage() const;
    void displayImage(const cv::Mat& img);
    void clear();

private slots:
    void handleLoad();

private:
    QLabel* imageLabel;
    QLabel* titleLabel;
    QLabel* resolutionLabel;
    cv::Mat currentImage;
    bool isInputPanel;

    QImage matToQImage(const cv::Mat& mat);
    void updateResolutionLabel();
};

#endif // IMAGEPANEL_H