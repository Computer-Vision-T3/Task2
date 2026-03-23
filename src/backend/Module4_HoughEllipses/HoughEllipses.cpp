#include "HoughEllipses.h"
#include <vector>
#include <cmath>
#include <algorithm>

cv::Mat HoughEllipses::detect(cv::Mat src, cv::Mat canvas, int minMajor, int maxMajor, int minVotes) {
    if (src.empty()) return canvas;

    cv::Mat gray, edges;
    if (src.channels() > 1) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else gray = src.clone();

    // 1. Clean Pre-processing
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.2);
    cv::Canny(gray, edges, 50, 150);

    std::vector<cv::Point> edgePts;
    for (int y = 0; y < edges.rows; y++) {
        for (int x = 0; x < edges.cols; x++) {
            if (edges.at<uchar>(y, x) > 0) edgePts.push_back(cv::Point(x, y));
        }
    }

    // Limit points for performance, but use enough for accuracy
    std::vector<cv::Point> sampled;
    int target = 400; 
    int step = std::max(1, (int)(edgePts.size() / target));
    for (size_t i = 0; i < edgePts.size() && sampled.size() < target; i += step) {
        sampled.push_back(edgePts[i]);
    }

    if (maxMajor <= 0) maxMajor = std::max(edges.cols, edges.rows);

    struct ElData { cv::Point2f c; float a, b, ang; float score; };
    std::vector<ElData> candidates;

    int N = sampled.size();
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            cv::Point2f p1 = sampled[i], p2 = sampled[j];
            float dist = cv::norm(p1 - p2);
            float a = dist / 2.0f;
            if (a < minMajor || a > maxMajor) continue;

            cv::Point2f center = (p1 + p2) * 0.5f;
            float angle = std::atan2(p2.y - p1.y, p2.x - p1.x);
            float cosA = std::cos(angle), sinA = std::sin(angle);

            std::vector<int> b_acc(std::ceil(a) + 1, 0);
            for (int k = 0; k < N; k++) {
                if (k == i || k == j) continue;
                float dx = sampled[k].x - center.x, dy = sampled[k].y - center.y;
                if (std::sqrt(dx*dx + dy*dy) >= a) continue;

                float u = std::abs(dx * cosA + dy * sinA);
                float v = std::abs(-dx * sinA + dy * cosA);
                float b2 = (a*a * v*v) / (a*a - u*u + 0.00001f);
                if (b2 > 0) {
                    int b = cvRound(std::sqrt(b2));
                    if (b > 5 && b <= a) b_acc[b]++;
                }
            }

            auto it = std::max_element(b_acc.begin(), b_acc.end());
            int votes = *it, bestB = std::distance(b_acc.begin(), it);

            // --- IMPROVED VERIFICATION ---
            if (votes >= minVotes) {
                int hits = 0, total = 40;
                for (int t = 0; t < 360; t += (360/total)) {
                    float rad = t * CV_PI / 180.0f;
                    // Note: OpenCV uses a and b for half-axes
                    float ex = a * std::cos(rad), ey = bestB * std::sin(rad);
                    int rx = cvRound(center.x + (ex * cosA - ey * sinA));
                    int ry = cvRound(center.y + (ex * sinA + ey * cosA));
                    
                    if (rx >= 0 && rx < edges.cols && ry >= 0 && ry < edges.rows) {
                        bool foundLocal = false;
                        for(int dy_f=-1; dy_f<=1 && !foundLocal; dy_f++)
                            for(int dx_f=-1; dx_f<=1; dx_f++)
                                if (edges.at<uchar>(std::max(0,std::min(ry+dy_f, edges.rows-1)), 
                                                   std::max(0,std::min(rx+dx_f, edges.cols-1))) > 0) 
                                { foundLocal = true; break; }
                        if (foundLocal) hits++;
                    }
                }
                float score = (float)hits / total;
                if (score > 0.6) candidates.push_back({center, a, (float)bestB, (float)(angle * 180/CV_PI), score});
            }
        }
    }

    // --- AGGRESSIVE CLUSTERING (Fixes the messy overlap) ---
    std::sort(candidates.begin(), candidates.end(), [](const ElData& x, const ElData& y) { 
        return x.score > y.score; 
    });
    
    cv::Mat output = canvas.clone();
    std::vector<ElData> finalSelection;

    for (const auto& e : candidates) {
        bool duplicate = false;
        for (const auto& f : finalSelection) {
            float distCenter = cv::norm(e.c - f.c);
            float diffSize = std::abs(e.a - f.a) + std::abs(e.b - f.b);
            // If centers are closer than 30px and sizes are similar, ignore the weaker one
            if (distCenter < 40 && diffSize < 30) { 
                duplicate = true; break; 
            }
        }
        if (!duplicate) {
            cv::ellipse(output, e.c, cv::Size(e.a, e.b), e.ang, 0, 360, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
            finalSelection.push_back(e);
            if (finalSelection.size() >= 10) break; // Maximum ellipses to detect
        }
    }

    return output;
}
