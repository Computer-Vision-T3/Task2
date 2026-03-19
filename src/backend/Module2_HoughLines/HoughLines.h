#ifndef HOUGHLINES_H
#define HOUGHLINES_H

#include <opencv2/opencv.hpp>

class HoughLines {
public:
    // Required function signature per your project architecture
    static cv::Mat detect(cv::Mat img, int threshold);
};

#endif
