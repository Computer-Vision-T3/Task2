#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class GreedySnake {
public:
    // The main entry point. If initialContour is empty, it smartly defaults to a large circle.
    static std::vector<cv::Point> evolve(cv::Mat& image, float alpha, float beta, float gamma, int iterations, const std::vector<cv::Point>& initialContour = {});

private:
    static std::vector<cv::Point> generateInitialCircle(int width, int height);
    static cv::Mat computeExternalEnergy(const cv::Mat& image);
    static float getContinuityEnergy(cv::Point curr, cv::Point prev, float avgDistance);
    static float getCurvatureEnergy(cv::Point prev, cv::Point curr, cv::Point next);
    static float calculateAverageDistance(const std::vector<cv::Point>& contour);
};