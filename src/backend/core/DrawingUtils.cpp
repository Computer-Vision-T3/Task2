#include "DrawingUtils.h"
#include <cmath>

void DrawingUtils::drawCircle(cv::Mat& image, cv::Point center, int radius, cv::Scalar color, int thickness) {
    cv::circle(image, center, radius, color, thickness, cv::LINE_AA);
}

void DrawingUtils::drawEllipse(cv::Mat& image, cv::Point center, cv::Size axes, double angle, cv::Scalar color, int thickness) {
    cv::ellipse(image, center, axes, angle, 0.0, 360.0, color, thickness, cv::LINE_AA);
}

void DrawingUtils::drawPolarLine(cv::Mat& image, float rho, float theta, cv::Scalar color, int thickness) {
    // Convert polar coordinates to Cartesian space
    double a = std::cos(theta);
    double b = std::sin(theta);
    
    // Find the point where the line is closest to the origin
    double x0 = a * rho;
    double y0 = b * rho;
    
    // Project the line far out in both directions (10,000 pixels guarantees it crosses the whole screen)
    cv::Point pt1(cvRound(x0 + 10000 * (-b)), cvRound(y0 + 10000 * (a)));
    cv::Point pt2(cvRound(x0 - 10000 * (-b)), cvRound(y0 - 10000 * (a)));
    
    // Draw the anti-aliased line
    cv::line(image, pt1, pt2, color, thickness, cv::LINE_AA);
}