#ifndef CANNY_DETECTOR_H
#define CANNY_DETECTOR_H

#include <opencv2/opencv.hpp>

class CannyDetector {
public:
    cv::Mat sobel(const cv::Mat& image, int kernel_size = 3);
    cv::Mat roberts(const cv::Mat& image);
    cv::Mat prewitt(const cv::Mat& image);
    cv::Mat canny(const cv::Mat& image, int low, int high);

    cv::Mat cannyHandmade(const cv::Mat& image, int low, int high);

private:
    void sobelGradients(const cv::Mat& image, cv::Mat& magnitude, cv::Mat& angle);
    cv::Mat nonMaxSuppression(const cv::Mat& mag, const cv::Mat& angle);
    cv::Mat hysteresis(const cv::Mat& img, int low, int high);

    cv::Mat normalize(const cv::Mat& img);
};

#endif