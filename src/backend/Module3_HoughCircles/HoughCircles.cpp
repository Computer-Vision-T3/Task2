#include "HoughCircles.h"
#include <cmath>
#include <algorithm>

cv::Mat HoughCircles::detect(cv::Mat src, cv::Mat canvas, int minR, int maxR, int threshold) {
    if (src.empty()) return canvas;

    cv::Mat gray, edges, dx, dy;
    if (src.channels() > 1) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else gray = src.clone();

    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);
    // Calculate gradients to know WHERE to vote
    cv::Sobel(gray, dx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, dy, CV_32F, 0, 1, 3);
    cv::Canny(gray, edges, 50, 150);

    int rows = edges.rows, cols = edges.cols;
    if (maxR <= 0) maxR = std::min(rows, cols) / 2;
    int numRadii = maxR - minR + 1;

    std::vector<cv::Mat> accumulator(numRadii);
    for (int i = 0; i < numRadii; ++i) accumulator[i] = cv::Mat::zeros(rows, cols, CV_32S);

    // 1. Gradient-Directed Voting (Very Accurate)
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (edges.at<uchar>(y, x) > 0) {
                float gx = dx.at<float>(y, x);
                float gy = dy.at<float>(y, x);
                float angle = std::atan2(gy, gx);

                for (int r = minR; r <= maxR; r++) {
                    int rIdx = r - minR;
                    // Center is at (x ± r*cos, y ± r*sin)
                    for (int sign : {-1, 1}) {
                        int cx = x + sign * cvRound(r * std::cos(angle));
                        int cy = y + sign * cvRound(r * std::sin(angle));
                        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
                            accumulator[rIdx].at<int>(cy, cx)++;
                        }
                    }
                }
            }
        }
    }

    cv::Mat output = canvas.clone();
    struct Circle { int x, y, r, v; };
    std::vector<Circle> candidates;

    // 2. Peak Detection
    for (int rIdx = 0; rIdx < numRadii; rIdx++) {
        cv::Mat blurredAcc;
        accumulator[rIdx].convertTo(blurredAcc, CV_32F);
        cv::GaussianBlur(blurredAcc, blurredAcc, cv::Size(3, 3), 0); // Smooth votes

        for (int y = 10; y < rows - 10; y++) {
            for (int x = 10; x < cols - 10; x++) {
                float votes = blurredAcc.at<float>(y, x);
                if (votes > threshold) {
                    candidates.push_back({x, y, rIdx + minR, (int)votes});
                }
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Circle& a, const Circle& b) { return a.v > b.v; });

    // 3. Circle Validation & NMS
    std::vector<Circle> finalCircles;
    for (const auto& c : candidates) {
        bool duplicate = false;
        for (const auto& f : finalCircles) {
            float dist = std::sqrt(std::pow(c.x - f.x, 2) + std::pow(c.y - f.y, 2));
            if (dist < f.r * 0.8) { duplicate = true; break; }
        }
        if (!duplicate) {
            // Verify: count actual edge pixels on circumference
            int support = 0;
            for (int a = 0; a < 360; a += 10) {
                int ex = c.x + cvRound(c.r * std::cos(a * CV_PI / 180.0));
                int ey = c.y + cvRound(c.r * std::sin(a * CV_PI / 180.0));
                if (ex >= 0 && ex < cols && ey >= 0 && ey < rows && edges.at<uchar>(ey, ex) > 0) support++;
            }
            if (support > 8) { // Requires 25% edge coverage
                cv::circle(output, cv::Point(c.x, c.y), c.r, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                finalCircles.push_back(c);
                if (finalCircles.size() >= 5) break; 
            }
        }
    }
    return output;
}
