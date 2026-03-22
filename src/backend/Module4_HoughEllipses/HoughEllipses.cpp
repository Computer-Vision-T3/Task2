#include "HoughEllipses.h"
#include <vector>
#include <cmath>
#include <algorithm>

cv::Mat HoughEllipses::detect(cv::Mat img, int minMajor, int maxMajor, int minVotes, int cannyLow, int cannyHigh)
{
    // 1. Create a color copy for drawing output
    cv::Mat outputImg = img.clone();
    if (outputImg.channels() == 1)
    {
        cv::cvtColor(outputImg, outputImg, cv::COLOR_GRAY2BGR);
    }

    // 2. Convert to Grayscale
    cv::Mat gray;
    if (img.channels() == 3)
    {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = img.clone();
    }

    // 3. Blur and Canny Edge Detection using parameters from the .h file
    cv::Mat blurred, edges;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Canny(blurred, edges, cannyLow, cannyHigh);

    // 4. Extract all edge pixels into a list
    std::vector<cv::Point> edgePoints;
    for (int y = 0; y < edges.rows; y++)
    {
        for (int x = 0; x < edges.cols; x++)
        {
            if (edges.at<uchar>(y, x) > 0)
            {
                edgePoints.push_back(cv::Point(x, y));
            }
        }
    }

    // 5. SUBSAMPLING (CRITICAL FOR PERFORMANCE)
    // The Xie-Ji algorithm is incredibly slow (O(N^3)). We must reduce the points
    // to prevent the app from freezing indefinitely. We limit it to ~300 points.
    std::vector<cv::Point> sampledEdges;
    int step = std::max(1, (int)(edgePoints.size() / 300));
    for (size_t i = 0; i < edgePoints.size(); i += step)
    {
        sampledEdges.push_back(edgePoints[i]);
    }

    // 6. Handle the default maxMajor value (0 means half the image size)
    if (maxMajor <= 0)
    {
        maxMajor = std::min(img.cols, img.rows) / 2;
    }

    // Structure to hold our surviving ellipses
    struct EllipseData
    {
        cv::Point2f center;
        float a, b, angle;
    };
    std::vector<EllipseData> detectedEllipses;

    // 7. THE XIE-JI ALGORITHM
    int N = sampledEdges.size();

    // Loop through every pair of points (Assume they are the major axis endpoints)
    for (int i = 0; i < N - 1; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            cv::Point2f p1 = sampledEdges[i];
            cv::Point2f p2 = sampledEdges[j];

            // Calculate distance between points (which equals 2 * major axis 'a')
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float a = dist / 2.0f;

            // Reject if the axis isn't the size we want
            if (a < minMajor || a > maxMajor)
                continue;

            // Calculate center and angle
            cv::Point2f center((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
            float angle = std::atan2(dy, dx);

            // 1D Accumulator for the minor axis 'b' (Max 'b' is 'a', making a circle)
            int maxB = std::ceil(a);
            std::vector<int> accumulator(maxB + 1, 0);

            // Loop through all OTHER points to vote on 'b'
            for (int k = 0; k < N; k++)
            {
                if (k == i || k == j)
                    continue;
                cv::Point2f p3 = sampledEdges[k];

                // Translate point to center and rotate it to match the axis
                float px = p3.x - center.x;
                float py = p3.y - center.y;
                float u = px * std::cos(angle) + py * std::sin(angle);
                float v = -px * std::sin(angle) + py * std::cos(angle);

                // Point must be inside the bounds of the major axis
                if (std::abs(u) >= a)
                    continue;

                // Xie-Ji Geometric Formula to calculate the minor axis 'b'
                float b_sq = (a * a * v * v) / (a * a - u * u);

                if (b_sq > 0)
                {
                    int b = std::round(std::sqrt(b_sq));
                    // If it's a valid 'b', cast a vote!
                    if (b > 0 && b <= maxB)
                    {
                        accumulator[b]++;
                    }
                }
            }

            // Find the highest voted 'b'
            int maxVote = 0;
            int bestB = 0;
            for (int b = 1; b <= maxB; b++)
            {
                if (accumulator[b] > maxVote)
                {
                    maxVote = accumulator[b];
                    bestB = b;
                }
            }

            // If it received enough votes, and isn't just a flat line, keep it!
            if (maxVote >= minVotes && bestB >= 5)
            {
                // Convert angle back to degrees for drawing
                float angleDegrees = angle * 180.0f / (float)CV_PI;
                detectedEllipses.push_back({center, a, (float)bestB, angleDegrees});
            }
        }
    }

    // 8. Draw the detected ellipses
    for (const auto &el : detectedEllipses)
    {
        cv::ellipse(outputImg, el.center, cv::Size(el.a, el.b), el.angle, 0, 360, cv::Scalar(0, 255, 0), 2);
    }

    return outputImg;
}