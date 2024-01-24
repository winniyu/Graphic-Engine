#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "include/GPixel.h"
#include <algorithm>
using namespace std;

class MyComposeShader : public GShader{

private:
    GShader* fShader1;
    GShader* fShader2;

public:
    MyComposeShader(GShader* shader1, GShader* shader2) : fShader1(shader1), fShader2(shader2){      
    }

    bool isOpaque() override{
       return (fShader1->isOpaque() && fShader2->isOpaque());
    }

    bool setContext(const GMatrix& ctm) override{
        return (fShader1->setContext(ctm) && fShader2->setContext(ctm));
    }

    GPixel modulate(GPixel pixel1, GPixel pixel2){
        unsigned a = (GPixel_GetA(pixel1) * GPixel_GetA(pixel2) + 128) * 257 >> 16;
        unsigned r = (GPixel_GetR(pixel1) * GPixel_GetR(pixel2) + 128) * 257 >> 16;
        unsigned g = (GPixel_GetG(pixel1) * GPixel_GetG(pixel2) + 128) * 257 >> 16;
        unsigned b = (GPixel_GetB(pixel1) * GPixel_GetB(pixel2) + 128) * 257 >> 16;
        return GPixel_PackARGB(a, r, g, b);
    }

    void shadeRow(int x, int y, int count, GPixel row[])override{
        GPixel output1[count];
        GPixel output2[count];
        fShader1 -> shadeRow(x, y, count, output1);
        fShader2 -> shadeRow(x, y, count, output2);
        for(int i = 0; i < count; i++){
            row[i] = modulate(output1[i], output2[i]);
        }
    }      
};
std::unique_ptr<GShader> GCreateComposeShader(GShader* shader1, GShader* shader2){
    return std::unique_ptr<GShader>(new MyComposeShader(shader1, shader2));
}

