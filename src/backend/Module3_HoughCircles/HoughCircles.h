#ifndef HOUGHCIRCLES_H
#define HOUGHCIRCLES_H

#include <opencv2/opencv.hpp>
#include <vector>

struct DetectedCircle {
    int x, y, radius, votes;
};

class HoughCircles {
public:
    // Added cv::Mat canvas to the signature
    static cv::Mat detect(cv::Mat src, cv::Mat canvas, int minR, int maxR, int threshold);
};

#endif
