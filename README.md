# 👁️ Vision Studio - Task 2

This repository contains the C++ Qt/OpenCV implementation for Task 2: Hough Transforms and Active Contours (Snakes). 

## 🏗️ Architecture & Team Assignments

The application is strictly divided into an isolated Frontend and Backend. **Do not modify the `src/frontend/` folder.** All algorithmic work must be done inside your assigned `src/backend/` module.

* **Module 1: Canny Edge Detector (From Scratch)**
  * **Assigned to:** Member 1
  * **Path:** `src/backend/Module1_CannyEdge/`
  * **Expected Function:** `static cv::Mat apply(cv::Mat img, int lowThresh, int highThresh);`

* **Module 2: Hough Transform (Lines)**
  * **Assigned to:** Member 2
  * **Path:** `src/backend/Module2_HoughLines/`
  * **Expected Function:** `static cv::Mat detect(cv::Mat img, int threshold);`

* **Module 3: Hough Transform (Circles)**
  * **Assigned to:** Member 3
  * **Path:** `src/backend/Module3_HoughCircles/`
  * **Expected Function:** `static cv::Mat detect(cv::Mat img, int minR, int maxR, int threshold);`

* **Module 4: Hough Transform (Ellipses)**
  * **Assigned to:** Member 4
  * **Path:** `src/backend/Module4_HoughEllipses/`
  * **Expected Function:** `static cv::Mat detect(cv::Mat img);`

* **Module 5: Active Contours (Greedy Snake) & Analytics**
  * **Assigned to:** Zeyad
  * **Path:** `src/backend/Module5_ActiveContours/`
  * **Expected Function 1:** `static std::vector<cv::Point> evolve(cv::Mat& img, float alpha, float beta, float gamma, int iter);`
  * **Expected Function 2:** `static QString generateReport(const std::vector<cv::Point>& contour);`

---

## 🚀 How to Build & Run

### 1. Generate the Build
```bash
mkdir build
cd build
cmake ..