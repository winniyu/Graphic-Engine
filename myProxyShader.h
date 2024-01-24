#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include <algorithm>
using namespace std;

class MyProxyShader : public GShader{

private:
    GShader* fRealShader;
    GMatrix  fExtraTransform;

public:
    MyProxyShader(GShader* shader, const GMatrix& extraTransform) : fRealShader(shader), fExtraTransform(extraTransform){      
    }

    bool isOpaque() override{
       return fRealShader->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override{
        return fRealShader->setContext(ctm * fExtraTransform);

    }
    void shadeRow(int x, int y, int count, GPixel row[])override{
       fRealShader->shadeRow(x, y, count, row);
    }      
    
};

std::unique_ptr<GShader> GCreateProxyShader(GShader* shader, const GMatrix& extraTransform){
    return std::unique_ptr<GShader>(new MyProxyShader(shader, extraTransform));
}
