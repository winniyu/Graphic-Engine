#include "include/GFinal.h"
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GCanvas.h"
#include "include/GPath.h"
#include "myRadicalShader.h"
#include "myColorMatrixShader.h"







class MyFinal : public GFinal {
    std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius,
                                                    const GColor colors[], int count,
                                                    GShader::TileMode mode) override {
        return GCreateRadialGradientShader(center, radius, colors, count, mode);
    }
    std::unique_ptr<GShader> createColorMatrixShader(const GColorMatrix& colorMatrix,
                                                             GShader* realShader)override{
        return GCreateRadialGradientShader(colorMatrix, realShader);
    }

    GPath strokePolygon(const GPoint points[], int count, float width, bool isClosed)override{
        GPath* path = new GPath();
        if(count < 2){
            return *path;
        }
        float diameter = width;
        float radius = diameter / 2; 

        for(int i = 1; i < count; ++i){
            float angle = atan(-1 / ((points[i].y - points[i-1].y) / (points[i].x - points[i-1].x)));
            float dx = radius * cos(angle);
            float dy = radius * sin(angle);
            path -> addCircle(points[i-1], radius, GPath::Direction::kCW_Direction);
            path -> moveTo(GPoint{points[i-1].x + dx, points[i-1].y + dy});
            path -> lineTo(GPoint{points[i-1].x - dx, points[i-1].y - dy});
            path -> lineTo(GPoint{points[i].x - dx, points[i].y - dy});
            path -> lineTo(GPoint{points[i].x + dx, points[i].y + dy});
            path -> addCircle(points[i], radius, GPath::Direction::kCW_Direction);
        }
        if(isClosed){
            float angle = atan(-1 / ((points[count - 1].y - points[0].y) / (points[count - 1].x - points[0].x)));
            float dx = radius * cos(angle);
            float dy = radius * sin(angle);
            path -> addCircle(points[0], radius, GPath::Direction::kCW_Direction);
            path -> moveTo(GPoint{points[0].x + dx, points[0].y + dy});
            path -> lineTo(GPoint{points[0].x - dx, points[0].y - dy});
            path -> lineTo(GPoint{points[count - 1].x - dx, points[count - 1].y - dy});
            path -> lineTo(GPoint{points[count - 1].x + dx, points[count - 1].y + dy});
            path -> addCircle(points[count - 1], radius, GPath::Direction::kCW_Direction);
        }
        return *path;
    }






};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new MyFinal());
}



