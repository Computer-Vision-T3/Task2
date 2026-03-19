#include "HoughLines.h"
#include <cmath>
#include <vector>

cv::Mat HoughLines::detect(cv::Mat img, int threshold) {
    if (img.empty()) return img;

    // 1. Optimized Pre-processing
    cv::Mat gray, edges;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else gray = img.clone();

    // Use a smaller blur (3x3) to keep thin vertical lines sharp
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 1.0);
    
    // Balanced Canny thresholds to catch road markings but ignore grass texture
    cv::Canny(gray, edges, 50, 150); 

    cv::Mat output;
    if (img.channels() == 1) cv::cvtColor(img, output, cv::COLOR_GRAY2BGR);
    else img.copyTo(output);

    int rows = edges.rows;
    int cols = edges.cols;
    double diagonal = std::sqrt(rows * rows + cols * cols);
    int numThetas = 180;
    int numRhos = static_cast<int>(2 * diagonal);
    
    std::vector<std::vector<int>> accumulator(numRhos, std::vector<int>(numThetas, 0));

    // Precompute Trigo for 0 to 180 degrees
    std::vector<double> sinT(numThetas), cosT(numThetas);
    for (int t = 0; t < numThetas; t++) {
        double rad = (t * CV_PI) / 180.0;
        sinT[t] = std::sin(rad); 
        cosT[t] = std::cos(rad);
    }

    // 2. Voting
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

    // 3. Peak Detection with Angle-Aware Suppression
    for (int r = 2; r < numRhos - 2; r++) {
        for (int t = 0; t < numThetas; t++) {
            int currentVotes = accumulator[r][t];
            
            // Adjust threshold slightly based on angle 
            // (Vertical/Inclined lines often have fewer votes than the horizon)
            int localThreshold = threshold;
            if (t < 45 || t > 135) localThreshold = static_cast<int>(threshold * 0.7); // More sensitive to vertical

            if (currentVotes >= localThreshold) {
                bool isPeak = true;
                // Larger 5x5 check to prevent thick bundles of lines
                for (int dr = -2; dr <= 2; dr++) {
                    for (int dt = -2; dt <= 2; dt++) {
                        if (dr == 0 && dt == 0) continue;
                        int nt = (t + dt + numThetas) % numThetas; // Wrap angles
                        if (accumulator[r + dr][nt] > currentVotes) {
                            isPeak = false;
                            break;
                        }
                    }
                    if (!isPeak) break;
                }

                if (isPeak) {
                    double rho = r - diagonal;
                    double theta = (t * CV_PI) / 180.0;
                    
                    double a = std::cos(theta), b = std::sin(theta);
                    double x0 = a * rho, y0 = b * rho;

                    cv::Point p1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
                    cv::Point p2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

                    // Draw clean lines
                    cv::line(output, p1, p2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
                }
            }
        }
    }

    return output;
}
