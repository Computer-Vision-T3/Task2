#include "CannyDetector.h"
#include <cmath>
#include <queue>
#include <algorithm>

using namespace cv;

cv::Mat CannyDetector::apply(cv::Mat img, int lowThresh, int highThresh) {
    if (img.empty() || img.rows < 5 || img.cols < 5)
    return Mat();

if (lowThresh > highThresh)
    std::swap(lowThresh, highThresh);
    Mat gray;
    if (img.channels() > 1) cvtColor(img, gray, COLOR_BGR2GRAY);
    else gray = img.clone();
    return cannyHandmade(gray, lowThresh, highThresh);
}

cv::Mat CannyDetector::cannyHandmade(const Mat& image, int low, int high) {
    // Step 1: Noise reduction
    // Use a 5x5 Gaussian with sigma=2.0 to smooth noise while preserving
    // meaningful edges (7x7 was too aggressive and blurred real edges)
    Mat blurred;
    GaussianBlur(image, blurred, Size(5, 5), 2.0);

    // Step 2: Gradient computation using Sobel operators
    Mat gx, gy, mag;
    Sobel(blurred, gx, CV_32F, 1, 0, 3);
    Sobel(blurred, gy, CV_32F, 0, 1, 3);
    magnitude(gx, gy, mag);

    // Step 3: Sub-pixel Non-Maximum Suppression
    // Interpolates gradient magnitudes between neighbors to produce
    // perfectly thin 1-pixel-wide edge lines
    Mat suppressed = Mat::zeros(mag.size(), CV_32F);

    for (int i = 2; i < mag.rows - 2; i++) {
        for (int j = 2; j < mag.cols - 2; j++) {
            float x = gx.at<float>(i, j);
            float y = gy.at<float>(i, j);
            float M = mag.at<float>(i, j);

            // Skip pixels with negligible gradient magnitude
            if (M < 1e-4) continue;

            float mag1, mag2;
            float absX = std::abs(x);
            float absY = std::abs(y);

            // Interpolate along the gradient direction using Gx/Gy ratio
            if (absX >= absY) {
                // Gradient is more horizontal — interpolate left/right neighbors
                float weight = (absX > 1e-6f) ? absY / absX : 0.0f;
                int xDir = (x > 0) ? 1 : -1;
                int diag = (x * y >= 0) ? 1 : -1; // diagonal row direction
                mag1 = (1 - weight) * mag.at<float>(i,          j + xDir)  +
                       weight       * mag.at<float>(i + diag,   j + xDir);
                mag2 = (1 - weight) * mag.at<float>(i,          j - xDir)  +
                       weight       * mag.at<float>(i - diag,   j - xDir);
            } else {
                // Gradient is more vertical — interpolate top/bottom neighbors
                float weight = (absY > 1e-6f) ? absX / absY : 0.0f;
                int yDir = (y > 0) ? 1 : -1;
                int diag = (x * y >= 0) ? 1 : -1; // diagonal column direction
                mag1 = (1 - weight) * mag.at<float>(i + yDir,   j)         +
                       weight       * mag.at<float>(i + yDir,   j + diag);
                mag2 = (1 - weight) * mag.at<float>(i - yDir,   j)         +
                       weight       * mag.at<float>(i - yDir,   j - diag);
            }

            // Keep only local maxima along the gradient direction
            if (M >= mag1 && M >= mag2) {
                suppressed.at<float>(i, j) = M;
            }
        }
    }

    // Step 4: Hysteresis thresholding to finalize edge connectivity
    return hysteresis(suppressed, low, high);
}

cv::Mat CannyDetector::hysteresis(const Mat& img, int low, int high) {
    Mat result = Mat::zeros(img.size(), CV_8U);
    std::queue<std::pair<int, int>> q;

    // First pass: classify pixels as strong (255) or weak (100)
    // Start from i=1 to leave a 1-pixel border for safe neighbor access
    for (int i = 1; i < img.rows - 1; i++) {
        for (int j = 1; j < img.cols - 1; j++) {
            float val = img.at<float>(i, j);
            if (val >= high) {
                result.at<uchar>(i, j) = 255;
                q.push({i, j});
            } else if (val >= low) {
                result.at<uchar>(i, j) = 100; // Temporary label for weak pixels
            }
        }
    }

    // 8-connectivity offsets: dr = row offset, dc = column offset
    int dr[] = {-1, -1, -1,  0,  0,  1,  1,  1};
    int dc[] = {-1,  0,  1, -1,  1, -1,  0,  1};

    // BFS from strong pixels: promote connected weak pixels to strong
    while (!q.empty()) {
        auto [cy, cx] = q.front(); q.pop();
        for (int k = 0; k < 8; k++) {
            int ny = cy + dr[k], nx = cx + dc[k];

            // Bounds check to prevent out-of-bounds memory access
            if (ny < 0 || ny >= result.rows || nx < 0 || nx >= result.cols) continue;

            if (result.at<uchar>(ny, nx) == 100) {
                result.at<uchar>(ny, nx) = 255;
                q.push({ny, nx});
            }
        }
    }

    // Final pass: suppress any remaining isolated weak pixels
    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            if (result.at<uchar>(i, j) == 100) result.at<uchar>(i, j) = 0;
        }
    }

    return result;
}
