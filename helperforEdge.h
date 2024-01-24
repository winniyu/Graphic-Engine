#include "include/GRect.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include <iostream>
#include <vector>
#include <algorithm>


struct Edge {
    float m, b, x0;
    int ymax, ymin, wind;//bottom, top
    Edge(GPoint top, GPoint bottom, int w){
        if(top.y > bottom.y){
            std::swap(top, bottom);
        }
        ymin = GRoundToInt(top.y);
        ymax = GRoundToInt(bottom.y);
        if(ymin == ymax){
            return;
        }
        m = static_cast<float>((top.x - bottom.x) / (top.y - bottom.y));
        b = static_cast<float>(top.x - m * top.y);
        x0 = static_cast<float>(m * (ymin - top.y + 0.5) + top.x);
        wind = w;
        }
    bool isValid(int y){
        if(y >= ymax || y < ymin){
            return false;
        }
        else{
            return true;
        }
    }
    int computeX(int y){
        return GRoundToInt(static_cast<float>(m * (y + 0.5) + b));
    }
    
};


void ClipEdges(GPoint p0, GPoint p1, const GIRect& rect, std::vector<Edge>& edges){
    int w = 1;
    if(p0.y>p1.y){
        std::swap(p0,p1);
        w=-1;
    }
    //top
    if(p1.y <= rect.top){
        return;
    }
    else if(p0.y <= rect.top){
        p0.x = p0.x + (p1.x - p0.x) * (rect.top - p0.y) / (p1.y - p0.y);
        p0.y = rect.top;    
    }
    //bottom
    if(p0.y >= rect.bottom){
        return;
    }
    else if(p1.y >= rect.bottom){
        p1.x = p1.x - (p1.x - p0.x) * (p1.y - rect.bottom) / (p1.y - p0.y);
        p1.y = rect.bottom;
    }
    
    if(p0.x>p1.x){
        std::swap(p0,p1);
    }
    //left
    if(p1.x <= rect.left){
        p0.x = rect.left;
        p1.x = rect.left;
    }
    else if(p0.x < rect.left){
        GPoint temp;
        temp.x = rect.left;
        temp.y = p0.y;
        p0.y = p0.y + (p1.y - p0.y) * (rect.left - p0.x) / (p1.x - p0.x);
        p0.x = rect.left;
        if(GRoundToInt(p0.y) != GRoundToInt(temp.y)){
        Edge edge(p0, temp, w);
        edges.push_back(edge);
        }
    }
    //right
    if(p0.x > rect.right){
        p0.x = rect.right;
        p1.x = rect.right;
    }
    else if(p1.x > rect.right){
        GPoint temp;
        temp.x = rect.right;
        temp.y = p1.y;
        p1.y = p1.y - (p1.y - p0.y) * (p1.x - rect.right) / (p1.x - p0.x);
        p1.x = rect.right;
        if(GRoundToInt(p1.y) != GRoundToInt(temp.y)){
        Edge edge(p1, temp, w);
        edges.push_back(edge);
        }
    }
    if(GRoundToInt(p0.y) != GRoundToInt(p1.y)){
        Edge edge(p0, p1, w);
        edges.push_back(edge);
    }
}

std::vector<Edge> findPolygonEdges(const GPoint points[], int count, const GIRect& rect){
    std::vector<Edge> finalEdges;
    finalEdges.reserve(count * 3);
    for (int i = 0; i < count; ++i) {
        GPoint start = points[i];
        GPoint end = points[(i + 1) % count];
        if(GRoundToInt(start.y) != GRoundToInt(end.y)){
            ClipEdges(start, end, rect, finalEdges);
        }
    }
    return finalEdges;
}
bool compareEdges(const Edge& e0, const Edge& e1){
        if (e0.ymin != e1.ymin) {
            return e0.ymin > e1.ymin;
        }
        if (e0.x0 != e1.x0){
            return e0.x0 > e1.x0;
        }
        return e0.m > e1.m;
    }

bool comparePath(const Edge& e0, const Edge& e1){
        if (e0.ymin != e1.ymin) {
            return e0.ymin < e1.ymin;
        }
        if (e0.x0 != e1.x0){
            return e0.x0 < e1.x0;
        }
        return e0.m < e1.m;
    }

bool compareX(const Edge& e0, const Edge& e1){
         return e0.x0 < e1.x0;
    }



