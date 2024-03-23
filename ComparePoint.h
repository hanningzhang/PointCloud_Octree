#include "Point.h"

struct ComparePoint {
    bool operator()(const std::pair<double, Point>& a, const std::pair<double, Point>& b) const {
        return a.first < b.first;  // Compare based on the distance (double in pair) value
    }
};

