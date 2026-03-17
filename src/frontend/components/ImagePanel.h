#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <vector>

class ImagePanel : public QWidget {
    Q_OBJECT

public:
    explicit ImagePanel(const QString& title, bool isInput = false, QWidget *parent = nullptr);
    
    cv::Mat getImage() const;
    void displayImage(const cv::Mat& img);
    void clear();

    // Fetches the anchor points drawn by the user's mouse clicks
    std::vector<cv::Point> getClickedPoints() const { return clickedPoints; }

protected:
    // Captures mouse clicks on the widget
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void handleLoad();

private:
    QLabel* imageLabel;
    QLabel* titleLabel;
    QLabel* resolutionLabel;
    cv::Mat currentImage;
    bool isInputPanel;
    
    // Stores the translated image coordinates from user clicks
    std::vector<cv::Point> clickedPoints;

    QImage matToQImage(const cv::Mat& mat);
    void updateResolutionLabel();
};

#endif // IMAGEPANEL_H