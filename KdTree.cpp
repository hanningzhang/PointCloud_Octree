#include "KdTree.h"
#include "Point.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

KdTree::KdTree() : root(nullptr) {};

// Build tree from root
void KdTree::buildTree(vector<Point> &points) { 
    root = buildTreeRecursive(points, 0); 
}


// Retrieves the k nearest neighbors; returns them in order from nearest to farthest
vector<Point> KdTree::kNearestNeighbors(Point& queryPoint, int k) {
    priority_queue<pair<double, Point>, vector<pair<double, Point>>, ComparePoint> nearestNeighbors;

    kNearestNeighborsRecursive(root, queryPoint, k, nearestNeighbors, 0);

    vector<Point> result;
    while (!nearestNeighbors.empty()) {
        result.push_back(nearestNeighbors.top().second); // Extract the Point from the pair
        nearestNeighbors.pop();
    }
    reverse(result.begin(), result.end()); // Ensure nearest to farthest order
    return result;
}

// Perform Range Query
void KdTree::rangeQuery(std::vector<Point>& results, Bounds& queryRange){
    if (root == nullptr) {
        return;
    }

    // Check if the current node is within the range
    if (queryRange.contains(root->point)) {
        results.push_back(root->point);
    }

    // Check if the left subtree is within the range
    if (root->left != nullptr) {
        recursiveRangeQuery(root->left, results,queryRange);
    }

    // Check if the right subtree is within the range
    if (root->right != nullptr) {
        recursiveRangeQuery(root->right, results,queryRange);
    }
}
void KdTree::recursiveRangeQuery(Node* currentNode, std::vector<Point>& results, Bounds& queryRange){
    if (currentNode == nullptr) {
        return;
    }

    // Check if the current node is within the range
    if (queryRange.contains(currentNode->point)) {
        results.push_back(currentNode->point);
    }

    // Check if the left subtree is within the range
    if (currentNode->left != nullptr) {
        recursiveRangeQuery(currentNode->left, results,queryRange);
    }

    // Check if the right subtree is within the range
    if (currentNode->right != nullptr) {
        recursiveRangeQuery(currentNode->right, results,queryRange);
    }
}


// Extend tree nodes
KdTree::Node* KdTree::buildTreeRecursive(vector<Point> &points, int depth) {
    if (points.empty()) {
        return nullptr;
    }

    int axis = depth % 3; 

    // Sort points based on the current axis
    sort(points.begin(), points.end(), [axis](const Point& a, const Point& b) {
        switch (axis) {         // Sort based on selected axis
            case 0: return a.x < b.x;
            case 1: return a.y < b.y;
            case 2: return a.z < b.z;
            // case 3: return a.r < b.r;
            // case 4: return a.g < b.g;
            // case 5: return a.b < b.b;
            default: return false; // Should never be reached
        }
    });

    // Pick out the current split node
    int median = points.size() / 2;
    Node* medianPoint = new Node(points[median]);

    // Create non-const vectors for recursive calls
    vector<Point> leftPoints(points.begin(), points.begin() + median);
    vector<Point> rightPoints(points.begin() + median + 1, points.end());

    medianPoint->left = buildTreeRecursive(leftPoints, depth + 1);
    medianPoint->right = buildTreeRecursive(rightPoints, depth + 1);

    return medianPoint;
}



void KdTree::kNearestNeighborsRecursive(Node* currentNode, Point& queryPoint, int k,
                                        priority_queue<pair<double, Point>, vector<pair<double, Point>>, ComparePoint>& nearestNeighbors, int depth) {
    if (currentNode == nullptr) {
        return;
    }

    int axis = depth % 3; // Now includes RGB in axis selection

    // Calculate squared distance (euclidean)
    double squaredDistance = pow(currentNode->point.x - queryPoint.x, 2) +
                             pow(currentNode->point.y - queryPoint.y, 2) +
                             pow(currentNode->point.z - queryPoint.z, 2);

    nearestNeighbors.push({squaredDistance, currentNode->point});


    // If there are fewer than k elements in the queue or the current distance is less than the max distance in the queue
    if (nearestNeighbors.size() < k || squaredDistance < nearestNeighbors.top().first) {
        nearestNeighbors.push({squaredDistance, currentNode->point}); // Add current node to the priority queue

        // If the queue has more than k elements, remove the furthest one
        if (nearestNeighbors.size() > k) {
            nearestNeighbors.pop();
        }
    }


    // Decide which subtree to explore based on the axis
    Node* nextNode = nullptr;
    Node* otherNode = nullptr;

    bool goLeft = false;
    switch (axis) {
        case 0: goLeft = queryPoint.x < currentNode->point.x; break;
        case 1: goLeft = queryPoint.y < currentNode->point.y; break;
        case 2: goLeft = queryPoint.z < currentNode->point.z; break;
        // case 3: goLeft = queryPoint.r < currentNode->point.r; break;
        // case 4: goLeft = queryPoint.g < currentNode->point.g; break;
        // case 5: goLeft = queryPoint.b < currentNode->point.b; break;
    }
    nextNode = goLeft ? currentNode->left : currentNode->right;
    otherNode = goLeft ? currentNode->right : currentNode->left;

    kNearestNeighborsRecursive(nextNode, queryPoint, k, nearestNeighbors, depth + 1);

    // Check if we need to explore the other side
    if (nearestNeighbors.size() < k || squaredDistance < nearestNeighbors.top().first) {
        kNearestNeighborsRecursive(otherNode, queryPoint, k, nearestNeighbors, depth + 1);
    }
}

