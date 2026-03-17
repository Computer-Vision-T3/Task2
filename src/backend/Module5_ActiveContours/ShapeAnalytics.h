#pragma once
#include <opencv2/opencv.hpp>
#include <QString>
#include <vector>

class ShapeAnalytics {
public:
    // Generates the final HTML string injected straight into the UI Sidebar
    static QString generateReport(const std::vector<cv::Point>& contour);

private:
    static double calculateArea(const std::vector<cv::Point>& contour);
    static double calculatePerimeter(const std::vector<cv::Point>& contour);
    static QString computeChainCode(const std::vector<cv::Point>& contour);
};