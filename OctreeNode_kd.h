#include <vector>
#include "Point.h"
#include "Bound.h"
#include "RTree.h"
#include "KdTree.h"

typedef RTree<Point, float, 3> RTreePoints;
typedef Point ValueType;

struct OctreeNode_kd {
    Bounds bound;                               // Bounding box of the node
    vector<Point> points;                       // Points contained in this node
    OctreeNode_kd* children[8] = {nullptr};        // Pointers to octants
    RTreePoints* rtree = nullptr;               // R-tree in this leaf node
    KdTree* kdtree = nullptr;

    OctreeNode_kd(const Bounds& b) : bound(b) {}

    ~OctreeNode_kd() {
        for (auto& child : children) {
            delete child;
            child = nullptr;
        }
        delete rtree; 
        rtree = nullptr;

        delete kdtree;
        kdtree = nullptr;
    }

    bool isLeaf() const {
        return children[0] == nullptr;
    }

    void convertToLeaf() {
        for (int i = 0; i < 8; i++) {
            if (children[i]) {
                delete children[i];
                children[i] = nullptr;
            }
        }
    }
};