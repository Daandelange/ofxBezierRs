#include "ofMain.h"
#include "bezierShape.h"

// Statics
bool bezierShape::bShowNumbers = true;

//--------------------------------------------------------------
void bezierShape::draw(bool connectLast, ofColor lineColor, ofColor bezierColor){
    const static bool printNumbers = false;

    bezrsBezierHandle* bhPrev = connectLast ? &*this->beziers.rbegin() : nullptr;
    int i = 0;
    for(bezrsBezierHandle& bh : this->beziers){
        ofSetLineWidth(1);
        if(printNumbers)
            ofDrawBitmapStringHighlight(ofToString(i).c_str(), bh.pos.x, bh.pos.y, ofColor(0,0,0,50), ofColor(255,255,255));

        ofNoFill();
        ofSetColor(bezierColor,255);
        ofDrawLine(bh.pos.x, bh.pos.y, bh.in_bez.x, bh.in_bez.y);
        ofDrawLine(bh.pos.x, bh.pos.y, bh.out_bez.x, bh.out_bez.y);

        ofFill();
        ofDrawCircle(bh.in_bez.x, bh.in_bez.y, 2);
        ofDrawCircle(bh.out_bez.x, bh.out_bez.y, 2);

        ofSetColor(lineColor);
        ofDrawCircle(bh.pos.x, bh.pos.y, 3);

        ofNoFill();
        if(bhPrev != nullptr && this->beziers.size()>1){
            ofSetColor(lineColor, 30);
            ofDrawLine(bhPrev->pos.x, bhPrev->pos.y, bh.pos.x, bh.pos.y);
            ofSetLineWidth(2);
            ofSetColor(lineColor, 255);
            ofDrawBezier(bhPrev->pos.x, bhPrev->pos.y, bhPrev->out_bez.x, bhPrev->out_bez.y, bh.in_bez.x, bh.in_bez.y, bh.pos.x, bh.pos.y);
        }

        // Show numbers ?
        if(bezierShape::bShowNumbers){
            glm::vec2 offset = {5,5};
            if(i==0) offset *= -1; // fix offset of first ?
            ofDrawBitmapStringHighlight(ofToString(i), bh.pos.x+offset.x, bh.pos.y+offset.y, ofColor(0,0,0,80));
        }

        bhPrev = &bh;
        i++;
    }
}
