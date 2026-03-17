#pragma once
#include <opencv2/opencv.hpp>

class DrawingUtils {
public:
    // Standard geometric drawing with built-in Anti-Aliasing (smooth edges)
    static void drawCircle(cv::Mat& image, cv::Point center, int radius, cv::Scalar color = cv::Scalar(0, 255, 0), int thickness = 2);
    static void drawEllipse(cv::Mat& image, cv::Point center, cv::Size axes, double angle, cv::Scalar color = cv::Scalar(255, 0, 255), int thickness = 2);
    
    // Crucial helper for Member 2: Converts Hough (Rho, Theta) to Cartesian (X, Y) and draws an infinite line
    static void drawPolarLine(cv::Mat& image, float rho, float theta, cv::Scalar color = cv::Scalar(0, 0, 255), int thickness = 2);
};