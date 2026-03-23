#ifndef HOUGHLINES_H
#define HOUGHLINES_H

#include <opencv2/opencv.hpp>

class HoughLines {
public:
    // Added cv::Mat canvas to the signature
    static cv::Mat detect(cv::Mat src, cv::Mat canvas, int threshold);
};

#endif
