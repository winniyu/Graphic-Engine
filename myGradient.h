#include "include/GShader.h"
#include "include/GMatrix.h"
#include <vector>

GPixel Convert1(const GColor& c){
    GColor convertColor = c.pinToUnit();
    unsigned r = GRoundToInt(convertColor.r * convertColor.a * 255);
    unsigned g = GRoundToInt(convertColor.g * convertColor.a * 255);
    unsigned b = GRoundToInt(convertColor.b * convertColor.a * 255);
    unsigned a = GRoundToInt(convertColor.a * 255);
    return GPixel_PackARGB(a,r,g,b);
}

class MyGradient : public GShader{
    public:
        MyGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode):fCount(count), fMode(mode){
            if(count < 1){
                return;
            }
            fColors.reserve(fColors.size() + count);
            fColorDiff.reserve(fColorDiff.size() + count - 1);
            for(int i = 0; i < count; ++i){
                fColors.push_back(colors[i]);
            }
            for(int i = 0; i < count - 1; ++i){
                fColorDiff.push_back(colors[i+1] - colors[i]);
            }
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            fLocalMatrix = GMatrix(dx,-dy,p0.x, dy, dx, p0.y);
            fCTInverse = GMatrix();
        }
    
        bool isOpaque() override{
            for(int i = 0; i < fCount; ++i){
                if (fColors[i].a < 1.0f) {
                return false;
                } 
            }
            return true;
        }


        bool setContext(const GMatrix& ctm) override{
            if((ctm * fLocalMatrix).invert(&fCTInverse)){
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
            GPoint p = {x+0.5f, y+0.5f};
            //rotate
            GPoint gradient_p = fCTInverse * p;
            if (fCount == 1){
                std::fill(row, row + count, Convert1(fColors[0]));
            }
            else{
                if(fCTInverse[0] == 0 && fCTInverse[3] == 0){
                    float fx = tile(gradient_p.x, fMode);
                    //scale
                    fx = fx * (fCount - 1);
                    int index = GFloorToInt(fx);
                    //fract(x)
                    float t = fx - index;

                    //exactly on point
                    if(t == 0){
                        std::fill(row, row + count, Convert1(fColors[index]));
                    }
                    else{
                        std::fill(row, row + count, Convert1(fColors[index]+ t * fColorDiff[index]));
                    }
                }
                else{ 
                    //first scale then loop
                    const int countMinus1 = fCount - 1;
                    for (int i = 0; i < count; ++i) {
                        float fx = tile(gradient_p.x, fMode);
                        fx = fx * countMinus1;
                        int index = GFloorToInt(fx);
                        float t = fx - index;

                        if(t == 0){
                            row[i] = (Convert1(fColors[index]));
                        }
                        else{
                            row[i] = (Convert1(fColors[index]+ t * fColorDiff[index]));
                        }
                        gradient_p.x += fCTInverse[0];
                    }
                }  
            }
        }
    
private:
    int fCount;
    std::vector<GColor> fColors;
    std::vector<GColor> fColorDiff;
    GMatrix fLocalMatrix;
    GMatrix fCTInverse;
    GShader::TileMode fMode;
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode){
    return std::unique_ptr<GShader>(new MyGradient(p0, p1, colors, count, mode));
}

