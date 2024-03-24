#pragma once

#include <vector>
#include <queue>
#include "Point.h"
#include "ComparePoint.h"
#include "Bound.h"
using namespace std;


class KdTree {
    public:
        struct Node {
            Point point; 
            Node* left;
            Node* right;
            
            Node(Point p) : point(p), left(nullptr), right(nullptr) {}
        };


        KdTree();

        ~KdTree() {
            destroyTree(root);
        }

        // Disable copy constructor and copy assignment operator
        KdTree(const KdTree&) = delete;
        KdTree& operator=(const KdTree&) = delete;

        // Define move constructor and move assignment operator
        KdTree(KdTree&& other) noexcept : root(other.root) {
            other.root = nullptr;
        }

        KdTree& operator=(KdTree&& other) noexcept {
            if (this != &other) {
                destroyTree(root);
                root = other.root;
                other.root = nullptr;
            }
            return *this;
        }

        // Function to build the KdTree
        void buildTree(vector<Point>& points);

        // Function to perform k-NN search
        vector<Point> kNearestNeighbors(Point& queryPoint, int k);

        // Function to perform range query
        void rangeQuery(std::vector<Point>& results, Bounds& queryRange);

    private:
        Node* root;

        void destroyTree(Node* node) {
            if (node) {
                destroyTree(node->left);
                destroyTree(node->right);
                delete node;
            }
        }


        Node* buildTreeRecursive (vector<Point> &input, int depth);

        // Helper function for k-NN search
        void kNearestNeighborsRecursive(Node* currentNode, Point& queryPoint, int k,
                                    priority_queue<pair<double, Point>, vector<pair<double, Point>>, ComparePoint>& nearestNeighbors, int depth);

        void recursiveRangeQuery(Node* currentNode, std::vector<Point>& results, Bounds& queryRange);
};