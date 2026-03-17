#include "MathUtils.h"
#include <cmath>

double MathUtils::euclideanDistance(cv::Point p1, cv::Point p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

double MathUtils::euclideanDistance(double x1, double y1, double x2, double y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

double MathUtils::degreesToRadians(double degrees) {
    return degrees * (CV_PI / 180.0);
}

double MathUtils::radiansToDegrees(double radians) {
    return radians * (180.0 / CV_PI);
}

int MathUtils::clamp(int value, int minVal, int maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}