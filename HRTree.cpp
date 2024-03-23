#include "RTreePoints.h"
#include <map>


class RTreeNode {
public: 
    float time;
    RTreePoints* tree;
    RTreeNode(vector<Point> points): time(points.front().time) {
        this->tree = new RTreePoints;
        RInsert(this->tree, points);
    }
    ~RTreeNode(){
        delete this->tree;
        this->tree = nullptr;
    }
    // void HRTreeNodeInsert(Point point){
    //     RInsertPoint(this->tree, point);
    // }
    void HRSearch(std::vector<Point>& results, Bounds& queryRange){
        RSearch(this->tree, results, queryRange);
        return;
    }
    void print_tree_node(){
        cout<< "Tree node with the time "<< this->time<<endl;
        auto list = tree->ListTree();
        int counter = 0;
        for (auto aabb : list) {
        cout << "TreeList [" << counter++ << "]: "
            << aabb.m_min[0] << ", "
            << aabb.m_min[1] << ", "
            << aabb.m_min[2] << "; "
            << aabb.m_max[0] << ", "
            << aabb.m_max[1] << ", "
            << aabb.m_max[2] << endl;
        }
        return;
    }
};

bool compareByAttribute(RTreeNode& obj1, RTreeNode& obj2) {
    return obj1.time < obj2.time;
}
bool compareByPoints(Point& obj1, Point& obj2) {
    return obj1.time < obj2.time;
}

class HRTree {
private:
    std::map<float, RTreeNode*> trees;
public: 

    HRTree(vector<Point> points){
        if (points.empty()){
            return;
        }
        std::sort(points.begin(), points.end(), compareByPoints);
         // Group the elements based on 'attribute'
        std::map<float, std::vector<Point>> groupedMap;
        for (const Point& point : points) {
            groupedMap[point.time].push_back(point);
        }
        for (const auto& entry : groupedMap) {
            // cout<< entry.first;
            this->trees[entry.first] = new RTreeNode(groupedMap[entry.first]);
        }
        return;
    }
    ~HRTree() {
        this->trees.clear();
    }
    void print_tree(){
        for (auto& entry : this->trees) {
            entry.second->print_tree_node();
        }
        return;
    }
    void HRTreeSearch(float start, float end, std::vector<Point>& results, Bounds& queryRange){
        for (auto& entry : this->trees) {
            if (entry.first >= start && entry.first <= end){
                entry.second->HRSearch(results, queryRange);
            }
        }
        return;
    }
};


int main(){
    vector<Point> points; 
    Point tmp = Point(1.1, 2, 3, 1.1, 1.1, 1.1, "", 1.1);
    points.push_back(tmp);
    tmp = Point(1.2, 2.5, 3.1, 1.2, 1.2, 1.2, "", 1.2);
    points.push_back(tmp);
    tmp = Point(1.3,2.2,3.3,1.3 ,1.3, 1.3, "", 1.3);
    points.push_back(tmp);
    // for (Point point : points){
    //     cout<< point.time;
    // }
    HRTree* tree = new HRTree(points);
    // tree->print_tree();
    std::vector<Point> results;
    Point min = Point(0,0,0);
    Point max = Point(15,15,15);
    Bounds bound = Bounds(min, max);
    tree->HRTreeSearch(1.2, 1.2, results, bound);
    for (Point point : results) point.print_point();




    return 0;
}