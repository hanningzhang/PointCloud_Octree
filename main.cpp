#include <iostream>
#include <fstream>
#include <sstream>

#include "Bound.h"
#include "Octree.h"
using namespace std;


Bounds computeBoundingBoxFromCSV(const vector<string>& filenames) {
    Bounds bounds;
    bool firstPoint = true;

    for (auto& filename : filenames) {
        ifstream file(filename);
        string line;

        // Skip the header
        if (!getline(file, line)) {
            cerr << "Error reading the file or the file is empty: " << filename << endl;
            continue;   // Skip to the next file
        }

        while (getline(file, line)) {
            stringstream ss(line);
            string value;
            vector<string> values;

            while (getline(ss, value, ',')) {
                values.push_back(value);
            }

            float x = stof(values[0]);
            float y = stof(values[1]);
            float z = stof(values[2]);

            if (firstPoint) {
                bounds.min = Point(x, y, z);
                bounds.max = Point(x, y, z);
                firstPoint = false;
            }
            else {
                bounds.update(Point(x, y, z));
            }
        }
    }
    return bounds;
}


void buildOctreeFromCSV(const vector<string>& filenames, Octree& octree) {
    for (auto& filename : filenames) {
        ifstream file(filename);
        string line;

        // Skip header
        getline(file, line);
        while (getline(file, line)) {
            stringstream ss(line);
            string value;
            vector<string> values;

            while (getline(ss, value, ',')) {
                values.push_back(value);
            }

            float x = stof(values[0]);
            float y = stof(values[1]);
            float z = stof(values[2]);
            int r = stoi(values[3]);
            int g = stoi(values[4]);
            int b = stoi(values[5]);
            string source = values[6];
            Point point(x, y, z, r, g, b, source);
            octree.insert(point);
        }
    }
}


int main() {
    vector<string> filenames;
    string filePath;

    while (true) {
        cout << "Enter CSV file paths (Enter 'done' when finished):" << endl;
        getline(cin, filePath);
        if (filePath == "done") { 
            break;
        }
        filenames.push_back(filePath);
    }

    if (filenames.empty()) {
        cerr << "No filenames provided." << endl;
        return 1; 
    }

    /*
    "Montreal-PointCloud/Montreal1.csv",
    "Montreal-PointCloud/Montreal2.csv"
    */

    // Compute the bounding box 
    Bounds bounds = computeBoundingBoxFromCSV(filenames);
    Point origin = bounds.getCenter();       
    float initialSize = bounds.getSize();   

    // Variable settings for 1GB CSV Point Cloud Data(Montreal)
    int maxDepth = 100;
    int maxPointsPerNode = 1000;
    int depthAdjustmentFactor = 20;

    // Build Octree
    Octree octree(origin, initialSize, maxDepth, maxPointsPerNode, depthAdjustmentFactor);
    buildOctreeFromCSV(filenames, octree);

    octree.visualize();

    return 0;
}

