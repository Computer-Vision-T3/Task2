#include "CannyDetector.h"
#include <cmath>
#include <queue>
#include <algorithm>

using namespace cv;

cv::Mat CannyDetector::apply(cv::Mat img, int lowThresh, int highThresh) {
    if (img.empty()) return img;
    Mat gray;
    if (img.channels() > 1) cvtColor(img, gray, COLOR_BGR2GRAY);
    else gray = img.clone();
    return cannyHandmade(gray, lowThresh, highThresh);
}

cv::Mat CannyDetector::cannyHandmade(const Mat& image, int low, int high) {
    // 1. Noise Reduction: Use a strong 7x7 Gaussian to kill grass texture
    Mat blurred;
    GaussianBlur(image, blurred, Size(7, 7), 1.5);

    // 2. Gradients (Sobel)
    Mat gx, gy, mag;
    Sobel(blurred, gx, CV_32F, 1, 0, 3);
    Sobel(blurred, gy, CV_32F, 0, 1, 3);
    magnitude(gx, gy, mag);

    // 3. Sub-pixel Non-Maximum Suppression
    // This logic interpolates values to get perfectly thin 1-pixel lines
    Mat suppressed = Mat::zeros(mag.size(), CV_32F);
    
    for (int i = 2; i < mag.rows - 2; i++) {
        for (int j = 2; j < mag.cols - 2; j++) {
            float x = gx.at<float>(i, j);
            float y = gy.at<float>(i, j);
            float M = mag.at<float>(i, j);
            
            if (M < 1e-4) continue; // Ignore nearly zero gradients

            float mag1, mag2;
            float absX = std::abs(x);
            float absY = std::abs(y);

            // Use the ratio of Gx/Gy to interpolate between the 8 neighbors
            if (absX >= absY) {
                float weight = absY / absX;
                float s = (x * y > 0) ? 1 : -1;
                // Interpolate along horizontal/diagonal
                mag1 = (1 - weight) * mag.at<float>(i, j + (x > 0 ? 1 : -1)) + 
                       weight * mag.at<float>(i + (s > 0 ? 1 : -1), j + (x > 0 ? 1 : -1));
                mag2 = (1 - weight) * mag.at<float>(i, j - (x > 0 ? 1 : -1)) + 
                       weight * mag.at<float>(i - (s > 0 ? 1 : -1), j - (x > 0 ? 1 : -1));
            } else {
                float weight = absX / absY;
                float s = (x * y > 0) ? 1 : -1;
                // Interpolate along vertical/diagonal
                mag1 = (1 - weight) * mag.at<float>(i + (y > 0 ? 1 : -1), j) + 
                       weight * mag.at<float>(i + (y > 0 ? 1 : -1), j + (s > 0 ? 1 : -1));
                mag2 = (1 - weight) * mag.at<float>(i - (y > 0 ? 1 : -1), j) + 
                       weight * mag.at<float>(i - (y > 0 ? 1 : -1), j - (s > 0 ? 1 : -1));
            }

            if (M >= mag1 && M >= mag2) {
                suppressed.at<float>(i, j) = M;
            }
        }
    }

    // 4. Hysteresis
    return hysteresis(suppressed, low, high);
}

cv::Mat CannyDetector::hysteresis(const Mat& img, int low, int high) {
    Mat result = Mat::zeros(img.size(), CV_8U);
    std::queue<std::pair<int, int>> q;

    // Seed strong pixels
    for (int i = 2; i < img.rows - 2; i++) {
        for (int j = 2; j < img.cols - 2; j++) {
            float val = img.at<float>(i, j);
            if (val >= high) {
                result.at<uchar>(i, j) = 255;
                q.push({i, j});
            } else if (val >= low) {
                result.at<uchar>(i, j) = 100; // Temporary label for weak
            }
        }
    }

    // Connect weak to strong
    int dx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1,-1, 1,-1, 0, 1};

    while (!q.empty()) {
        auto [cy, cx] = q.front(); q.pop();
        for (int k = 0; k < 8; k++) {
            int ny = cy + dy[k], nx = cx + dx[k];
            if (result.at<uchar>(ny, nx) == 100) {
                result.at<uchar>(ny, nx) = 255;
                q.push({ny, nx});
            }
        }
    }

    // Kill remaining isolated weak pixels
    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            if (result.at<uchar>(i, j) == 100) result.at<uchar>(i, j) = 0;
        }
    }

    return result;
}
