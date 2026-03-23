#include "GreedySnake.h"
#include <cmath>
#include <algorithm>

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

    // Initial clean-up to ensure perfect spacing right out of the gate
    contour = interpolatePoints(contour, 12);

    // Draw initial contour (BLUE) 
    for (size_t i = 0; i < contour.size(); i++) {
        cv::line(image, contour[i], contour[(i + 1) % contour.size()], cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
    }

    cv::Mat externalEnergy = computeExternalEnergy(image);
    
    // Run twice as many iterations under the hood to ensure it covers long distances
    int actualIterations = iterations * 2; 

    for (int iter = 0; iter < actualIterations; iter++) {
        
        // THE MAGIC BULLET: Re-space the points every 5 iterations to prevent zigzags and bunching!
        if (iter % 5 == 0) {
            contour = interpolatePoints(contour, 12);
        }

        int numPoints = contour.size();
        bool pointsMoved = false;

        for (int i = 0; i < numPoints; i++) {
            cv::Point prev = contour[(i - 1 + numPoints) % numPoints];
            cv::Point curr = contour[i];
            cv::Point next = contour[(i + 1) % numPoints];

            std::vector<NeighborPoint> neighbors;
            float min_cont = 1e9, max_cont = -1e9;
            float min_curv = 1e9, max_curv = -1e9;
            float min_ext  = 1e9, max_ext  = -1e9;

            // 7x7 Search Window (Radius 3)
            int searchRadius = 3; 
            for (int dy = -searchRadius; dy <= searchRadius; dy++) {
                for (int dx = -searchRadius; dx <= searchRadius; dx++) {
                    cv::Point neighbor(curr.x + dx, curr.y + dy);

                    if (neighbor.x < 0 || neighbor.x >= image.cols || neighbor.y < 0 || neighbor.y >= image.rows) continue;

                    // Calculate raw energies
                    float e_cont = getContinuityEnergy(neighbor, prev, 0); 
                    float e_curv = getCurvatureEnergy(prev, neighbor, next);
                    float e_ext  = externalEnergy.at<float>(neighbor.y, neighbor.x);

                    min_cont = std::min(min_cont, e_cont); max_cont = std::max(max_cont, e_cont);
                    min_curv = std::min(min_curv, e_curv); max_curv = std::max(max_curv, e_curv);
                    min_ext  = std::min(min_ext, e_ext);   max_ext  = std::max(max_ext, e_ext);

                    neighbors.push_back({neighbor, e_cont, e_curv, e_ext});
                }
            }

            cv::Point bestMove = curr;
            float minTotalEnergy = 1e9;

            float diff_cont = std::max(1e-5f, max_cont - min_cont);
            float diff_curv = std::max(1e-5f, max_curv - min_curv);
            float diff_ext  = std::max(1e-5f, max_ext - min_ext);

            for (const auto& n : neighbors) {
                float n_cont = (n.e_cont - min_cont) / diff_cont;
                float n_curv = (n.e_curv - min_curv) / diff_curv;
                float n_ext  = (n.e_ext  - min_ext)  / diff_ext;

                // Balanced weights (Gamma gets a minor boost to guide it home)
                float totalEnergy = (alpha * n_cont) + (beta * n_curv) + (gamma * 5.0f * n_ext);

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

        // Only break if the points completely stop moving
        if (!pointsMoved) break;
    }

    // Final cleanup before drawing
    contour = interpolatePoints(contour, 12);

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

    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    // Wipe the invisible window frames
    cv::rectangle(edges, cv::Point(0, 0), cv::Point(edges.cols - 1, edges.rows - 1), cv::Scalar(0), 10);

    cv::Mat binary = cv::Mat::ones(edges.size(), CV_8UC1) * 255;
    binary.setTo(0, edges > 0);

    cv::Mat distMap;
    cv::distanceTransform(binary, distMap, cv::DIST_L2, 5);
    cv::normalize(distMap, distMap, 0.0, 1.0, cv::NORM_MINMAX);

    return distMap; 
}

std::vector<cv::Point> GreedySnake::generateInitialCircle(int width, int height) {
    std::vector<cv::Point> contour;
    cv::Point center(width / 2, height / 2);
    
    int radiusX = (width / 2) - 5;
    int radiusY = (height / 2) - 5;

    int numPoints = 100; 
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * CV_PI * i / numPoints;
        int x = center.x + radiusX * std::cos(angle);
        int y = center.y + radiusY * std::sin(angle);
        
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));
        
        contour.push_back(cv::Point(x, y));
    }
    return contour;
}

float GreedySnake::getContinuityEnergy(cv::Point curr, cv::Point prev, float avgDistance) {
    // True elasticity: simply acts as a rubber band trying to minimize its length
    return std::pow(curr.x - prev.x, 2) + std::pow(curr.y - prev.y, 2);
}

float GreedySnake::getCurvatureEnergy(cv::Point prev, cv::Point curr, cv::Point next) {
    float cx = prev.x - 2 * curr.x + next.x;
    float cy = prev.y - 2 * curr.y + next.y;
    return (cx * cx + cy * cy);
}

float GreedySnake::calculateAverageDistance(const std::vector<cv::Point>& contour) {
    return 0; // Deprecated due to the new resampler, but kept to satisfy the header signature
}

std::vector<cv::Point> GreedySnake::interpolatePoints(const std::vector<cv::Point>& points, int targetSpacing) {
    if (points.size() < 3) return points;
    
    // 1. Calculate total perimeter
    float perimeter = 0;
    std::vector<float> cumDist;
    cumDist.push_back(0);
    
    for (size_t i = 0; i < points.size(); i++) {
        cv::Point p1 = points[i];
        cv::Point p2 = points[(i + 1) % points.size()];
        perimeter += cv::norm(p1 - p2);
        cumDist.push_back(perimeter);
    }
    
    // 2. Calculate exactly how many points we need to maintain perfect spacing
    int numPoints = std::max(10, (int)std::round(perimeter / targetSpacing));
    float step = perimeter / numPoints;
    
    std::vector<cv::Point> resampled;
    
    // 3. Destroy old points and lay down the new evenly spaced points
    int currentSegment = 0;
    for (int i = 0; i < numPoints; i++) {
        float targetD = i * step;
        
        while (currentSegment < points.size() && cumDist[currentSegment + 1] < targetD) {
            currentSegment++;
        }
        
        if (currentSegment >= points.size()) currentSegment = points.size() - 1;
        
        float segStart = cumDist[currentSegment];
        float segEnd = cumDist[currentSegment + 1];
        float segLen = segEnd - segStart;
        
        float t = (segLen > 0) ? (targetD - segStart) / segLen : 0.0f;
        
        cv::Point p1 = points[currentSegment];
        cv::Point p2 = points[(currentSegment + 1) % points.size()];
        
        float nx = p1.x + t * (p2.x - p1.x);
        float ny = p1.y + t * (p2.y - p1.y);
        
        resampled.push_back(cv::Point(cvRound(nx), cvRound(ny)));
    }
    
    return resampled;
}