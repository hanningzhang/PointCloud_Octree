#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

#include "Octree.h"
using namespace std;

#include <filesystem>
namespace fs = filesystem;

const size_t max_concurrent_tasks = 30;     // concurrency control for building octree

Bounds computeBoundingBoxFromSingleCSV(const string& filename) {
    Bounds bounds;
    bool firstPoint = true;

    ifstream file(filename);
    string line;

    if (!getline(file, line)) { // Skip header
        cerr << "Error reading the file or the file is empty: " << filename << endl;
        return bounds; // Return empty bounds if file is empty or can't be read
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
        } else {
            bounds.update(Point(x, y, z));
        }
    }
    return bounds;
}


// Single file processing
void processFileAndInsertPoints(const string& filename, Octree& octree, mutex& octreeMutex) {
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

        // Synchronize access to the octree
        lock_guard<mutex> guard(octreeMutex);
        octree.insert(point);
    }
}

void buildOctreeFromCSV(const vector<string>& filenames, Octree& octree) {
    mutex octreeMutex;
    vector<future<void>> futures;

    for (const auto& filename : filenames) {
        // Check if reached the maximum number of concurrent tasks
        if (futures.size() >= max_concurrent_tasks) {
            // Wait for at least one task to complete
            bool taskCompleted = false;
            while (!taskCompleted) {
                for (auto it = futures.begin(); it != futures.end(); ) {
                    auto& fut = *it;
                    if (fut.wait_for(chrono::seconds(0)) == future_status::ready) {
                        fut.get();      // Get the result to clear any stored exception
                        it = futures.erase(it); // Remove the completed future
                        taskCompleted = true;
                        break;      // Break the loop as we only need one task to complete
                    } else {
                        it++;
                    }
                }
            }
        }

        futures.push_back(async(launch::async, processFileAndInsertPoints, filename, ref(octree), ref(octreeMutex)));
    }

    for (auto& fut : futures) {
        fut.get();      // waits for the tasks to finish
    }
}



int main() {
    string choice;
    cout << "Please select input file by folder name(0) or path(1):" << endl;
    getline(cin, choice);

    vector<string> filenames;   // Pre-allocate for optimization

    if (choice == "0") {
        string folderPath;
        cout << "Enter the path to the folder containing CSV files:" << endl;
        getline(cin, folderPath);

        for (auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.path().extension() == ".csv") {
                filenames.push_back(entry.path().string());
            }
        }
    }

    else if (choice == "1") {
        string filePath;
        while (true) {
            cout << "Enter CSV file paths (Enter 'done' when finished):" << endl;
            getline(cin, filePath);
            if (filePath == "done") { 
                break;
            }
            filenames.push_back(filePath);
        }
    }


    if (filenames.empty()) {
        cerr << "No filenames provided." << endl;
        return 1; 
    }


    vector<future<Bounds>> futureBounds;
    // Launch a task for each file
    for (const auto& filename : filenames) {
        futureBounds.push_back(async(launch::async, computeBoundingBoxFromSingleCSV, filename));
    }
    Bounds bounds;
    bool firstBounds = true;

    // Wait for all tasks to complete and merge the results
    for (auto& fb : futureBounds) {
        Bounds b = fb.get();    // Block until the future is ready
        if (firstBounds) {
            bounds = b;
            firstBounds = false;
        } else {
            bounds.update(b.min);
            bounds.update(b.max);
        }
    }

    /*
    
    // Normal Processing
    
    // Variable settings
    int maxDepth = 7;
    int maxPointsPerNode = 8000;
    Octree octree(bounds, maxDepth, maxPointsPerNode);

    buildOctreeFromCSV(filenames, octree);

    octree.visualize("Octree Structure 1");


    // Trim octree
    octree.trim(maxDepth * (2/3));

    octree.visualize("Octree Structure 2");

    

    octree.buildRtrees();

    // Range query
    Bounds queryRange;
    queryRange.min = Point(bounds.getCenter().x - 15, bounds.getCenter().y - 15, bounds.getCenter().z - 15); 
    queryRange.max = Point(bounds.getCenter().x + 15, bounds.getCenter().y + 15, bounds.getCenter().z + 15);  
    vector<Point> queryResults;

    octree.executeRangeQuery(queryRange, queryResults);

    cout << "Query Result: " << endl;
    for (const Point& point : queryResults) {
        cout << "Point: " << point.x << " " << point.y << " " << point.z << endl;
    }

    
    */

    

    // Code for testing the time for query and construrction

    // Variable settings arrays
    vector<int> multimMaxDepths = {7};
    vector<int> multiMaxPointsPerNodes = {4000, 6000, 8000};

    // Open CSV file for output in append mode
    ofstream csvFile("octree_timing_results.csv", ios_base::app); 
    csvFile << "MaxDepth,MaxPointsPerNode,ConstructionTime(ms),RangeQueryTime(ms)\n";

    int fixed = 0;
    for (int n=0; n<1; n++) {
        for (int i=0; i<multiMaxPointsPerNodes.size(); i++) {

            Octree octree(bounds, multimMaxDepths[fixed], multiMaxPointsPerNodes[i]);

            // Measure construction time
            auto start1 = chrono::high_resolution_clock::now();
            buildOctreeFromCSV(filenames, octree);
            octree.trim(multimMaxDepths[fixed]/2);
            octree.buildRtrees();
            auto stop1 = chrono::high_resolution_clock::now();

            // Measure range query time
            Bounds queryRange;
            vector<Point> queryResults;
            int searchSize = 50;
            queryRange.min = Point(bounds.getCenter().x - searchSize, bounds.getCenter().y - searchSize, bounds.getCenter().z - searchSize); 
            queryRange.max = Point(bounds.getCenter().x + searchSize, bounds.getCenter().y + searchSize, bounds.getCenter().z + searchSize);  


            auto start2 = chrono::high_resolution_clock::now();
            octree.executeRangeQuery(queryRange, queryResults);
            auto stop2 = chrono::high_resolution_clock::now();

            // Calculate durations
            auto constructionDuration = chrono::duration_cast<chrono::milliseconds>(stop1 - start1);
            auto queryDuration = chrono::duration_cast<chrono::milliseconds>(stop2 - start2);

            // Write to CSV
            csvFile << multimMaxDepths[fixed] << "," << multiMaxPointsPerNodes[i] << "," << constructionDuration.count() << "," << queryDuration.count() << "\n";
        }
    }
    csvFile.close();

    

    return 0;
}

