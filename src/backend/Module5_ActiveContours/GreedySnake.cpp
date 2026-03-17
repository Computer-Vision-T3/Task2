#include "GreedySnake.h"
#include <cmath>
#include <algorithm>

// A helper struct to store neighborhood data during the normalization phase
struct NeighborPoint {
    cv::Point pt;
    float e_cont;
    float e_curv;
    float e_ext;
};

std::vector<cv::Point> GreedySnake::evolve(cv::Mat& image, float alpha, float beta, float gamma, int iterations, const std::vector<cv::Point>& initialContour) {
    
    std::vector<cv::Point> contour = initialContour;
    if (contour.empty()) {
        contour = generateInitialCircle(image.cols, image.rows);
    }

    // Draw initial contour (BLUE)
    for (size_t i = 0; i < contour.size(); i++) {
        cv::line(image, contour[i], contour[(i + 1) % contour.size()], cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
    }

    cv::Mat externalEnergy = computeExternalEnergy(image);
    int numPoints = contour.size();
    
    for (int iter = 0; iter < iterations; iter++) {
        float avgDist = calculateAverageDistance(contour);
        bool pointsMoved = false;

        for (int i = 0; i < numPoints; i++) {
            cv::Point prev = contour[(i - 1 + numPoints) % numPoints];
            cv::Point curr = contour[i];
            cv::Point next = contour[(i + 1) % numPoints];

            std::vector<NeighborPoint> neighbors;
            float min_cont = 1e9, max_cont = -1e9;
            float min_curv = 1e9, max_curv = -1e9;
            float min_ext  = 1e9, max_ext  = -1e9;

            // STEP 1: Calculate raw energies and find local min/max for normalization
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    cv::Point neighbor(curr.x + dx, curr.y + dy);

                    if (neighbor.x < 0 || neighbor.x >= image.cols || neighbor.y < 0 || neighbor.y >= image.rows) {
                        continue;
                    }

                    float e_cont = getContinuityEnergy(neighbor, prev, avgDist);
                    float e_curv = getCurvatureEnergy(prev, neighbor, next);
                    float e_ext  = externalEnergy.at<float>(neighbor.y, neighbor.x);

                    min_cont = std::min(min_cont, e_cont); max_cont = std::max(max_cont, e_cont);
                    min_curv = std::min(min_curv, e_curv); max_curv = std::max(max_curv, e_curv);
                    min_ext  = std::min(min_ext, e_ext);   max_ext  = std::max(max_ext, e_ext);

                    neighbors.push_back({neighbor, e_cont, e_curv, e_ext});
                }
            }

            // STEP 2: Normalize the energies to [0, 1] and apply Greedy selection
            cv::Point bestMove = curr;
            float minTotalEnergy = 1e9;

            for (const auto& n : neighbors) {
                // Protect against division by zero if all pixels in the 3x3 window have the exact same energy
                float n_cont = (max_cont - min_cont) > 1e-5 ? (n.e_cont - min_cont) / (max_cont - min_cont) : 0;
                float n_curv = (max_curv - min_curv) > 1e-5 ? (n.e_curv - min_curv) / (max_curv - min_curv) : 0;
                float n_ext  = (max_ext - min_ext)   > 1e-5 ? (n.e_ext - min_ext) / (max_ext - min_ext) : 0;

                float totalEnergy = (alpha * n_cont) + (beta * n_curv) + (gamma * n_ext);

                if (totalEnergy < minTotalEnergy) {
                    minTotalEnergy = totalEnergy;
                    bestMove = n.pt;
                }
            }

            if (bestMove != curr) {
                contour[i] = bestMove;
                pointsMoved = true;
            }
        }

        if (!pointsMoved) break;
    }

    // Draw final evolved contour (RED)
    for (size_t i = 0; i < contour.size(); i++) {
        cv::line(image, contour[i], contour[(i + 1) % contour.size()], cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        cv::circle(image, contour[i], 2, cv::Scalar(0, 255, 255), -1); 
    }

    return contour;
}

cv::Mat GreedySnake::computeExternalEnergy(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }

    // Heavy blur to create a strong magnetic field for the edges
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2.0);

    cv::Mat gradX, gradY, magnitude;
    cv::Sobel(gray, gradX, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gradY, CV_32F, 0, 1, 3);
    cv::magnitude(gradX, gradY, magnitude);

    double minVal, maxVal;
    cv::minMaxLoc(magnitude, &minVal, &maxVal);
    if (maxVal > 0) {
        magnitude = magnitude / maxVal;
    }

    return -magnitude; 
}

std::vector<cv::Point> GreedySnake::generateInitialCircle(int width, int height) {
    std::vector<cv::Point> contour;
    cv::Point center(width / 2, height / 2);
    int radius = static_cast<int>(std::min(width, height));

    int numPoints = 100; // Increased points for smoother wrapping
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * CV_PI * i / numPoints;
        int x = center.x + radius * std::cos(angle);
        int y = center.y + radius * std::sin(angle);
        contour.push_back(cv::Point(x, y));
    }
    return contour;
}

float GreedySnake::getContinuityEnergy(cv::Point curr, cv::Point prev, float avgDistance) {
    float dist = std::sqrt(std::pow(curr.x - prev.x, 2) + std::pow(curr.y - prev.y, 2));
    return std::pow(dist - avgDistance, 2);
}

float GreedySnake::getCurvatureEnergy(cv::Point prev, cv::Point curr, cv::Point next) {
    float cx = prev.x - 2 * curr.x + next.x;
    float cy = prev.y - 2 * curr.y + next.y;
    return (cx * cx + cy * cy);
}

float GreedySnake::calculateAverageDistance(const std::vector<cv::Point>& contour) {
    float totalDist = 0;
    int n = contour.size();
    for (int i = 0; i < n; i++) {
        cv::Point p1 = contour[i];
        cv::Point p2 = contour[(i + 1) % n];
        totalDist += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
    }
    return totalDist / n;
}