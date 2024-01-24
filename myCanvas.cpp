#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GPaint.h"
#include "include/GBitmap.h"
#include "include/GPoint.h"
#include "helperforEdge.h"
#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include "helperBlend.h"
#include "myTriShader.h"
#include "myProxyShader.h"
#include "myComposeShader.h"
#include "myGradient.h"

#include <stack>
#include <cstdio>
using namespace std;

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {
        matrixStack.push(GMatrix());
    }
    
    void save() override{
        matrixStack.push(matrixStack.top());
    }
    void restore() override{
        if(matrixStack.size()>0){
            matrixStack.pop();
        }
    }
    void concat(const GMatrix& matrix) override{
        GMatrix current = matrixStack.top();
        matrixStack.top() = GMatrix::Concat(current, matrix);
    }
    
    void clear(const GColor& color) override {
        GPixel clearPixel = Convert(color);
        for (int y = 0; y < fDevice.height(); ++y) {
            for (int x = 0; x < fDevice.width(); ++x) {
                *fDevice.getAddr(x, y) = clearPixel;
            }
        }
    }
    
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GIRect int_Rect = rect.round();
        int_Rect.left = std::max(0, int_Rect.left);
        int_Rect.top = std::max(0, int_Rect.top);
        int_Rect.right = std::max(0,std::min(fDevice.width(), int_Rect.right));
        int_Rect.bottom = std::max(0,std::min(fDevice.height(), int_Rect.bottom));


        if(paint.getShader()==nullptr && matrixStack.top() == GMatrix()){
            GBlendMode blendMode = paint.getBlendMode();
            GPixel src = Convert(paint.getColor());
            BlendFunctionNormal blendFunctionNormal = selectBlendFunctionNormal(blendMode,GPixel_GetA(src));
            for(int y = int_Rect.top; y < int_Rect.bottom; ++y){
                blendFunctionNormal(fDevice,int_Rect.left,int_Rect.right,y,src);
            }
        }
        else{
            GPoint points[4];  
            points[0] = {rect.left, rect.top};
            points[1] = {rect.right,rect.top};
            points[2] = {rect.right,rect.bottom};
            points[3] = {rect.left,rect.bottom};
            drawConvexPolygon(points, 4, paint);
        }
        
    }
    
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override{
        if(count < 3){
            return;
        }
        //transform points
        GPoint Trans_points[count];
        matrixStack.top().mapPoints(Trans_points, points, count);
        
        // build initial edges and clipped
        GRect rect = GRect::WH(fDevice.width(), fDevice.height());
        GIRect int_Rect = rect.round();
        std::vector<Edge> clippedEdges = findPolygonEdges(Trans_points, count, int_Rect);
        if(clippedEdges.size() < 2){
            return;
        }
        // sort edges
        std::sort(clippedEdges.begin(), clippedEdges.end(), compareEdges);
        // blit x
        int top = clippedEdges.back().ymin;
        int bottom = clippedEdges.front().ymax;
        Edge e0 = clippedEdges.back();
        clippedEdges.pop_back();
        Edge e1 = clippedEdges.back();
        clippedEdges.pop_back();
        float left_x = e0.x0;
        float right_x = e1.x0;
        GBlendMode blendMode = paint.getBlendMode();
        GPixel src = Convert(paint.getColor());
        //blendfunction for normal
        BlendFunctionNormal blendFunctionNormal = selectBlendFunctionNormal(blendMode,GPixel_GetA(src));
        //blendfunction for shader
        BlendFunctionShader blendFunctionShader = selectBlendFunctionShader(blendMode);
        for (int y = top; y < bottom; ++y){
            //test
            if(y >= e0.ymax && !clippedEdges.empty()){
                e0 = clippedEdges.back();
                left_x = e0.x0;
                clippedEdges.pop_back();
            }
            if(y >= e1.ymax && !clippedEdges.empty()){
                e1 = clippedEdges.back();
                right_x = e1.x0;
                clippedEdges.pop_back();
            }
            //draw
            int temp_left = GRoundToInt(left_x);
            temp_left = std::max(0, temp_left);
            int temp_right = GRoundToInt(right_x);
            temp_right = std::min(temp_right, fDevice.width());
            int width = temp_right - temp_left;
            if(paint.getShader() != nullptr){
                if(paint.getShader() -> setContext(matrixStack.top())){
                    GPixel row[width];
                    paint.getShader() -> shadeRow(temp_left,y,width,row);
                    if (paint.getShader() -> isOpaque()) {
                        for (int i = 0; i < width; ++i) {
                            *fDevice.getAddr(temp_left + i, y) = row[i];
                        }
                    }
                    else{
                        blendFunctionShader(fDevice,temp_left,y,width,row);
                    }
                }
            }
            else{
                blendFunctionNormal(fDevice,temp_left,temp_right,y,src);
            }
            left_x += e0.m;
            right_x += e1.m;
        }
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        //transform
        GPath transP = path;
        transP.transform(matrixStack.top());
        //clip
        GRect rect = GRect::WH(fDevice.width(), fDevice.height());
        GIRect int_Rect = rect.round();
        std::vector<Edge> ClippedEdges;
        GPath::Edger edger(transP);
        GPoint p[GPath::kMaxNextPoints];
        GPath::Verb v = edger.next(p);
        while(v != GPath::kDone){
            if(v == GPath::kLine){
                ClipEdges(p[0], p[1], int_Rect, ClippedEdges);
            }
            else if(v == GPath::kQuad){
                GPoint A = p[0];
                //(A - 2B + C)/4
                GPoint E = (p[0]-2*p[1]+p[2])*0.25;
                //int num_segs = (int)ceil(sqrt(|E|/tolerance))
                int num_segs = GCeilToInt(sqrt(4 * E.length()));
                float segs = 1.0f / num_segs;
                float t = segs;
                GPoint dst[5];
                for(int i = 0; i < num_segs; ++i){
                    GPath::ChopQuadAt(p, dst, t);
                    GPoint P = dst[2];
                    ClipEdges(A, P, int_Rect, ClippedEdges);
                    A = P;
                    t += segs;
                }
                ClipEdges(A, p[2], int_Rect, ClippedEdges);
            }
            else if (v == GPath::kCubic){
                GPoint A = p[0];
                GPoint E;
                //E0 = A - 2B + C
                GPoint E0 = p[0]-2*p[1]+p[2];
                //E1 = B - 2C + D
                GPoint E1 = p[1]-2*p[2]+p[3];
                //E.x = max(abs(E0.x), abs(E1.x))
                E.x = std::max(std::abs(E0.x), std::abs(E1.x));
                //E.y = max(abs(E0.y), abs(E1.y))
                E.y = std::max(std::abs(E0.y), std::abs(E1.y));
                //int num_segs = (int)ceil(sqrt((3*|E|)/(4*tolerance)))
                int num_segs = GCeilToInt(sqrt((3*E.length())));
                float segs = 1.0f / num_segs;
                float t = segs;
                GPoint dst[7];
                for(int i = 0; i < num_segs; ++i){
                    GPath::ChopCubicAt(p, dst, t);
                    GPoint P = dst[3];
                    ClipEdges(A, P, int_Rect, ClippedEdges);
                    A = P;
                    t += segs;
                }
                ClipEdges(A, p[3], int_Rect, ClippedEdges);
            }
            v = edger.next(p);
        }
        if(ClippedEdges.size() < 1){
            return;
        }
        //sort
        std::sort(ClippedEdges.begin(), ClippedEdges.end(), comparePath);
        GBlendMode blendMode = paint.getBlendMode();
        GPixel src = Convert(paint.getColor());
        //blendfunction for normal
        BlendFunctionNormal blendFunctionNormal = selectBlendFunctionNormal(blendMode,GPixel_GetA(src));
        //blendfunction for shader
        BlendFunctionShader blendFunctionShader = selectBlendFunctionShader(blendMode);

        //blit
        int top = ClippedEdges[0].ymin;
        int left_edge;
        int right_edge;
        int y = top;
        while(ClippedEdges.size() > 0){
            int i = 0;
            int w = 0;
            while(ClippedEdges[i].isValid(y) && i < ClippedEdges.size()){
                int x = ClippedEdges[i].computeX(y);
                if(w==0){
                    left_edge = x;
                }
                w+=ClippedEdges[i].wind;
                if(w==0){
                    right_edge = x;
                    //draw
                    int temp_left = std::max(0, left_edge);
                    int temp_right = std::min(right_edge, fDevice.width());
                    int width = temp_right - temp_left;
                    if(paint.getShader() != nullptr){
                        if(paint.getShader() -> setContext(matrixStack.top())){
                            GPixel row[width];
                            paint.getShader() -> shadeRow(temp_left,y,width,row);
                            if (paint.getShader() -> isOpaque()) {
                                for (int i = 0; i < width; ++i) {
                                        *fDevice.getAddr(temp_left + i, y) = row[i];
                                }
                            }
                            else{
                                blendFunctionShader(fDevice,temp_left,y,width,row);
                            }
                        }
                    }
                    else{
                        blendFunctionNormal(fDevice,temp_left,temp_right,y,src);
                    }
                }
                if(ClippedEdges[i].isValid(y+1)){
                    ClippedEdges[i].x0 += ClippedEdges[i].m;
                    ++i;
                }
                else{
                    //remove
                    ClippedEdges.erase(ClippedEdges.begin()+i);
                }        
            }
            ++y;
            while(ClippedEdges[i].isValid(y) && i < ClippedEdges.size()){
                ++i;
            }
            //resort
            std::sort(ClippedEdges.begin(), ClippedEdges.begin()+i, compareX);
        }
    }

    GMatrix compute_basis(GPoint p0, GPoint p1, GPoint p2){
        return GMatrix(p1.x-p0.x, p2.x-p0.x, p0.x, p1.y-p0.y, p2.y-p0.y, p0.y);
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override{
        int n = 0;
        GPoint point0, point1, point2;
        GColor color0, color1, color2;
        GPoint texs0, texs1, texs2;

        for (int i = 0; i < count; ++i) {
            point0 = verts[indices[n+0]];
            point1 = verts[indices[n+1]];
            point2 = verts[indices[n+2]];
            if(colors != nullptr && texs != nullptr){
                color0 = colors[indices[n+0]];
                color1 = colors[indices[n+1]];
                color2 = colors[indices[n+2]];
                texs0 = texs[indices[n+0]];
                texs1 = texs[indices[n+1]];
                texs2 = texs[indices[n+2]];
                GMatrix P = compute_basis(point0, point1, point2);
                GMatrix T = compute_basis(texs0, texs1, texs2);
                GMatrix invT;
                T.invert(&invT);
                MyComposeShader composeShader(new MyTriShader(new GColor[3] {color0, color1, color2}, new GPoint[3]{point0, point1, point2}), new MyProxyShader(paint.getShader(), P*invT));
                GPaint composeP(&composeShader);
                drawConvexPolygon(new GPoint[3]{point0, point1, point2}, 3, composeP);
            }
            else if(colors != nullptr){
                color0 = colors[indices[n+0]];
                color1 = colors[indices[n+1]];
                color2 = colors[indices[n+2]];
                MyTriShader triShader(new GColor[3] {color0, color1, color2}, new GPoint[3]{point0, point1, point2});
                GPaint triP(&triShader);
                drawConvexPolygon(new GPoint[3]{point0, point1, point2}, 3, triP);
            }
            else if(texs != nullptr){
                texs0 = texs[indices[n+0]];
                texs1 = texs[indices[n+1]];
                texs2 = texs[indices[n+2]];
                GMatrix P = compute_basis(point0, point1, point2);
                GMatrix T = compute_basis(texs0, texs1, texs2);
                GMatrix invT;
                T.invert(&invT);
                MyProxyShader proxyShader(paint.getShader(), P*invT);
                GPaint proxyP(&proxyShader);
                drawConvexPolygon(new GPoint[3]{point0, point1, point2}, 3, proxyP);
            }
            else{
                drawConvexPolygon(new GPoint[3]{point0, point1, point2}, 3, paint);
            }
            n += 3;
        }
    }
    GPoint PointSubdivision(GPoint A, GPoint B, GPoint C, GPoint D, float u, float v){
        return (1.0f - u) * (1.0f - v) * A + (1.0f - v) * u * B + u * v * C + v * (1.0f - u) * D;
    }

    GColor ColorSubdivision(GColor A, GColor B, GColor C, GColor D, float u, float v){
        return (1.0f - u) * (1.0f - v) * A + (1.0f - v) * u * B + u * v * C + v * (1.0f - u) * D;
    }

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override{
        GPoint A = verts[0];
        GPoint B = verts[1];
        GPoint C = verts[2];
        GPoint D = verts[3];

        GPoint transVerts[4];
        GColor transColor[4];
        GPoint transTex[4];
        int indices[6] = {0,1,3,1,2,3};
        for(int u = 0; u <= level; ++u){
            for(int v = 0; v <= level; ++v){
                float u1 = (float)u/(float)(level+1);
                float u2 = (float)(u+1)/(float)(level+1);
                float v1 = (float)v/(float)(level+1);
                float v2 = (float)(v+1)/(float)(level+1);

                transVerts[0] = PointSubdivision(A,B,C,D,u1,v1);
                transVerts[1] = PointSubdivision(A,B,C,D,u2,v1);
                transVerts[2] = PointSubdivision(A,B,C,D,u2,v2);
                transVerts[3] = PointSubdivision(A,B,C,D,u1,v2);

                if(colors != nullptr){
                    transColor[0] = ColorSubdivision(colors[0],colors[1],colors[2],colors[3],u1,v1);
                    transColor[1] = ColorSubdivision(colors[0],colors[1],colors[2],colors[3],u2,v1);
                    transColor[2] = ColorSubdivision(colors[0],colors[1],colors[2],colors[3],u2,v2);
                    transColor[3] = ColorSubdivision(colors[0],colors[1],colors[2],colors[3],u1,v2);
                }
                if(texs != nullptr){
                    transTex[0] = PointSubdivision(texs[0],texs[1],texs[2],texs[3],u1,v1);
                    transTex[1] = PointSubdivision(texs[0],texs[1],texs[2],texs[3],u2,v1);
                    transTex[2] = PointSubdivision(texs[0],texs[1],texs[2],texs[3],u2,v2);
                    transTex[3] = PointSubdivision(texs[0],texs[1],texs[2],texs[3],u1,v2);
                }
                drawMesh(transVerts, colors ? transColor : nullptr, texs ? transTex : nullptr, 2, indices, paint);

            }
        }

    }


    
    
private:
    GBitmap fDevice;
    std::stack<GMatrix> matrixStack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GColor skyBlueColor = GColor::RGB(0.5f, 0.8f, 1.0f);
    canvas->clear(skyBlueColor);
    
    return "infinite blue";
}

