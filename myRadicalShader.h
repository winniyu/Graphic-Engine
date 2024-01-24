#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "include/GPixel.h"
#include <algorithm>
using namespace std;

GPixel color2pixel(const GColor& c){
    GColor convertColor = c.pinToUnit();
    unsigned r = GRoundToInt(convertColor.r * convertColor.a * 255);
    unsigned g = GRoundToInt(convertColor.g * convertColor.a * 255);
    unsigned b = GRoundToInt(convertColor.b * convertColor.a * 255);
    unsigned a = GRoundToInt(convertColor.a * 255);
    return GPixel_PackARGB(a,r,g,b);
}

class MyRadicalShader : public GShader{

private:
    float fRadius;
    std::vector<GColor> fColors;
    int fCount;
    GMatrix fLocalM;
    GMatrix fInverseM;
    GShader::TileMode fMode;

public:
    MyRadicalShader(GPoint center, float radius, const GColor colors[], int count, GShader::TileMode mode) {
        fRadius = radius;
        fCount = count;
        fMode = mode;
        fLocalM = GMatrix(
            1, 0, center.x,
            0, 1, center.y
        );

        if(count < 1){
            return;
        }

        fColors.reserve(fColors.size() + count);
        for(int i = 0; i < count; ++i){
            fColors.push_back(colors[i]);
        }
    }
   

    bool isOpaque() override {
        for(int i = 0; i < fCount; ++i){
            if (fColors[i].a < 1.0f) {
                return false;
            } 
        }
        return true;
    }

    bool setContext(const GMatrix& ctm) override {
        return (ctm*fLocalM).invert(&fInverseM);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint p = {x+0.5f, y+0.5f};
        //rotate
        GPoint gradient_p = fInverseM * p;
        float dx = fInverseM[0];
        for (int i = 0; i < count; i++) {
            float sq = sqrtf((gradient_p.x)*(gradient_p.x) + (gradient_p.y)*(gradient_p.y));
            float t = sq / fRadius;
  
            if (fMode == GShader::TileMode::kClamp) {
                t = GPinToUnit(t);
            } else if (fMode == GShader::TileMode::kRepeat) {
                t -= floorf(t);
            } else if (fMode == GShader::TileMode::kMirror) {
                if ((int)floorf(t)%2 == 0) {
                    t -= floorf(t);
                } else {
                    t = 1. - (t-floorf(t));
                }
            }
            int int_color = floor((fCount-1) * t);
            float stride = 1. / (fCount-1);
            float int_mapped = int_color*stride;

            if (fCount == 1) {
                row[i] = color2pixel(fColors[0]);
                continue;
            }

            GColor L_C = fColors[int_color];
            GColor R_C = fColors[int_color+1 >= fCount ? int_color : int_color+1];
                
            t = GPinToUnit((t-int_mapped) / stride);

            row[i] = color2pixel({
                L_C.r*(1.0f-t) + R_C.r*t,
                L_C.g*(1.0f-t) + R_C.g*t,
                L_C.b*(1.0f-t) + R_C.b*t,
                L_C.a*(1.0f-t) + R_C.a*t
            });

            gradient_p.x += dx;
        }
    }
};

std::unique_ptr<GShader> GCreateRadialGradientShader(GPoint center, float radius,
                                                        const GColor colors[], int count,
                                                        GShader::TileMode mode) {
    return std::unique_ptr<GShader>(new MyRadicalShader(center, radius, colors, count, mode));
}