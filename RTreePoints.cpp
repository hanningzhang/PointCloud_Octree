#include "RTree.h"
#include "Point.h" 
#include <vector>
#include <iostream>
#include "Bound.h"
typedef RTree<Point, float, 3> RTreePoints;
typedef Point ValueType;


void RInsert(RTreePoints* tree, vector<Point> points){
    if (points.size()==0){
        return;
    }
    for (Point point : points){
        // Point* pointer = new Point();
        tree->Insert(point.arr, point.arr, point);
    }
}

bool MySearchCallback(ValueType point)
{
//   point.print_point();
  return true; // keep going
}

void RSearch(RTreePoints* tree, std::vector<Point>& results, Bounds& queryRange){
    
    tree->Search(queryRange.min.arr, queryRange.max.arr, results, MySearchCallback);
    return;
}



int main(){
    RTreePoints* tree = new RTreePoints;
    vector<Point> points;
    Point tmp_0 = Point(1.1,2.2,3.3);
    Point tmp_1 = Point(0.1,2.5,3.1);
    Point tmp_2 = Point(1.2,2.2,3.3);
    points.push_back(tmp_0);
    points.push_back(tmp_1);
    points.push_back(tmp_2);

    RInsert(tree, points);
    // auto list = tree->ListTree();
    // int counter = 0;
    // for (auto aabb : list) {
    // cout << "TreeList [" << counter++ << "]: "
    //     << aabb.m_min[0] << ", "
    //     << aabb.m_min[1] << ", "
    //     << aabb.m_min[2] << "; "
    //     << aabb.m_max[0] << ", "
    //     << aabb.m_max[1] << ", "
    //     << aabb.m_max[2] << endl;
    // }
    std::vector<Point> results;
    Point min = Point(0,0,0);
    Point max = Point(15,15,15);
    Bounds bound = Bounds(min, max);
    RSearch(tree, results, bound);

    // float min[3] = {0., 0., 0.};
    // float max[3] = {15., 15., 15.};
    
    // int nhits = tree->Search(min, max, results, MySearchCallback);
    for (Point point : results){
        point.print_point();
    }
    return 0;
}
