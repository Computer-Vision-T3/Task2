#ifndef IMAGESTATEMANAGER_H
#define IMAGESTATEMANAGER_H

#include <opencv2/opencv.hpp>

class ImageStateManager {
public:
    ImageStateManager();
    void setOriginalImage(const cv::Mat& image);
    void setOutputImage(const cv::Mat& image);
    
    cv::Mat getNextInput();
    cv::Mat getOriginalImage() const;
    void triggerClear();

private:
    cv::Mat originalImage;
    cv::Mat currentOutput;
    bool clearFlag;
};

#endif // IMAGESTATEMANAGER_H