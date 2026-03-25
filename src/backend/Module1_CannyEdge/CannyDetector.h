#ifndef CANNY_DETECTOR_H
#define CANNY_DETECTOR_H

#include <opencv2/opencv.hpp>

class CannyDetector {
public:
    // This is the main function expected by your project architecture
    static cv::Mat apply(cv::Mat img, int lowThresh, int highThresh);

    static cv::Mat sobel(const cv::Mat& image, int kernel_size = 3);
    static cv::Mat roberts(const cv::Mat& image);
    static cv::Mat prewitt(const cv::Mat& image);
    static cv::Mat canny(const cv::Mat& image, int low, int high);
    static cv::Mat cannyHandmade(const cv::Mat& image, int low, int high);

private:
    static cv::Mat applyGaussianBlur(const cv::Mat& input);
    static void calculateGradients(const cv::Mat& input, cv::Mat& magnitude, cv::Mat& angle);
    static cv::Mat nonMaximumSuppression(const cv::Mat& magnitude, const cv::Mat& angle);
    static cv::Mat hysteresis(const cv::Mat& img, int low, int high);
};

#endif
