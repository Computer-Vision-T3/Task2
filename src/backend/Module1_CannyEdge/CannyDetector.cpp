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

    Mat blurred = applyGaussianBlur(gray);
    Mat mag, angle;
    calculateGradients(blurred, mag, angle);
    Mat suppressed = nonMaximumSuppression(mag, angle);
    return hysteresis(suppressed, lowThresh, highThresh);
}

cv::Mat CannyDetector::applyGaussianBlur(const Mat& input) {
    // 5x5 Gaussian kernel (sigma ≈ 1.4), sum = 159
    float kernel[5][5] = {
        {2,  4,  5,  4,  2},
        {4,  9, 12,  9,  4},
        {5, 12, 15, 12,  5},
        {4,  9, 12,  9,  4},
        {2,  4,  5,  4,  2}
    };
    const float kernelSum = 159.0f;

    Mat output = Mat::zeros(input.size(), CV_32F);
    Mat padded;
    copyMakeBorder(input, padded, 2, 2, 2, 2, BORDER_REPLICATE);

    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            float sum = 0.0f;
            for (int ki = -2; ki <= 2; ki++) {
                for (int kj = -2; kj <= 2; kj++) {
                    sum += static_cast<float>(padded.at<uchar>(i + ki + 2, j + kj + 2))
                           * kernel[ki + 2][kj + 2];
                }
            }
            output.at<float>(i, j) = sum / kernelSum;
        }
    }
    return output;
}

void CannyDetector::calculateGradients(const Mat& input, Mat& magnitude, Mat& angle) {
    magnitude = Mat::zeros(input.size(), CV_32F);
    angle     = Mat::zeros(input.size(), CV_8U);

    const int Kx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int Ky[3][3] = {{ 1, 2, 1}, { 0, 0, 0}, {-1,-2,-1}};

    Mat padded;
    copyMakeBorder(input, padded, 1, 1, 1, 1, BORDER_REPLICATE);

    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            float gx = 0.0f, gy = 0.0f;
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    float pixel = padded.at<float>(i + ki + 1, j + kj + 1);
                    gx += pixel * static_cast<float>(Kx[ki + 1][kj + 1]);
                    gy += pixel * static_cast<float>(Ky[ki + 1][kj + 1]);
                }
            }
            magnitude.at<float>(i, j) = std::sqrt(gx * gx + gy * gy);

            // Angle in degrees, mapped to [0, 180)
            float theta = std::atan2(gy, gx) * 180.0f / static_cast<float>(CV_PI);
            if (theta < 0.0f) theta += 180.0f;

            // Quantise to 4 directions: 0, 45, 90, 135
            uchar dir;
            if ((theta >= 0.0f && theta < 22.5f) || (theta >= 157.5f && theta < 180.0f))
                dir = 0;
            else if (theta >= 22.5f && theta < 67.5f)
                dir = 45;
            else if (theta >= 67.5f && theta < 112.5f)
                dir = 90;
            else
                dir = 135;

            angle.at<uchar>(i, j) = dir;
        }
    }
}

cv::Mat CannyDetector::nonMaximumSuppression(const Mat& magnitude, const Mat& angle) {
    Mat output = Mat::zeros(magnitude.size(), CV_32F);

    for (int i = 1; i < magnitude.rows - 1; i++) {
        for (int j = 1; j < magnitude.cols - 1; j++) {
            float q = 255.0f, r = 255.0f;
            uchar currentAngle = angle.at<uchar>(i, j);

            if (currentAngle == 0) {
                q = magnitude.at<float>(i, j + 1);
                r = magnitude.at<float>(i, j - 1);
            } else if (currentAngle == 45) {
                q = magnitude.at<float>(i + 1, j - 1);
                r = magnitude.at<float>(i - 1, j + 1);
            } else if (currentAngle == 90) {
                q = magnitude.at<float>(i + 1, j);
                r = magnitude.at<float>(i - 1, j);
            } else if (currentAngle == 135) {
                q = magnitude.at<float>(i - 1, j - 1);
                r = magnitude.at<float>(i + 1, j + 1);
            }

            float cur = magnitude.at<float>(i, j);
            output.at<float>(i, j) = (cur >= q && cur >= r) ? cur : 0.0f;
        }
    }
    return output;
}

cv::Mat CannyDetector::hysteresis(const Mat& img, int low, int high) {
    Mat result = Mat::zeros(img.size(), CV_8U);
    std::queue<std::pair<int, int>> q;

    // First pass: classify pixels as strong (255) or weak (100)
    for (int i = 1; i < img.rows - 1; i++) {
        for (int j = 1; j < img.cols - 1; j++) {
            float val = img.at<float>(i, j);
            if (val >= static_cast<float>(high)) {
                result.at<uchar>(i, j) = 255;
                q.push({i, j});
            } else if (val >= static_cast<float>(low)) {
                result.at<uchar>(i, j) = 100;
            }
        }
    }

    // 8-connectivity offsets
    const int dr[] = {-1, -1, -1,  0,  0,  1,  1,  1};
    const int dc[] = {-1,  0,  1, -1,  1, -1,  0,  1};

    // BFS from strong pixels: promote connected weak pixels to strong
    while (!q.empty()) {
        auto [cy, cx] = q.front(); q.pop();
        for (int k = 0; k < 8; k++) {
            int ny = cy + dr[k], nx = cx + dc[k];
            if (ny < 0 || ny >= result.rows || nx < 0 || nx >= result.cols) continue;
            if (result.at<uchar>(ny, nx) == 100) {
                result.at<uchar>(ny, nx) = 255;
                q.push({ny, nx});
            }
        }
    }

    // Final pass: suppress remaining isolated weak pixels
    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            if (result.at<uchar>(i, j) == 100)
                result.at<uchar>(i, j) = 0;
        }
    }

    return result;
}
