#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class MathUtils {
public:
    // Calculates the straight-line distance between two points
    static double euclideanDistance(cv::Point p1, cv::Point p2);
    static double euclideanDistance(double x1, double y1, double x2, double y2);

    // Angle conversions (Crucial for Hough Transforms)
    static double degreesToRadians(double degrees);
    static double radiansToDegrees(double radians);

    // Keeps a value strictly within a min/max boundary (prevents array out-of-bounds)
    static int clamp(int value, int minVal, int maxVal);
};