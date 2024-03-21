#include <vector>
#include <algorithm>
#include "Point.h"


struct Bounds {
    Point min;
    Point max;

    Bounds() : min(Point()), max(Point()) {}
    Bounds(const Point& minimum, const Point& maximum) : min(minimum), max(maximum) {}


    void update(const Point& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    Point getCenter() {
        return Point(
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        );
    }

    float getSize() {
        float maxX = max.x - min.x;
        float maxY = max.y - min.y;
        float maxZ = max.z - min.z;

        return std::max({maxX, maxY, maxZ});
    }

    bool intersects(const Bounds& other) {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    bool contains(const Point& point) {
        return (point.x >= min.x && point.x <= max.x) &&
               (point.y >= min.y && point.y <= max.y) &&
               (point.z >= min.z && point.z <= max.z);
    }
};


