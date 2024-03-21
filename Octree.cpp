#include <iostream>
#include <algorithm>

#include "Octree.h"

// R-tree functions
void RInsert(RTreePoints* tree, const vector<Point>& points){
    if (points.size()==0){
        return;
    }
    for (const Point& point : points){
        // Point* pointer = new Point();
        tree->Insert(point.arr, point.arr, point);
    }
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
}

bool MySearchCallback(ValueType point)
{
//   point.print_point();
    return true; // keep going
}

void RSearch(RTreePoints* tree, std::vector<Point>& results, Bounds& queryRange){
    auto list = tree->ListTree();
    int counter = 0;
    // for (auto aabb : list) {
    // cout << "TreeList [" << counter++ << "]: "
    //     << aabb.m_min[0] << ", "
    //     << aabb.m_min[1] << ", "
    //     << aabb.m_min[2] << "; "
    //     << aabb.m_max[0] << ", "
    //     << aabb.m_max[1] << ", "
    //     << aabb.m_max[2] << endl;
    // }
    // queryRange.min.print_point();
    // queryRange.max.print_point();
    // cout<<endl<<endl;
    tree->Search(queryRange.min.arr, queryRange.max.arr, results, MySearchCallback);
    // for (Point point : results){
    //     point.print_point();
    // }
    return;
}
// End of R-tree functions


int Octree::getOctant(const Point& origin, Point& point) {  
    int octant = 0;
    if (point.x >= origin.x) octant |= 4;       // The third bit (from the right, 0-indexed) of octant is set 
    if (point.y >= origin.y) octant |= 2;       // The second bit (from the right, 0-indexed) of octant is set 
    if (point.z >= origin.z) octant |= 1;       // The first bit (from the right, 0-indexed) of octant is set 
    return octant;      // 8 possible results from 3 bits 000 to 111 representing 8 octants
}

void Octree::insert(OctreeNode* node, Point& point, int depth) {
    if (node->isLeaf()) {
        if (node->points.size() < maxPointsPerNode || depth >= maxDepth) {
            node->points.push_back(point);
            return;
        }
        // Subdivide the node if it exceeds capacity and within depth limit
        for (Point p : node->points) {
            subdivideAndInsert(node, p, depth);     // Reinsert existing points in the current node
        }
        node->points.clear();
        subdivideAndInsert(node, point, depth);     // Insert target point
    }
    else {
        subdivideAndInsert(node, point, depth);
    }
}

void Octree::subdivideAndInsert(OctreeNode* node, Point& point, int depth) {
    int octant = getOctant(node->bound.getCenter(), point);
    
    if (node->children[octant] == nullptr) {
        Bounds childBounds = calculateChildBounds(node->bound, octant);
        node->children[octant] = new OctreeNode(childBounds);
    }
    insert(node->children[octant], point, depth + 1);
}

/* Not usable with R-tree
void Octree::visualizeNode(OctreeNode* node, int level, ofstream& outFile) {
    if (!node) return;

    for (int i=0; i<level; i++) outFile << "  ";   // Indentation

    if (node->isLeaf()) {
        outFile << "Level " << level << ": Leaf node with " << node->points.size() << " points\n";
    }
    else {
        outFile << "Level " << level << ": Internal node\n";
        for (int i=0; i<8; i++) {
            visualizeNode(node->children[i], level+1, outFile);
        }
    }
}
*/


void Octree::mergeUnderpopulatedNodes(OctreeNode* node, int depth, int startDepth) {
    if (!node || node->isLeaf()) return; 

    // bottom-up merge
    for (int i = 0; i < 8; i++) {
        mergeUnderpopulatedNodes(node->children[i], depth + 1, startDepth);
    }
    
    if (depth >= startDepth) {
        bool allChildrenAreLeaves = true;
        int totalPoints = 0;
        for (int i = 0; i < 8; i++) {   // check if children are all leaves
            if (node->children[i]) {
                if (!node->children[i]->isLeaf()) {
                    allChildrenAreLeaves = false;
                    break;  // break if one child is not leaf (already checked-exceed max)
                } else {
                    totalPoints += node->children[i]->points.size();
                }
            }
        }

        if (allChildrenAreLeaves && totalPoints <= maxPointsPerNode * 1.5) {    // if all children combined have less than 1.5*max threshold, merge
            vector<Point> mergedPoints;
            Bounds mergedBounds;
            for (int i = 0; i < 8; i++) {
                if (node->children[i]) {
                    mergedPoints.insert(mergedPoints.end(), node->children[i]->points.begin(), node->children[i]->points.end());
                    for (Point& point : node->children[i]->points) {
                        mergedBounds.update(point);
                    }
                    delete node->children[i];
                    node->children[i] = nullptr;
                }
            }
            node->points = mergedPoints;  
            node->bound = mergedBounds;
            node->convertToLeaf();  
        }
        
        else if (allChildrenAreLeaves) {    // if some children combined have less than max threshold, combine them
            // sort children
            vector<pair<int, int>> childPointCounts;
            childPointCounts.reserve(8);    // Reserve memory to avoid reallocations

            for (int i = 0; i < 8; i++) {
                if (node->children[i]) {
                    childPointCounts.emplace_back(node->children[i]->points.size(), i);;
                }
            }   
            sort(childPointCounts.begin(), childPointCounts.end()); // Sort the vector by point count in ascending order

            vector<Point> mergedPoints;
            vector<OctreeNode*> newLeafNodes;
            Bounds mergedBounds;

            for (const auto& [pointCount, index] : childPointCounts) {
                if (!mergedPoints.empty() && mergedPoints.size() + pointCount > maxPointsPerNode * 1.2) {    // Newly merged exceed the max limit
                    OctreeNode* newLeaf = new OctreeNode(mergedBounds);
                    newLeaf->points.swap(mergedPoints);  // Move the merged points to the new leaf node, use swap to save memory
                    newLeaf->convertToLeaf();
                    newLeafNodes.push_back(newLeaf);    // Add node
                    mergedBounds = Bounds();
                }

                auto& childPoints = node->children[index]->points;
                mergedPoints.insert(mergedPoints.end(),
                            make_move_iterator(childPoints.begin()),
                            make_move_iterator(childPoints.end()));     // Use move iterators to save memory
                for (const Point& point : childPoints) {
                    mergedBounds.update(point);
                }
                delete node->children[index]; 
                node->children[index] = nullptr;
            }

            // Handle remaining merged points
            if (!mergedPoints.empty()) {
                OctreeNode* newLeaf = new OctreeNode(mergedBounds);
                newLeaf->points.swap(mergedPoints);
                newLeaf->convertToLeaf();
                newLeafNodes.push_back(newLeaf);
            }

            // Reinsert any remaining children after the merged one
            for (size_t i = 0; i < newLeafNodes.size(); i++) {
                node->children[i] = newLeafNodes[i];
            }

            // Clear any remaining child pointers
            for (size_t i = newLeafNodes.size(); i < 8; i++) {
                node->children[i] = nullptr;
            }
        }
    }

}

void Octree::rangeQuery(Bounds& queryRange, vector<Point>& results, OctreeNode* node, int depth) {
    // Check if the current node's bounds intersect with the query range
    if (!node->bound.intersects(queryRange)) {
        return;
    }

    if (node->isLeaf()) {
        // If it's a leaf node, query the R-tree
        if (node->rtree) {
            vector<Point> rtreeResults;
            RSearch(node->rtree, rtreeResults, queryRange);
            results.insert(results.end(), rtreeResults.begin(), rtreeResults.end());
        } 
    } 
    else {
        for (int i = 0; i < 8; i++) {
            if (node->children[i]) {
                rangeQuery(queryRange, results, node->children[i], depth + 1);
            }
        }
    }
}


Bounds Octree::calculateChildBounds(Bounds& parentBounds, int octant) {
    Point center = parentBounds.getCenter();
    Point min = parentBounds.min;
    Point max = parentBounds.max;
    Point childMin, childMax;

    // Calculate min and max points for the child bounds based on the octant
    if (octant & 4) {
        childMin.x = center.x;
        childMax.x = max.x;
    } else {
        childMin.x = min.x;
        childMax.x = center.x;
    }
    if (octant & 2) {
        childMin.y = center.y;
        childMax.y = max.y;
    } else {
        childMin.y = min.y;
        childMax.y = center.y;
    }
    if (octant & 1) {
        childMin.z = center.z;
        childMax.z = max.z;
    } else {
        childMin.z = min.z;
        childMax.z = center.z;
    }

    return Bounds(childMin, childMax);
}

void Octree::initializeRTrees(OctreeNode* node) {
    if (node->isLeaf()) {
        node->rtree = new RTreePoints();
        RInsert(node->rtree, node->points);
    } 
    else {
        for (int i = 0; i < 8; i++) {
            if (node->children[i]) {
                initializeRTrees(node->children[i]);
            }
        }
    }
}
