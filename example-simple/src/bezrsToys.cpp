//#include "ofMain.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
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

// Retrieves bezrs float data to oF (c++)
inline std::vector<double> floatsRawToVec(bezrsFloatsRaw& _floatsRaw){
    // Copy result
    std::vector<double> ret(_floatsRaw.len);
    size_t bhi = 0;
    for (const double* fr = _floatsRaw.data; bhi < _floatsRaw.len; fr++){
        // Populate shape
        ret.push_back(*fr); // Todo: potentially unsafe ? Check pointer ?
        bhi++;
    }
    return ret;
}

// Math helpers
#include <cmath>
double getModuloTime(double _interval = 1.){
    return std::fmod(ofGetElapsedTimef()/_interval, 1.);//_interval)/_interval/_interval;
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

//--------------------------------------------------------------
void boundingBoxToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Retrieve and destroy internal handle
    bb = bezrs_shape_boundingbox(bezRsShape);

    // Place rect in shape
    _outShape.beziers = bezrs_beziers_from_rect(bb);
    _outShape.bChanged = true;

    // Destroy manually
    bezrs_shape_destroy(bezRsShape);

}

void boundingBoxToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Calculates the bounding box that contains the shape.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("Position = ") + ofToString(bb.pos) , textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("Size = ") + ofToString(bb.size) , textPos.x, textPos.y);
    textPos.y -= 30;
}

//--------------------------------------------------------------
void hitTestToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Update mouse pos
    mousePos.x=ofGetMouseX(), mousePos.y = ofGetMouseY();

    // Animate simPos
    simPos += simVec;
    static const float mouseHitTolerance = 10;
    if(glm::distance(simPos, mousePos) <= mouseHitTolerance){
        float randAngle = ofRandom(0.,TWO_PI);
        simVec = glm::vec2(sin(randAngle)*simSpeed,cos(randAngle)*simSpeed);
    }
    else {
        if(simPos.x < 0 || simPos.x > ofGetWidth()) simVec.x *=-1;
        if(simPos.y < 0 || simPos.y > ofGetHeight()) simVec.y *=-1;
    }

    // Do hit tests
    simPosHit = bezrs_shape_containspoint(bezRsShape, to_bezrsPos(simPos));
    mousePosHit = bezrs_shape_containspoint(bezRsShape, to_bezrsPos(mousePos));

    // Project points
    mouseProjection = to_glmVec2(bezrs_shape_project_pos(bezRsShape, to_bezrsPos(mousePos)));
    simProjection = to_glmVec2(bezrs_shape_project_pos(bezRsShape, to_bezrsPos(simPos)));

    // Place rect in shape
    _outShape.beziers = _inShape.beziers;
    _outShape.bChanged = true;

    // Destroy manually
    bezrs_shape_destroy(bezRsShape);

}

void hitTestToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Checks if a point is contained in the shape.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight("Also finds the closest point on the curve, aka. projection.", textPos.x, textPos.y);

    // Draw positions
    ofFill();
    ofSetColor(simPosHit?ofColor::darkGreen:ofColor(ofColor::green, 100));
    ofDrawCircle(simPos, 5.f);
    ofDrawLine(simPos, simProjection);
    ofSetColor(mousePosHit?ofColor::darkBlue:ofColor(ofColor::blue,100));
    ofDrawCircle(mousePos, 5.f);
    ofDrawLine(mousePos, mouseProjection);
    ofNoFill();

    // Projections


}

//--------------------------------------------------------------
void evaluateToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Retrieve and destroy internal handle
    static const float cycle = 10.f;
    tval = getModuloTime(cycle);
    //tPos = {tval*500., 10};
    tPos = bezrs_shape_posfromtvalue(bezRsShape, tval);
    tNormal = bezrs_shape_normalfromtvalue(bezRsShape, tval);
    tTangent = bezrs_shape_tangentfromtvalue(bezRsShape, tval);
    tCurvature = bezrs_shape_curvaturefromtvalue(bezRsShape, tval);

    _outShape.beziers = _inShape.beziers;
    _outShape.bChanged = true;

    // Destroy manually
    bezrs_shape_destroy(bezRsShape);
}

void evaluateToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Evaluates a t-value position.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("Position = ") + ofToString(tPos), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("TValue = ") + ofToString(tval), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("Green = Tangent, Orange = Normal, Pink=Curvature"), textPos.x, textPos.y);
    textPos.y -= 30;

    // Curvature
    ofSetColor(ofColor::violet);
    if( std::abs(tCurvature) > 0.001 ){ // close to 0 means infinite !
        double radius = 1./tCurvature;
        //radius *= 10.;
        ofDrawCircle(tPos.x+tNormal.x*radius, tPos.y+tNormal.y*radius, radius);
        ofDrawLine(tPos.x, tPos.y, tPos.x+tNormal.x*radius, tPos.y+tNormal.y*radius);
    }

    // Tangent
    static const double mult = 20.;
    ofSetColor(ofColor::darkGreen);
    ofDrawLine(tPos.x, tPos.y, tPos.x+tTangent.x*mult, tPos.y+tTangent.y*mult);

    // Normal
    ofSetColor(ofColor::orange);
    ofDrawLine(tPos.x+tNormal.x*mult*-.5, tPos.y+tNormal.y*mult*-.5, tPos.x+tNormal.x*mult*.5, tPos.y+tNormal.y*mult*.5);

    // Draw evaluated position
    ofSetColor(ofColor::black);
    ofDrawCircle(tPos.x, tPos.y, 3);
}

//--------------------------------------------------------------
void selfIntersectToy::applyFX(const bezierShape& _inShape, bezierShape& _outShape) {
    // Create internal handle
    bezrsShape* bezRsShape = sendShapeToBezRs(_inShape);

    // Force fixed values
    offset = 30;
    join = bezrsJoinType::Bevel;
    static const float cycle = 10.f;
    //offset = getSineTime(cycle)*40.f; // to animate

    // Transform the shape
    bezrs_cubic_bezier_offset(bezRsShape, offset, join, 0);

    // Retrieve offset shape from internal handle
    populateShapeFromBezRs(bezRsShape, _outShape, false);

    // Find intersections !
    bezrsFloatsRaw intersectionTValues = bezrs_shape_selfintersections(bezRsShape, 0.001, 0.001);
//    std::vector<double> floatsVec = floatsRawToVec(intersectionTValues);
    floatsVec = floatsRawToVec(intersectionTValues);

    // Convert t-values to coordinates
//    selfIntersects.clear();
    selfIntersects = {};
    //std::cout << "TValueSize=" << floatsVec.size() << std::endl;
    for(double& tvalue : floatsVec){
        //if(tvalue>1 || tvalue < 0) std::cout << "TVALUE=" << tvalue << "\t / " << std::fmod(tvalue, 1.) << std::endl;;
        bezrsPos p = bezrs_shape_posfromtvalue(bezRsShape, std::fmod(std::abs(tvalue), 1.));
        //std::cout << "TValue=" << tvalue << "\t = " << p << std::endl;
        selfIntersects.push_back(p);
    }

    // Delete handle manually
    bezrs_shape_destroy(bezRsShape);
}

void selfIntersectToy::drawParams(const bezierShape& _sh){
    glm::vec2 textPos = {50, ofGetHeight() - 50};
    ofDrawBitmapStringHighlight("Finds self intersections in offset to detect errors.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight("Also demonstrates how to retrieve a pos from a t-value.", textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(ofToString("Offset = ")+ofToString(offset), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(getBezrsJoinString(join), textPos.x, textPos.y);
    textPos.y -= 30;
    ofDrawBitmapStringHighlight(std::string("Amount of self intersections: ")+ofToString(selfIntersects.size()), textPos.x, textPos.y, ofColor(ofColor::purple, 200));
    textPos.y -= 30;

    // Draw self intersects
    ofSetColor(ofColor::purple);
    for(bezrsPos& p : selfIntersects){
        ofDrawCircle(p.x, p.y, 3);
    }
    ofSetColor(ofColor::cyan);
    const glm::vec2 size = {ofGetHeight()*.5, 10};
    ofDrawRectangle(0,0,size.x,size.y);
    for(double& p : floatsVec){
        ofDrawCircle(size.x*p, size.y*.5, size.y*.5);
    }
    if(floatsVec.size()>0){
        ofDrawRectangle(0,size.y,size.x*0.01*floatsVec.size(),size.y*.2);
    }

}
