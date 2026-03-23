#ifndef HOUGHELLIPSES_H
#define HOUGHELLIPSES_H

#include <opencv2/opencv.hpp>

class HoughEllipses {
public:
    // Added cv::Mat canvas to the signature
    static cv::Mat detect(cv::Mat src, cv::Mat canvas, 
                          int minMajor = 20, 
                          int maxMajor = 0, 
                          int minVotes = 10);
};

#endif
