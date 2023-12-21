#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

}

//--------------------------------------------------------------
void ofApp::update(){
    // Update
    if(shape.bChanged){
        offsetShape = {};// clear data

        if(shape.beziers.size()>1){ // BezRs needs at least 2 points not to panic
            // Make raw handle
            bezrsShapeRaw bezRsShapeInput = { shape.beziers.data(), shape.beziers.size(), true };
            // Build internal/opaque shape from raw input
            bezrsShape* bezRsShape = bezrs_shape_create( &bezRsShapeInput, true);
            // Transform the shape
            bezrs_cubic_bezier_offset(bezRsShape, 20, bezrsJoinType::Bevel, 0);
            // Retrieve resulting shape
            bezrsShapeRaw offsetShapeRaw = bezrs_shape_return_handle_data(bezRsShape);
            // Use result
            size_t bhi = 0;
            for (const bezrsBezierHandle* bh = offsetShapeRaw.data; bhi < offsetShapeRaw.len; bh++){
                // Populate shape
                offsetShape.beziers.push_back(bezrsBezierHandle(*bh));
                bhi++;
            }
            // Destroy shape handle
            bezrs_shape_destroy(bezRsShape);
        }

        shape.bChanged = false;
    }
}

//--------------------------------------------------------------
void ofxBezierShape::draw(bool connectLast, ofColor lineColor, ofColor bezierColor){
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

        bhPrev = &bh;
        i++;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofClear(255,255,255);
    ofDrawBitmapStringHighlight("Draw a shape using your mouse. The shape better be winded clockwise.", 50, 50, ofColor(0,0,0,100), ofColor(255,255,255));
    ofDrawBitmapStringHighlight("To erase the last point hit backspace.", 50, 80, ofColor(0,0,0,100), ofColor(255,255,255));

    // Draw the main shape
    shape.draw();

    // Preview bezier creation
    if(clickedPos.x!=0 && clickedPos.y!=0 && shape.beziers.size()>0){
        ofNoFill();
        bezrsBezierHandle* bhPrev = &*shape.beziers.rbegin();
        bezrsBezierHandle* bhFirst = &*shape.beziers.begin();
        glm::vec2 offset;
        offset = {clickedPos.x-bezierPreview.x, clickedPos.y-bezierPreview.y};

        bezrsBezierHandle newBezier;
        newBezier.pos.x=clickedPos.x;
        newBezier.pos.y=clickedPos.y;
        newBezier.in_bez.x=clickedPos.x+offset.x;
        newBezier.in_bez.y=clickedPos.y+offset.y;
        newBezier.out_bez.x=clickedPos.x+offset.x*-1.f;
        newBezier.out_bez.y=clickedPos.y+offset.y*-1.f;
        ofDrawBezier(bhPrev->pos.x, bhPrev->pos.y, bhPrev->out_bez.x, bhPrev->out_bez.y, newBezier.in_bez.x, newBezier.in_bez.y, newBezier.pos.x, newBezier.pos.y);
        ofDrawBezier(newBezier.pos.x, newBezier.pos.y, newBezier.out_bez.x, newBezier.out_bez.y, bhFirst->in_bez.x, bhFirst->in_bez.y, bhFirst->pos.x, bhFirst->pos.y);
    }

    // Draw transformed shape
    if(offsetShape.beziers.size()>1){
        offsetShape.draw(false, ofColor::red);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key==OF_KEY_BACKSPACE){
        if(shape.beziers.size()>0){
            shape.beziers.pop_back();
            shape.bChanged = true;
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    bezierPreview = {x,y};
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    clickedPos = {x, y};
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    bezrsBezierHandle bh;
    // No bezier ?
    if(clickedPos.x == x && clickedPos.y == y){
        bh.pos.x=bh.in_bez.x=bh.in_bez.x=x;
        bh.pos.y=bh.in_bez.y=bh.in_bez.y=y;
    }
    // with bezier
    else {
        glm::vec2 offset = {clickedPos.x-x, clickedPos.y-y};
        bh.pos.x=clickedPos.x;
        bh.pos.y=clickedPos.y;
        bh.in_bez.x=clickedPos.x+offset.x;
        bh.in_bez.y=clickedPos.y+offset.y;
        bh.out_bez.x=clickedPos.x+offset.x*-1.f;
        bh.out_bez.y=clickedPos.y+offset.y*-1.f;
    }

    shape.beziers.push_back(bh);
    shape.bChanged = true;

    // reset
    bezierPreview = {0,0};
    clickedPos = {0,0};
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
