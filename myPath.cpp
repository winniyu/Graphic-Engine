#include "include/GPath.h"

void GPath::addRect(const GRect& r, Direction dir){
    moveTo(r.left, r.top);
    //clockwise
    if(dir == Direction::kCW_Direction){
        lineTo(r.right, r.top);
        lineTo(r.right, r.bottom);
        lineTo(r.left, r.bottom);
    }
    //counter-clockwise
    else{
        lineTo(r.left, r.bottom);
        lineTo(r.right, r.bottom);
        lineTo(r.right, r.top);
    }
}

void GPath::addPolygon(const GPoint pts[], int count){
    moveTo(pts[0]);
    for(int i = 0; i<count; i++){
        lineTo(pts[i]);
    }
}

//helper function
GPoint lerp(const GPoint& p0, const GPoint& p1, float t) {
    GPoint temp;
    temp.x = p0.x + t * (p1.x - p0.x);
    temp.y = p0.y + t * (p1.y - p0.y);
    return temp;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    GPoint AB = lerp(src[0], src[1], t);
    GPoint BC = lerp(src[1], src[2], t);
    GPoint EF = lerp(AB,BC,t);

    dst[0] = src[0];
    dst[1] = AB;
    dst[2] = EF;
    dst[3] = BC;
    dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
    GPoint AB = lerp(src[0], src[1], t);
    GPoint BC = lerp(src[1], src[2], t);
    GPoint CD = lerp(src[2], src[3], t);
    GPoint EF = lerp(AB,BC,t);
    GPoint GH = lerp(BC,CD,t);
    GPoint HI = lerp(EF,GH,t);

    dst[0] = src[0];
    dst[1] = AB;
    dst[2] = EF;
    dst[3] = HI;
    dst[4] = GH;
    dst[5] = CD;
    dst[6] = src[3];
}

// Append a new contour respecting the Direction. The contour should be an approximate 
// circle (8 quadratic curves will suffice) with the specified center and radius.
void GPath::addCircle(GPoint center, float radius, Direction direction){
    float c = 0.552285f;
    GMatrix mx = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);
    moveTo(mx * GPoint{0, 1});
    if (direction == kCW_Direction) {
        cubicTo(mx * GPoint{-c, 1}, mx * GPoint{-1, c}, mx * GPoint{-1, 0});
        cubicTo(mx * GPoint{-1, -c}, mx * GPoint{-c, -1}, mx * GPoint{0, -1});
        cubicTo(mx * GPoint{c, -1}, mx * GPoint{1, -c}, mx * GPoint{1, 0});
        cubicTo(mx * GPoint{1, c}, mx * GPoint{c, 1}, mx * GPoint{0, 1});
    }
    else{
        cubicTo(mx * GPoint{c, 1}, mx * GPoint{1, c}, mx * GPoint{1, 0});
        cubicTo(mx * GPoint{1, -c}, mx * GPoint{c, -1}, mx * GPoint{0, -1});
        cubicTo(mx * GPoint{-c, -1}, mx * GPoint{-1, -c}, mx * GPoint{-1, 0});
        cubicTo(mx * GPoint{-1, c}, mx * GPoint{-c, 1}, mx * GPoint{0, 1});
    }
}

void QuadExtrema(GPoint A, GPoint B, GPoint C, std::vector<float>& tList){
    float dx = A.x - 2 * B.x + C.x;
    if(dx != 0){
        float tx = (A.x - B.x)/dx;
        if(tx >= 0 && tx <= 1){
            tList.push_back(tx);
        }
    }
    float dy = A.y - 2 * B.y + C.y;
    if(dy != 0){
        float ty =  (A.y - B.y)/dy;
        if(ty >= 0 && ty <= 1){
            tList.push_back(ty);
        }
    }
    
}

void CubicExtrema(GPoint A, GPoint B, GPoint C, GPoint D, std::vector<float>& tList){
    float aX = 3.0f * (-1.0f * A.x + 3 * B.x - 3 * C.x + D.x);
    float bX = 6.0f * (A.x - 2.0f * B.x + C.x);
    float cX = 3.0f * (B.x - A.x);
    float dX = bX * bX- 4.0f * aX * cX;

    if(aX == 0){
        if(bX != 0){
            float tx = -1 * cX / bX;
            tList.push_back(tx);
        }
    }

    if(dX >= 0 && aX != 0){
        float tx_positive = (-1.0 * bX + sqrt(dX)) / (2.0f * aX);
        float tx_negative = (-1.0 * bX - sqrt(dX)) / (2.0f * aX);
        if(tx_positive >= 0 && tx_positive <= 1){
            tList.push_back(tx_positive);
        }
        if(tx_negative >= 0 && tx_negative <= 1){
            tList.push_back(tx_negative);
        }
    }

    float aY = 3.0f * (-1.0f * A.y + 3 * B.y - 3 * C.y + D.y);
    float bY = 6.0f * (A.y - 2.0f * B.y + C.y);
    float cY = 3.0f * (B.y - A.y);
    float dY = bY * bY- 4.0f * aY * cY;

    if(aY == 0){
        if(bY != 0){
            float ty = -1 * cY / bY;
            tList.push_back(ty);
        }
    }

    if(dY >= 0 && aY != 0){
        float ty_positive = (-1.0 * bY + sqrt(dY)) / (2.0f * aY);
        float ty_negative = (-1.0 * bY - sqrt(dY)) / (2.0f * aY);
        if(ty_positive >= 0 && ty_positive <= 1){
            tList.push_back(ty_positive);
        }
        if(ty_negative >= 0 && ty_negative <= 1){
            tList.push_back(ty_negative);
        }
    }
}

//Revise your code to compute the tight bounds of each curve segment
GRect GPath::bounds() const{
    if(fPts.empty()){
        return GRect::LTRB(0,0,0,0);
    }
    float left = fPts[0].x;
    float top = fPts[0].y;
    float right = fPts[0].x;
    float bottom = fPts[0].y;
    GPoint pts[kMaxNextPoints];
    GPath::Edger edger(*this);
    GPath::Verb v = edger.next(pts);
    std::vector<float> tList;
    while (v != GPath::kDone) {
        if(v == GPath::kLine){
            left = std::min(left, pts[1].x);
            top = std::min(top, pts[1].y);
            right = std::max(right, pts[1].x);
            bottom = std::max(bottom, pts[1].y);
        }
        else if(v == GPath::kQuad){
            left = std::min(left, pts[2].x);
            top = std::min(top, pts[2].y);
            right = std::max(right, pts[2].x);
            bottom = std::max(bottom, pts[2].y);
            QuadExtrema(pts[0], pts[1], pts[2], tList);    
            while(!tList.empty()){
                float t = tList.back();
                tList.pop_back();
                GPoint dst[5];
                ChopQuadAt(pts, dst, t);
                left = std::min(left, dst[2].x);
                top = std::min(top, dst[2].y);
                right = std::max(right, dst[2].x);
                bottom = std::max(bottom, dst[2].y);
            }
            tList.clear();
        }
        else if(v == GPath::kCubic){
            left = std::min(left, pts[3].x);
            top = std::min(top, pts[3].y);
            right = std::max(right, pts[3].x);
            bottom = std::max(bottom, pts[3].y);
            CubicExtrema(pts[0], pts[1], pts[2], pts[3], tList);   
            while(!tList.empty()){
                float t = tList.back();
                tList.pop_back();
                GPoint dst[7];
                ChopCubicAt(pts, dst, t);
                left = std::min(left, dst[3].x);
                top = std::min(top, dst[3].y);
                right = std::max(right, dst[3].x);
                bottom = std::max(bottom, dst[3].y);
            }
            tList.clear();
        }
        v = edger.next(pts);
    }
    return GRect::LTRB(left, top, right, bottom);
}
            
void GPath::transform(const GMatrix& m){
     for(int i = 0; i < fPts.size(); ++i){
         fPts[i] = m * fPts[i];
     }
}