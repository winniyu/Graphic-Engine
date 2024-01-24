#include "include/GShader.h"
#include "include/GColor.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "include/GPixel.h"
#include "include/GFinal.h"
#include <algorithm>
using namespace std;

GPixel color2pixel1(const GColor& c){
    GColor convertColor = c.pinToUnit();
    unsigned r = GRoundToInt(convertColor.r  * convertColor.a * 255);
    unsigned g = GRoundToInt(convertColor.g  * convertColor.a * 255);
    unsigned b = GRoundToInt(convertColor.b * convertColor.a * 255);
    unsigned a = GRoundToInt(convertColor.a * 255);
    return GPixel_PackARGB(a,r,g,b);
}
class MyColorMatrixShader : public GShader{
    public:
        MyColorMatrixShader(const GColorMatrix& colorMatrix, GShader* realShader) : M(colorMatrix), fRealShader(realShader){}

        bool isOpaque() override{
        return fRealShader->isOpaque();
        }

        bool setContext(const GMatrix& ctm) override{
            return fRealShader->setContext(ctm);

        }
        float check(float num){
            if (num > 1){
                num = 1;
            }
            else if (num < 0){
                num = 0;
            }
            return num;
        }



        void shadeRow(int x, int y, int count, GPixel row[]) override{
            GPixel realRow[count];
            fRealShader -> shadeRow(x,y,count,realRow);
            for (int i = 0; i < count; ++i){
                float old_a = (GPixel_GetA(realRow[i])+ 128) * 257 >> 16;
                float old_r = (GPixel_GetR(realRow[i])+ 128) * 257 >> 16;
                float old_g = (GPixel_GetG(realRow[i])+ 128) * 257 >> 16;
                float old_b = (GPixel_GetB(realRow[i])+ 128) * 257 >> 16;
                
                GColor ori_color = GColor :: RGBA(old_r, old_g, old_b, old_a);

                float new_r = M[0] * ori_color.r + M[4] * ori_color.g + M[8] * ori_color.b + M[12] * ori_color.a  + M[16];
                float new_g = M[1] * ori_color.r + M[5] * ori_color.g + M[9] * ori_color.b + M[13] * ori_color.a  + M[17];
                float new_b = M[2] * ori_color.r + M[6] * ori_color.g + M[10] * ori_color.b + M[14] * ori_color.a + M[18];
                float new_a = M[3] * ori_color.r + M[7] * ori_color.g + M[11] * ori_color.b + M[15] * ori_color.a + M[19];

                new_r = check(new_r);
                new_g = check(new_g);
                new_b = check(new_b);
                new_a = check(new_a);

                GColor new_color = GColor :: RGBA(new_r, new_g, new_b, new_a);
                row[i] = color2pixel1(new_color);
            }

            
        }

    private:
        const GColorMatrix& M;
        GShader* fRealShader;
    };




    std::unique_ptr<GShader> GCreateRadialGradientShader(const GColorMatrix& colorMatrix, GShader* realShader){
        return std::unique_ptr<GShader>(new MyColorMatrixShader(colorMatrix, realShader));
    }

