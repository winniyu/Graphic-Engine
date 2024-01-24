#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include <algorithm>
using namespace std;

class MyShader : public GShader{
public:
    MyShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode mode) : fBitmap(bitmap), fLocalMatrix(localMatrix), fMode(mode){
        fCTInverse = GMatrix();        
    }

    
    bool isOpaque() override{
        return fBitmap.isOpaque();
    }

    bool setContext(const GMatrix& ctm) override{
        if(ctm.invert(&fCTInverse)){
            return true;
        }
        return false;
    }

    float tile(float t, GShader::TileMode mode){
        if (mode==GShader::kClamp){
            t = std::min(std::max(t, 0.0f), 1.0f);
        }
        else if (mode == GShader::kRepeat){
            t = t - GFloorToInt(t);
        }
        else if(mode==GShader::kMirror){
            t *= 0.5;
            t = t - floor(t);
            if (t > 0.5){
               t = 1 - t;
            }
            t *= 2;
        }
        return t;
    }



    void shadeRow(int x, int y, int count, GPixel row[])override{
        //m=0 if else
        GPoint p = {x+0.5f, y+0.5f};
        GMatrix m = fLocalMatrix * fCTInverse;
        //return to original
        GPoint original_p = m * p;
        if(m[0] == 0 && m[3] == 0){
            float tx = original_p.x/fBitmap.width();
            float ty = original_p.y/fBitmap.height();
            tx = tile(tx, fMode);
            ty = tile(ty, fMode);
            //floor
            int fx = GFloorToInt(tx * fBitmap.width());
            int fy = GFloorToInt(ty * fBitmap.height());
            //clamp
            fx = std::max(0,std::min(fBitmap.width()-1, fx));
            fy = std::max(0,std::min(fBitmap.height()-1, fy));
            for (int i = 0; i < count; ++i) {
                row[i] = *fBitmap.getAddr(fx, fy);
            }
        }
        else{
            for (int i = 0; i < count; ++i) {
                float tx = original_p.x/fBitmap.width();
                float ty = original_p.y/fBitmap.height();
                tx = tile(tx, fMode);
                ty = tile(ty, fMode);
                int fx = GFloorToInt(tx * fBitmap.width());
                int fy = GFloorToInt(ty * fBitmap.height());
                //clamp
                fx = std::max(0,std::min(fBitmap.width()-1, fx));
                fy = std::max(0,std::min(fBitmap.height()-1, fy));

                row[i] = *fBitmap.getAddr(fx, fy);
                original_p.x += m[0];
                original_p.y += m[3];
            }
        }        
    }
    
    
private:
    GBitmap fBitmap;
    GMatrix fLocalMatrix;
    GMatrix fCTInverse;
    GShader::TileMode fMode;

};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode mode){
    return std::unique_ptr<GShader>(new MyShader(bitmap, localMatrix, mode));
}
