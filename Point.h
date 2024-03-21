#pragma once
#include <string>
#include <iostream>

using namespace std;

// Define a point structure for standardized data points
struct Point {
    float x, y, z;              // Coordinates
    int r, g, b;                // RGB color data combined into a single integer
    string source;              // Record source file
    float arr[3] = {0.,0.,0.};
    float time;
    Point() : x(0), y(0), z(0), r(-1), g(-1), b(-1), source(""), time(-1) {}

    Point(float x, float y, float z, int r = -1, int g = -1, int b = -1, string source = "", float time = -1) : x(x), y(y), z(z), r(r), g(g), b(b), time(time) {
        this->source = source;
        
        this->arr[0] = x;
        this->arr[1] = y;
        this->arr[2] = z;
    }
    void print_point(){
        if (time==-1){
            cout << "x = " <<x << " y = " << y << " z = " << z << " r = " << r << " g = " << g << " b = " << b << endl;
        }
        else{
            cout << "x = " <<x << " y = " << y << " z = " << z << " time = " << time << endl;
        }
        
    }
};

