#pragma once

#include <opencv2/opencv.hpp>

/**
 * Module 4 – Hough Ellipse Detector (from scratch)
 *
 * Algorithm:
 *   For every pair of edge points (p1, p2) that could be the two endpoints of
 *   the major axis, we iterate over all third edge points p3 and use the
 *   geometric constraint of an ellipse to vote for (cx, cy, a, b, tau).
 *   Candidates that accumulate enough votes are kept, non-maximum suppression
 *   removes duplicates, and the surviving ellipses are drawn on the image.
 *
 * Reference:
 *   Yonghong Xie & Qiang Ji, "A New Efficient Ellipse Detection Method",
 *   ICPR 2002.
 */
class HoughEllipses
{
public:
    /**
     * Detect ellipses in @p img and return a colour copy with the detected
     * ellipses superimposed.
     *
     * @param img        Input image (grayscale or colour).
     * @param minMajor   Minimum semi-major axis length in pixels (default 20).
     * @param maxMajor   Maximum semi-major axis length in pixels (default 0 →
     *                   half the shorter image dimension).
     * @param minVotes   Minimum accumulator votes to accept a candidate
     *                   (default 10).
     * @param cannyLow   Low threshold forwarded to the internal Canny step.
     * @param cannyHigh  High threshold forwarded to the internal Canny step.
     */
    static cv::Mat detect(cv::Mat img,
                          int minMajor = 20,
                          int maxMajor = 0,
                          int minVotes = 10,
                          int cannyLow = 50,
                          int cannyHigh = 150);
};