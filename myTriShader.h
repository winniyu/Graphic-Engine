#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include <algorithm>
using namespace std;

class MyTriShader : public GShader{

private:
    GMatrix fLocalMatrix;
    GMatrix fCTInverse;
    GPoint U;
    GPoint V;
    GColor c0;
    GColor c1;
    GColor c2;

public:
    MyTriShader(const GColor c[3], const GPoint p[3]){
        c0 = c[0];
        c1 = c[1];
        c2 = c[2];
        U = p[1] - p[0];
        V = p[2] - p[0];    
        fLocalMatrix = GMatrix(U.x, V.x, p[0].x, U.y, V.y, p[0].y);
        fCTInverse = GMatrix();
    }

    bool isOpaque() override{
       return (c0.a == 1.0f && c1.a == 1.0f && c2.a == 1.0f);
    }

    bool setContext(const GMatrix& ctm) override{
        return (ctm * fLocalMatrix).invert(&fCTInverse);

    }
    GPixel premul_and_convert_to_gpixel(GColor& c){
        GColor convertColor = c.pinToUnit();
        unsigned r = GRoundToInt(convertColor.r * convertColor.a * 255);
        unsigned g = GRoundToInt(convertColor.g * convertColor.a * 255);
        unsigned b = GRoundToInt(convertColor.b * convertColor.a * 255);
        unsigned a = GRoundToInt(convertColor.a * 255);
        return GPixel_PackARGB(a,r,g,b);
    }

    void shadeRow(int x, int y, int count, GPixel row[])override{
       GPoint p = {x+0.5f, y+0.5f};
       GPoint gradient_p = fCTInverse * p;

       GColor DC1 = c1 - c0;
       GColor DC2 = c2 - c0;

       GColor C = gradient_p.x * DC1 + gradient_p.y * DC2 + c0;
       GColor DC = fCTInverse[0] * DC1 + fCTInverse[3] * DC2;

       for (int i = 0; i < count; ++i) {
        row[i] = premul_and_convert_to_gpixel(C);
        C += DC;
       }

    }      
};
std::unique_ptr<GShader> GCreateTriShader(const GColor c[3], const GPoint p[3]){
    return std::unique_ptr<GShader>(new MyTriShader(c, p));
}


