#include "ShapeAnalytics.h"
#include <cmath>

QString ShapeAnalytics::generateReport(const std::vector<cv::Point>& contour) {
    if (contour.size() < 3) return "Not enough points to form a shape.";
    
    double area = calculateArea(contour);
    double perimeter = calculatePerimeter(contour);
    QString chainCode = computeChainCode(contour);

    return QString(R"(
        <style>
            body { font-family: 'DM Sans', sans-serif; color: #2C2825; margin: 0; padding: 0; }
            .card { background: #FFFFFF; border: 1px solid #E6E0F7; border-radius: 12px; padding: 14px 14px; margin-bottom: 10px; }
            h3 { font-size: 15px; font-weight: 800; color: #2C2825; margin: 0 0 4px; }
            p { font-size: 12px; color: #7A7268; line-height: 1.6; margin: 8px 0 0; }
            .badge { display: inline-block; background: #EDE8FF; color: #5B4FCF; border-radius: 6px; padding: 3px 9px; font-size: 11px; font-weight: 800; }
        </style>
        <div class='card'>
            <h3>Shape Analytics</h3>
            <p>Metrics extracted from the finalized contour.</p>
        </div>
        <div class='card'>
            <p class='badge'>Perimeter: %1 px</p><br>
            <p class='badge'>Area: %2 px²</p>
        </div>
        <div class='card'>
            <h3>Chain Code (8-Directional)</h3>
            <p style="word-wrap: break-word;">%3</p>
        </div>
    )").arg(perimeter, 0, 'f', 2).arg(area, 0, 'f', 2).arg(chainCode);
}

double ShapeAnalytics::calculateArea(const std::vector<cv::Point>& contour) {
    // Calculates area using the Shoelace Formula
    double area = 0.0;
    int n = contour.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += (contour[i].x * contour[j].y) - (contour[j].x * contour[i].y);
    }
    return std::abs(area) / 2.0;
}

double ShapeAnalytics::calculatePerimeter(const std::vector<cv::Point>& contour) {
    // Calculates perimeter via the sum of Euclidean distances between points
    double perimeter = 0.0;
    int n = contour.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        double dx = contour[j].x - contour[i].x;
        double dy = contour[j].y - contour[i].y;
        perimeter += std::sqrt(dx*dx + dy*dy);
    }
    return perimeter;
}

QString ShapeAnalytics::computeChainCode(const std::vector<cv::Point>& contour) {
    QString chain = "";
    int n = contour.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        double dx = contour[j].x - contour[i].x;
        double dy = contour[j].y - contour[i].y; 
        
        // Freeman Chain Code standard mapping:
        // Because image Y-coordinates go DOWN instead of UP, we invert dy (-dy) 
        // to map properly to standard Cartesian degrees.
        double angle = std::atan2(-dy, dx) * 180.0 / CV_PI;
        
        // Keep angle positive
        if (angle < 0) angle += 360.0;
        
        // Map the 360 degrees into 8 discrete "slices" (Octants)
        int direction = (static_cast<int>(angle + 22.5) / 45) % 8;
        
        chain += QString::number(direction);
    }
    return chain;
}