#include "include/GPixel.h"
#include "include/GColor.h"
#include "include/GRect.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include "include/GBlendMode.h"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;


GPixel Convert(const GColor& c){
    GColor convertColor = c.pinToUnit();
    unsigned r = GRoundToInt(convertColor.r * convertColor.a * 255);
    unsigned g = GRoundToInt(convertColor.g * convertColor.a * 255);
    unsigned b = GRoundToInt(convertColor.b * convertColor.a * 255);
    unsigned a = GRoundToInt(convertColor.a * 255);
    return GPixel_PackARGB(a,r,g,b);
}


inline unsigned Division(int num){
    unsigned sum = (num + 128) * 257 >> 16;
    return sum;
}

inline GPixel Clear(GPixel src, GPixel dest){
    return GPixel_PackARGB(0,0,0,0);
}
inline GPixel Src(GPixel src, GPixel dest){
    return src;
}
inline GPixel Dst(GPixel src, GPixel dest){
    return dest;
}
inline GPixel SrcOver(GPixel src, GPixel dest) {
    unsigned r =
    GPixel_GetR(src) + Division(GPixel_GetR(dest) * (255 - GPixel_GetA(src)));
    unsigned g =
    GPixel_GetG(src) + Division(GPixel_GetG(dest) * (255 - GPixel_GetA(src)));
    unsigned b =
    GPixel_GetB(src) + Division(GPixel_GetB(dest) * (255 - GPixel_GetA(src)));
    unsigned a =
    GPixel_GetA(src) + Division(GPixel_GetA(dest) * (255 - GPixel_GetA(src)));
    return GPixel_PackARGB(a,r,g,b);
}

inline GPixel DstOver(GPixel src, GPixel dest) {
    unsigned r =
    GPixel_GetR(dest) + Division(GPixel_GetR(src) * (255 - GPixel_GetA(dest)));
    unsigned g =
    GPixel_GetG(dest) + Division(GPixel_GetG(src) * (255 - GPixel_GetA(dest)));
    unsigned b =
    GPixel_GetB(dest) + Division(GPixel_GetB(src) * (255 - GPixel_GetA(dest)));
    unsigned a =
    GPixel_GetA(dest) + Division(GPixel_GetA(src) * (255 - GPixel_GetA(dest)));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel SrcIn(GPixel src, GPixel dest) {
    unsigned r =
        Division(GPixel_GetR(src) * GPixel_GetA(dest));
    unsigned g =
        Division(GPixel_GetG(src) * GPixel_GetA(dest));
    unsigned b =
        Division(GPixel_GetB(src) * GPixel_GetA(dest));
    unsigned a =
        Division(GPixel_GetA(src) * GPixel_GetA(dest));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel DstIn(GPixel src, GPixel dest) {
    unsigned r =
        Division(GPixel_GetR(dest) * GPixel_GetA(src));
    unsigned g =
        Division(GPixel_GetG(dest) * GPixel_GetA(src));
    unsigned b =
        Division(GPixel_GetB(dest) * GPixel_GetA(src));
    unsigned a =
        Division(GPixel_GetA(dest) * GPixel_GetA(src));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel SrcOut(GPixel src, GPixel dest) {
    unsigned r =
        Division(GPixel_GetR(src) * (255 - GPixel_GetA(dest)));
    unsigned g =
        Division(GPixel_GetG(src) * (255 - GPixel_GetA(dest)));
    unsigned b =
        Division(GPixel_GetB(src) * (255 - GPixel_GetA(dest)));
    unsigned a =
        Division(GPixel_GetA(src) * (255 - GPixel_GetA(dest)));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel DstOut(GPixel src, GPixel dest) {
    unsigned r =
        Division(GPixel_GetR(dest) * (255 - GPixel_GetA(src)));
    unsigned g =
        Division(GPixel_GetG(dest) * (255 - GPixel_GetA(src)));
    unsigned b =
        Division(GPixel_GetB(dest) * (255 - GPixel_GetA(src)));
    unsigned a =
        Division(GPixel_GetA(dest) * (255 - GPixel_GetA(src)));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel SrcATop(GPixel src, GPixel dest) {
    unsigned r =
        Division(GPixel_GetA(dest) * GPixel_GetR(src) + GPixel_GetR(dest) * (255 - GPixel_GetA(src)));
    unsigned g =
        Division(GPixel_GetA(dest) * GPixel_GetG(src) + GPixel_GetG(dest) * (255 - GPixel_GetA(src)));
    unsigned b =
        Division(GPixel_GetA(dest) * GPixel_GetB(src) + GPixel_GetB(dest) * (255 - GPixel_GetA(src)));
    unsigned a =
        Division(GPixel_GetA(dest) * GPixel_GetA(src) + GPixel_GetA(dest) * (255 - GPixel_GetA(src)));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel DstATop(GPixel src, GPixel dest) {
    unsigned a =
    Division(GPixel_GetA(dest) * GPixel_GetA(src)) + Division((255 - GPixel_GetA(dest)) * GPixel_GetA(src));
    unsigned r =
    Division(GPixel_GetR(dest) * GPixel_GetA(src)) + Division((255 - GPixel_GetA(dest)) * GPixel_GetR(src));
    unsigned g =
    Division(GPixel_GetG(dest) * GPixel_GetA(src)) + Division((255 - GPixel_GetA(dest)) * GPixel_GetG(src));
    unsigned b =
    Division(GPixel_GetB(dest) * GPixel_GetA(src)) + Division((255 - GPixel_GetA(dest)) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel Xor(GPixel src, GPixel dest) {
    unsigned a =
    Division((255 - GPixel_GetA(src)) * GPixel_GetA(dest) + (255 - GPixel_GetA(dest)) * GPixel_GetA(src));
    unsigned r =
    Division((255 - GPixel_GetA(src)) * GPixel_GetR(dest) + (255 - GPixel_GetA(dest)) * GPixel_GetR(src));
    unsigned g =
    Division((255 - GPixel_GetA(src)) * GPixel_GetG(dest) + (255 - GPixel_GetA(dest)) * GPixel_GetG(src));
    unsigned b =
    Division((255 - GPixel_GetA(src)) * GPixel_GetB(dest) + (255 - GPixel_GetA(dest)) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

//Rect
typedef void (*BlendFunctionRect)(GBitmap&, GIRect, const GPixel);

void rClear(GBitmap& device, GIRect rect, GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            *device.getAddr(x, y) = GPixel_PackARGB(0, 0, 0, 0);
        }
    }
}

void rSrc(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            *device.getAddr(x, y) = source;
        }
    }
}

void rDest(GBitmap& device, GIRect rect, const GPixel source){
    return;
}

void rSrcOver(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = SrcOver(source, *p);
        }
    }
}

void rDstOver(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = DstOver(source, *p);
        }
    }
}

void rSrcIn(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = SrcIn(source, *p);
        }
    }
}

void rDstIn(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = DstIn(source, *p);
        }
    }
}

void rSrcOut(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = SrcOut(source, *p);
        }
    }
}

void rDstOut(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = DstOut(source, *p);
        }
    }
}

void rSrcATop(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = SrcATop(source, *p);
        }
    }
}

void rDstATop(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = DstATop(source, *p);
        }
    }
}

void rXor(GBitmap& device, GIRect rect, const GPixel source){
    for(int y = rect.top; y < rect.bottom; ++y){
        for(int x = rect.left; x < rect.right; ++x){
            GPixel* p = device.getAddr(x, y);
            *p = Xor(source, *p);
        }
    }
}

BlendFunctionRect selectBlendFunctionRect(GBlendMode blendMode, unsigned a) {
    switch (blendMode) {
        case GBlendMode::kClear:
            return rClear;
        case GBlendMode::kSrc:
            return rSrc;
        case GBlendMode::kDst:
            return rDest;
        case GBlendMode::kSrcOver:
            switch(a){
                case 0:
                    return rDest;
                case 255:
                    return rSrc;
                default:
                    return rSrcOver;
            }
        case GBlendMode::kDstOver:
            switch(a){
                case 0:
                    return rDest;
                default:
                    return rDstOver;
            }
        case GBlendMode::kSrcIn:
            switch(a){
                case 0:
                    return rClear;
                default:
                    return rSrcIn;
            }
            
        case GBlendMode::kDstIn:
            switch(a){
                case 0:
                    return rClear;
                case 255:
                    return rDest;
                default:
                    return rDstIn;
            }
        case GBlendMode::kSrcOut:
            switch(a){
                case 0:
                    return rClear;
                default:
                    return rSrcOut;
            }
            return rSrcOut;
        case GBlendMode::kDstOut:
            switch(a){
                case 0:
                    return rDest;
                case 255:
                    return rClear;
                default:
                    return rDstOut;
            }
        case GBlendMode::kSrcATop:
            switch(a){
                case 0:
                    return rDest;
                case 255:
                    return rSrcIn;
                default:
                    return rSrcATop;
            }
        case GBlendMode::kDstATop:
            switch (a){
                case 0:
                    return rClear;
                case 255:
                    return rDstOver;
                default:
                    return rDstATop;
            }
        case GBlendMode::kXor:
            switch (a){
                case 0:
                    return rDest;
                case 255:
                    return rSrcOut;
                default:
                    return rXor;
            }
        default:
            return rSrcOut;
    }
}


//normal
typedef void (*BlendFunctionNormal)(GBitmap&, int, int, int, const GPixel);

void nClear(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = GPixel_PackARGB(0, 0, 0, 0);
    }
}

void nSrc(GBitmap& fDevice, int left, int right, int y, const GPixel src){
   for (int i = left; i < right; ++i) {
       GPixel* p = fDevice.getAddr(i, y);
       *p = src;
    }
}

void nDest(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    return;
}

void nSrcOver(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = SrcOver(src, *p);
        }
}


void nDstOver(GBitmap& fDevice, int left, int right, int y, const GPixel src){
        for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = DstOver(src, *p);
        }
    }


void nSrcIn(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = SrcIn(src, *p);
    }
}

void nDstIn(GBitmap& fDevice, int left, int right, int y, const GPixel src){
     for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = DstIn(src, *p);
     }
}
    

void nSrcOut(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = SrcOut(src, *p);
    }
}


void nDstOut(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = DstOut(src, *p);
    }
 }


void nSrcATop(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
        *p = SrcATop(src, *p);
    }
}


void nDstATop(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
            *p = DstATop(src, *p);
        }
    }


void nXor(GBitmap& fDevice, int left, int right, int y, const GPixel src){
    for (int i = left; i < right; ++i) {
        GPixel* p = fDevice.getAddr(i, y);
            *p = Xor(src, *p);
    }
}

BlendFunctionNormal selectBlendFunctionNormal(GBlendMode blendMode, unsigned a) {
    switch (blendMode) {
        case GBlendMode::kClear:
            return nClear;
        case GBlendMode::kSrc:
            return nSrc;
        case GBlendMode::kDst:
            return nDest;
        case GBlendMode::kSrcOver:
            switch(a){
                case 0:
                    return nDest;
                case 255:
                    return nSrc;
                default:
                    return nSrcOver;
            }
        case GBlendMode::kDstOver:
            switch(a){
                case 0:
                    return nDest;
                default:
                    return nDstOver;
            }
        case GBlendMode::kSrcIn:
            switch(a){
                case 0:
                    return nClear;
                default:
                    return nSrcIn;
            }
            
        case GBlendMode::kDstIn:
            switch(a){
                case 0:
                    return nClear;
                case 255:
                    return nDest;
                default:
                    return nDstIn;
            }
        case GBlendMode::kSrcOut:
            switch(a){
                case 0:
                    return nClear;
                default:
                    return nSrcOut;
            }
            return nSrcOut;
        case GBlendMode::kDstOut:
            switch(a){
                case 0:
                    return nDest;
                case 255:
                    return nClear;
                default:
                    return nDstOut;
            }
        case GBlendMode::kSrcATop:
            switch(a){
                case 0:
                    return nDest;
                case 255:
                    return nSrcIn;
                default:
                    return nSrcATop;
            }
        case GBlendMode::kDstATop:
            switch (a){
                case 0:
                    return nClear;
                case 255:
                    return nDstOver;
                default:
                    return nDstATop;
            }
        case GBlendMode::kXor:
            switch (a){
                case 0:
                    return nDest;
                case 255:
                    return nSrcOut;
                default:
                    return nXor;
            }
        default:
            return nSrcOut;
    }
}

//shader
typedef void (*BlendFunctionShader)(GBitmap&, int, int, int, const GPixel row[]);

void sClear(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = GPixel_PackARGB(0, 0, 0, 0);
    }
}

void sSrc(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
   for (int i = 0; i < width; ++i) {
    GPixel* p = fDevice.getAddr(x + i, y);
    *p = row[i];
        }
    }

void sDest(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    return;
}

void sSrcOver(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = SrcOver(row[i], *p);
        }
}


void sDstOver(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
        for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = DstOver(row[i], *p);
        }
    }


void sSrcIn(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = SrcIn(row[i], *p);
    }
}

void sDstIn(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
     for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = DstIn(row[i], *p);
     }
}
    

void sSrcOut(GBitmap& fDevice,int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = SrcOut(row[i], *p);
    }
}


void sDstOut(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = DstOut(row[i], *p);
    }
 }


void sSrcATop(GBitmap& fDevice,int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
        *p = SrcATop(row[i], *p);
    }
}


void sDstATop(GBitmap& fDevice,int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
            *p = DstATop(row[i], *p);
        }
    }


void sXor(GBitmap& fDevice, int x, int y, int width, const GPixel row[]){
    for (int i = 0; i < width; ++i) {
        GPixel* p = fDevice.getAddr(x + i, y);
            *p = Xor(row[i], *p);
    }
}

BlendFunctionShader selectBlendFunctionShader(GBlendMode mode){
    switch (mode){
        case GBlendMode::kClear:
            return sClear;
        case GBlendMode::kSrc:
            return sSrc;
        case GBlendMode::kDst:
            return sDest;
        case GBlendMode::kSrcOver:
            return sSrcOver;
        case GBlendMode::kDstOver:
            return sDstOver;
        case GBlendMode::kSrcIn:
            return sSrcIn;
        case GBlendMode::kDstIn:
            return sDstIn;
        case GBlendMode::kSrcOut:
            return sSrcOut;
        case GBlendMode::kDstOut:
            return sDstOut;
        case GBlendMode::kSrcATop:
            return sSrcATop;
        case GBlendMode::kDstATop:
            return sDstATop;
        case GBlendMode::kXor:
            return sXor;
        default:
            return sSrcOut;
    }
}
