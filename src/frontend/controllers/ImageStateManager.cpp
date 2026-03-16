#include "ImageStateManager.h"

ImageStateManager::ImageStateManager() : clearFlag(false) {}

void ImageStateManager::setOriginalImage(const cv::Mat& image) {
    originalImage = image.clone();
    currentOutput = image.clone();
    clearFlag = false;
}

void ImageStateManager::setOutputImage(const cv::Mat& image) {
    currentOutput = image.clone();
}

cv::Mat ImageStateManager::getNextInput() {
    if (clearFlag || currentOutput.empty()) {
        clearFlag = false; // Reset flag after using
        return originalImage.clone();
    }
    return currentOutput.clone();
}

cv::Mat ImageStateManager::getOriginalImage() const {
    return originalImage.clone();
}

void ImageStateManager::triggerClear() {
    clearFlag = true;
}