#include "CannyDetector.h"
#include <cmath>
#include <queue>

using namespace cv;

// ================= Sobel =================
cv::Mat CannyDetector::sobel(const Mat& image, int kernel_size) {
    Mat grad_x, grad_y;
    Sobel(image, grad_x, CV_32F, 1, 0, kernel_size);
    Sobel(image, grad_y, CV_32F, 0, 1, kernel_size);

    Mat magnitude;
    magnitude = abs(grad_x) + abs(grad_y);

    return normalize(magnitude);
}

// ================= Prewitt =================
cv::Mat CannyDetector::prewitt(const Mat& image) {
    Mat kernel_x = (Mat_<float>(3,3) << 1,0,-1, 1,0,-1, 1,0,-1);
    Mat kernel_y = (Mat_<float>(3,3) << 1,1,1, 0,0,0, -1,-1,-1);

    Mat gx, gy;
    filter2D(image, gx, CV_32F, kernel_x);
    filter2D(image, gy, CV_32F, kernel_y);

    Mat mag;
    magnitude(gx, gy, mag);

    return normalize(mag);
}

// ================= Roberts =================
cv::Mat CannyDetector::roberts(const Mat& image) {
    Mat kernel_x = (Mat_<float>(2,2) << -1,0,0,1);
    Mat kernel_y = (Mat_<float>(2,2) << 0,-1,1,0);

    Mat gx, gy;
    filter2D(image, gx, CV_32F, kernel_x);
    filter2D(image, gy, CV_32F, kernel_y);

    Mat mag;
    magnitude(gx, gy, mag);

    return normalize(mag);
}

// ================= OpenCV Canny =================
cv::Mat CannyDetector::canny(const Mat& image, int low, int high) {
    Mat edges;
    Canny(image, edges, low, high);
    return edges;
}

// ================= Sobel Gradients =================
void CannyDetector::sobelGradients(const Mat& image, Mat& magnitude, Mat& angle) {
    Mat gx, gy;
    Sobel(image, gx, CV_32F, 1, 0, 3);
    Sobel(image, gy, CV_32F, 0, 1, 3);

    magnitude.create(image.size(), CV_32F);
    angle.create(image.size(), CV_32F);

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            float x = gx.at<float>(i,j);
            float y = gy.at<float>(i,j);

            magnitude.at<float>(i,j) = sqrt(x*x + y*y);
            angle.at<float>(i,j) = atan2(y,x) * 180 / CV_PI;

            if (angle.at<float>(i,j) < 0)
                angle.at<float>(i,j) += 180;
        }
    }
}

// ================= Non-Max Suppression =================
cv::Mat CannyDetector::nonMaxSuppression(const Mat& mag, const Mat& angle) {
    Mat result = Mat::zeros(mag.size(), CV_32F);

    for (int i = 1; i < mag.rows - 1; i++) {
        for (int j = 1; j < mag.cols - 1; j++) {
            float q = 0, r = 0;
            float ang = angle.at<float>(i,j);

            if ((0 <= ang && ang < 22.5) || (157.5 <= ang && ang <= 180)) {
                q = mag.at<float>(i, j+1);
                r = mag.at<float>(i, j-1);
            }
            else if (22.5 <= ang && ang < 67.5) {
                q = mag.at<float>(i-1, j-1);
                r = mag.at<float>(i+1, j+1);
            }
            else if (67.5 <= ang && ang < 112.5) {
                q = mag.at<float>(i-1, j);
                r = mag.at<float>(i+1, j);
            }
            else {
                q = mag.at<float>(i+1, j-1);
                r = mag.at<float>(i-1, j+1);
            }

            if (mag.at<float>(i,j) >= q && mag.at<float>(i,j) >= r)
                result.at<float>(i,j) = mag.at<float>(i,j);
        }
    }

    return result;
}

// ================= Hysteresis =================
// Fix: using BFS to properly track chains of weak pixels.
// Old problem: only checked direct neighbors in the original img (1-hop only),
// so a weak pixel connected to a strong one through another weak pixel was lost.
// Solution: start a BFS from every strong pixel and propagate through connected weak pixels.
cv::Mat CannyDetector::hysteresis(const Mat& img, int low, int high) {
    Mat result = Mat::zeros(img.size(), CV_8U);

    // Pass 1: classify each pixel — strong = 255, weak = 128, non-edge = 0
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            float val = img.at<float>(i, j);
            if (val >= high)
                result.at<uchar>(i, j) = 255;
            else if (val >= low)
                result.at<uchar>(i, j) = 128;
        }
    }

    // Pass 2: BFS from every strong pixel
    // any weak pixel reachable from a strong one (directly or through a chain) becomes strong
    std::queue<std::pair<int,int>> bfsQueue;

    // seed the queue with all strong pixels
    for (int i = 1; i < img.rows - 1; i++)
        for (int j = 1; j < img.cols - 1; j++)
            if (result.at<uchar>(i, j) == 255)
                bfsQueue.push({i, j});

    // 8-connected neighbors
    int dx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1,-1, 1,-1, 0, 1};

    while (!bfsQueue.empty()) {
        auto [ci, cj] = bfsQueue.front();
        bfsQueue.pop();

        for (int d = 0; d < 8; d++) {
            int ni = ci + dx[d];
            int nj = cj + dy[d];

            if (ni < 0 || ni >= img.rows || nj < 0 || nj >= img.cols)
                continue;

            // weak pixel reached via a strong one -> promote it and continue BFS from it
            if (result.at<uchar>(ni, nj) == 128) {
                result.at<uchar>(ni, nj) = 255;
                bfsQueue.push({ni, nj});
            }
        }
    }

    // Pass 3: any remaining weak pixel was never connected to a strong one -> noise, remove it
    for (int i = 0; i < img.rows; i++)
        for (int j = 0; j < img.cols; j++)
            if (result.at<uchar>(i, j) == 128)
                result.at<uchar>(i, j) = 0;

    return result;
}

// ================= Handmade Canny =================
cv::Mat CannyDetector::cannyHandmade(const Mat& image, int low, int high) {
    // Step 1: Gaussian Blur — smooth the image to remove noise before any gradient calculation
    // 5x5 kernel with sigma=1.4 are the standard values used in Canny
    Mat blurred;
    GaussianBlur(image, blurred, Size(5, 5), 1.4);

    // Step 2: Gradient Calculation (Sobel)
    Mat mag, angle;
    sobelGradients(blurred, mag, angle);

    // Step 3: Non-Maximum Suppression
    Mat suppressed = nonMaxSuppression(mag, angle);

    // Steps 4+5: Double Thresholding + Hysteresis
    return hysteresis(suppressed, low, high);
}

// ================= Normalize =================
cv::Mat CannyDetector::normalize(const Mat& img) {
    Mat result;
    normalize(img, result, 0, 255, NORM_MINMAX);
    result.convertTo(result, CV_8U);
    return result;
}