#include "HoughLines.h"
#include <cmath>
#include <vector>

// Signature updated: 'src' for math, 'canvas' for drawing
cv::Mat HoughLines::detect(cv::Mat src, cv::Mat canvas, int threshold) {
    if (src.empty()) return canvas;

    // 1. Process Edges on the CLEAN source image
    cv::Mat gray, edges;
    if (src.channels() > 1) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else gray = src.clone();
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 1.0);
    cv::Canny(gray, edges, 50, 150); 

    // 2. Prepare Output (Use the existing canvas to keep previous drawings)
    cv::Mat output = canvas.clone();
    if (output.channels() == 1) cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);

    int rows = edges.rows, cols = edges.cols;
    double diagonal = std::sqrt(rows * rows + cols * cols);
    int numThetas = 180, numRhos = static_cast<int>(2 * diagonal);
    std::vector<std::vector<int>> accumulator(numRhos, std::vector<int>(numThetas, 0));

    std::vector<double> sinT(numThetas), cosT(numThetas);
    for (int t = 0; t < numThetas; t++) {
        double rad = (t * CV_PI) / 180.0;
        sinT[t] = std::sin(rad); cosT[t] = std::cos(rad);
    }

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (edges.at<uchar>(y, x) > 0) { 
                for (int t = 0; t < numThetas; t++) {
                    double r = x * cosT[t] + y * sinT[t];
                    int rIdx = static_cast<int>(r + diagonal);
                    if (rIdx >= 0 && rIdx < numRhos) accumulator[rIdx][t]++;
                }
            }
        }
    }

    for (int r = 2; r < numRhos - 2; r++) {
        for (int t = 0; t < numThetas; t++) {
            int votes = accumulator[r][t];
            int lThresh = (t < 45 || t > 135) ? threshold * 0.7 : threshold;
            if (votes >= lThresh) {
                bool isPeak = true;
                for (int dr = -2; dr <= 2; dr++) {
                    for (int dt = -2; dt <= 2; dt++) {
                        if (dr == 0 && dt == 0) continue;
                        if (accumulator[r + dr][(t + dt + 180) % 180] > votes) { isPeak = false; break; }
                    }
                    if (!isPeak) break;
                }
                if (isPeak) {
                    double rho = r - diagonal, theta = (t * CV_PI) / 180.0;
                    double a = std::cos(theta), b = std::sin(theta);
                    double x0 = a * rho, y0 = b * rho;
                    cv::Point p1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
                    cv::Point p2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
                    // Draw RED lines
                    cv::line(output, p1, p2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
                }
            }
        }
    }
    return output;
}
