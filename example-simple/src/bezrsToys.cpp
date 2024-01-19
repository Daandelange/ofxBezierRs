//#include "ofMain.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"
#include "ofUtils.h"
#include "ofxBezierRs.h"
#include "bezrsToys.h"

//--------------------------------------------------------------
void bezrsToy::drawParams(const bezierShape& _sh){
    // Nothing drawn by default...
}

const char* bezrsToy::name_cstr(){
    return name.c_str();
}

//--------------------------------------------------------------
// Helpers

// Copies shape to raw handle then sends it to bezRS (Rust)
// Returned handle needs to be freed later !
inline bezrsShape* sendShapeToBezRs(const bezierShape& _inShape){
    // Make raw handle
    // Note: inline is important here, as `bezRsShapeInput` needs to be guaranteed until `bezRsShape` is freed.
    bezrsShapeRaw bezRsShapeInput = { _inShape.beziers.data(), _inShape.beziers.size(), true };
    // Build internal/opaque shape from raw input
    bezrsShape* bezRsShape = bezrs_shape_create( &bezRsShapeInput, true);
    return bezRsShape;
}

// Retrieves bezrs shape data to oF (c++)
// Destroys handle too
inline void populateShapeFromBezRs(bezrsShape* bezRsShape, bezierShape& _outShape, bool destroyShape=true){
    bezrsShapeRaw offsetShapeRaw = bezrs_shape_return_handle_data(bezRsShape);
    // Use result
    size_t bhi = 0;
    for (const bezrsBezierHandle* bh = offsetShapeRaw.data; bhi < offsetShapeRaw.len; bh++){
        // Populate shape
        _outShape.beziers.push_back(bezrsBezierHandle(*bh));
        bhi++;
    }
    // Destroy shape handle
    if(destroyShape) bezrs_shape_destroy(bezRsShape);
}

// Math helpers
#include <cmath>
float getModuloTime(float _interval = 1.f){
    return std::fmodf(ofGetElapsedTimef(), _interval)/_interval;
}
float getSineTime(float _interval = 1.f){
    return std::sin(ofGetElapsedTimef()*TWO_PI/_interval);
}

// Gui helpers
std::string getBezrsJoinString(bezrsJoinType join){
    std::string joinString("Join = ");
    joinString += (join==bezrsJoinType::Bevel?"Bevel":(join==bezrsJoinType::Mitter?"Mitter":(join==bezrsJoinType::Round?"Round":"Other")));
    if(join == bezrsJoinType::Round) joinString += " (unstable)";
    return joinString;
}


//--------------------------------------------------------------
void offsetToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Update vars
    updateParams();

    // Transform the shape
    bezrs_cubic_bezier_offset(bezRsShape, offset, join, 0);

    // Retrieve and destroy internal handle
    populateShapeFromBezRs(bezRsShape, _outShape, true);
}

void offsetToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Offsets the shape by an amount in pixels.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight("Beware the winding order : CCW reverses the direction and creates some artifacts.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(ofToString("Offset = ")+ofToString(offset), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(getBezrsJoinString(join), textPos.x, textPos.y);
}

void offsetToy::updateParams(){
    static const float cycle = 10.f;
    offset = getSineTime(cycle)*30.f;

    // Move joint type every cyle
    unsigned int now = ofGetElapsedTimef()/cycle;
    if( now != lastTime){
        lastTime = now;
        switch (join) {
            case bezrsJoinType::Bevel :
                join = bezrsJoinType::Mitter;
                break;
            case bezrsJoinType::Mitter :
                join = bezrsJoinType::Round;
                break;
            case bezrsJoinType::Round :
            default:
                join = bezrsJoinType::Bevel;
                break;
        }
    }
}

//--------------------------------------------------------------
void outlineToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Update vars
    updateParams();

    // Transform the shape
    bezrsShape* additionalOutlineShape = bezrs_shape_outline(bezRsShape, offset, join, bezrsCapType::Butt, 0);

    // Retrieve and destroy internal handle
    populateShapeFromBezRs(bezRsShape, _outShape, true);

    // Reset additional shape
    outlineShapeBis = {};

    // Got additional shape ? (needs destruction too!)
    if(additionalOutlineShape != nullptr){
        // Store additional shape until next frame
        populateShapeFromBezRs(additionalOutlineShape, outlineShapeBis, true);
    }
}

void outlineToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Creates an outline of a shape at a given distance.", textPos.x, textPos.y);
    //textPos.y -= 30;
    //ofDrawBitmapStringHighlight("Beware the winding order : CCW reverses the direction and creates some artifacts.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(ofToString("Distance = ")+ofToString(offset), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(getBezrsJoinString(join), textPos.x, textPos.y);
    //textPos.y -= 30;

    // Draw 2nd offset
    outlineShapeBis.draw(true, ofColor::red);
}

//--------------------------------------------------------------
void rotationToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Update vars
    center.x = ofGetWidth()*.5;
    center.y = ofGetHeight()*.5;
    rotation = getModuloTime(10.f)*TWO_PI;

    // Transform the shape
    bezrs_shape_rotate(bezRsShape, rotation, &center);

    // Retrieve and destroy internal handle
    populateShapeFromBezRs(bezRsShape, _outShape, true);

}

void rotationToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Rotates the shape (in radians) around a center point (green).", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(ofToString("Rotation = ")+ofToString(rotation), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(ofToString("Center = ")+ofToString(rotation), textPos.x, textPos.y);

    // Visualise center
    ofNoFill();
    ofSetColor(ofColor::green);
    const static int crossSize = 5;
    ofDrawLine(center.x - crossSize, center.y, center.x+crossSize, center.y);
    ofDrawLine(center.x, center.y-crossSize, center.x, center.y+crossSize);
}

//--------------------------------------------------------------
void reverseWindingToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Transform the shape
    bezrs_shape_reverse_winding(bezRsShape);

    // Retrieve and destroy internal handle
    populateShapeFromBezRs(bezRsShape, _outShape, true);

}

void reverseWindingToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Reverses the shape winding.", textPos.x, textPos.y);
    textPos.y -= 30;
}
