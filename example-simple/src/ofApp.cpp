#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Populate toys
    toys.push_back(new offsetToy());
    toys.push_back(new rotationToy());
    toys.push_back(new reverseWindingToy());
    toys.push_back(new outlineToy());
    toys.push_back(new boundingBoxToy());
    toys.push_back(new hitTestToy());
    toys.push_back(new inflectionsToy());
    toys.push_back(new evaluateToy());
    toys.push_back(new selfIntersectToy());

    // Generate an initial drawing
    generateNewShape();
}

//--------------------------------------------------------------
void ofApp::update(){
    // Update
    //if(shape.bChanged){
    bool invertAction = ofGetKeyPressed(OF_KEY_RETURN);
    if(
        (bAnimate != invertAction ) || // Force change ?
        shape.bChanged // Changed
    ){
        fxShape = {};// clear data

        if(shape.beziers.size()>1){ // BezRs needs at least 2 points not to panic
            bezrsToy* toy = toys[currentToy];
            if(toy){
                toy->applyFX(shape, fxShape);
            }
        }

        shape.bChanged = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    int textY = 50;
    ofClear(255,255,255);

    if(bShowHelp){
        ofDrawBitmapStringHighlight("Draw a shape using your mouse. The shape better be winded clockwise.", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
        ofDrawBitmapStringHighlight("To erase the last point hit backspace.", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
        ofDrawBitmapStringHighlight("Pause any animations by holding RETURN", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
        ofDrawBitmapStringHighlight("Gui Toggles: H=Help, I=Info", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
        ofDrawBitmapStringHighlight("Render Toggles: A=Animate, n=Numbers", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
        ofDrawBitmapStringHighlight("Use arrows to change toy. (--> <--)", 50, textY, ofColor(0,0,0,100), ofColor(255,255,255));
        textY+=30;
    }

    glm::vec2 infoTextPos(ofGetWidth()-50-200, 50);
    if(bShowInfo){
        ofDrawBitmapStringHighlight("Original Shape:", infoTextPos.x, infoTextPos.y, ofColor(0,0,0,100), ofColor(255,255,255));
        infoTextPos.y+=30;
        ofDrawBitmapStringHighlight(ofToString("Points: ")+ofToString(shape.beziers.size()), infoTextPos.x, infoTextPos.y, ofColor(0,0,0,100), ofColor(255,255,255));
        infoTextPos.y+=50;
        ofDrawBitmapStringHighlight("Transformed Shape:", infoTextPos.x, infoTextPos.y, ofColor(0,0,0,100), ofColor(255,255,255));
        infoTextPos.y+=30;
        ofDrawBitmapStringHighlight(ofToString("Points: ")+ofToString(fxShape.beziers.size()), infoTextPos.x, infoTextPos.y, ofColor(0,0,0,100), ofColor(255,255,255));
    }

    // Draw the main shape
    shape.draw();

    // Preview bezier creation
    if(clickedPos.x!=0 && clickedPos.y!=0){
        glm::vec2 offset;
        offset = {clickedPos.x-bezierPreview.x, clickedPos.y-bezierPreview.y};

        bezrsBezierHandle newBezier;
        newBezier.pos.x=clickedPos.x;
        newBezier.pos.y=clickedPos.y;
        newBezier.in_bez.x=clickedPos.x+offset.x;
        newBezier.in_bez.y=clickedPos.y+offset.y;
        newBezier.out_bez.x=clickedPos.x+offset.x*-1.f;
        newBezier.out_bez.y=clickedPos.y+offset.y*-1.f;

        // Draw shape preview
        ofNoFill();
        if(shape.beziers.size()>0){
            ofSetColor(ofColor::black);
            bezrsBezierHandle* bhPrev = &*shape.beziers.rbegin();
            bezrsBezierHandle* bhFirst = &*shape.beziers.begin();

            ofDrawBezier(bhPrev->pos.x, bhPrev->pos.y, bhPrev->out_bez.x, bhPrev->out_bez.y, newBezier.in_bez.x, newBezier.in_bez.y, newBezier.pos.x, newBezier.pos.y);
            ofDrawBezier(newBezier.pos.x, newBezier.pos.y, newBezier.out_bez.x, newBezier.out_bez.y, bhFirst->in_bez.x, bhFirst->in_bez.y, bhFirst->pos.x, bhFirst->pos.y);
        }

        // Draw bezier handle preview
        ofSetColor(ofColor::blue);
        ofDrawLine(newBezier.pos.x, newBezier.pos.y, newBezier.out_bez.x, newBezier.out_bez.y);
        ofDrawLine(newBezier.pos.x, newBezier.pos.y, newBezier.in_bez.x, newBezier.in_bez.y);

    }

    // Draw transformed shape
    if(fxShape.beziers.size()>1){
        fxShape.draw(true, ofColor::red);

        // Show Toy params
        bezrsToy* toy = toys[currentToy];
        if(toy){
            int textX = 50;
            static const int fontSize = 8; // from ofDrawBitmapStringHighlight SRC
            static const int padding = 10;
            static const int toyOffset = fontSize * 7 + padding; // 7 = strlen(string_below)
            ofDrawBitmapStringHighlight(ofToString("Toys : "), textX, textY, ofColor(ofColor::black, 100));
            textX += toyOffset;
            for(auto* t : toys){
                int textWidth = fontSize * ofToString(t->name_cstr()).length() + padding;
                if(textX+textWidth > 600){ // Start new line ?
                    textX = 50 + toyOffset;
                    textY+= 20 + padding*.5;
                }
                ofDrawBitmapStringHighlight(t->name_cstr(), textX, textY, ofColor(ofColor::black, t==toy?255:100));
                textX += textWidth;
            }
            textY+=30;
            toy->drawParams(fxShape);
        }
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    // Handle shape editing
    if(key==OF_KEY_BACKSPACE){
        if(shape.beziers.size()>0){
            shape.beziers.pop_back();
            shape.bChanged = true;
        }
    }

    // Handle Shortcuts
    else if(key=='h' || key=='H'){
        // Toggle help
        bShowHelp = !bShowHelp;
    }
    else if(key=='i' || key=='I'){
        // Toggle help
        bShowInfo = !bShowInfo;
    }
    else if(key=='a' || key=='A'){
        // Toggle help
        bAnimate = !bAnimate;
    }
    else if(key=='n' || key=='N'){
        // Toggle help
        bezierShape::bShowNumbers = !bezierShape::bShowNumbers;
    }
    else if(key==OF_KEY_PAGE_DOWN){
        generateNewShape();
    }

    // Handle toy slider
    else if(key==OF_KEY_DOWN || key==OF_KEY_LEFT){
        if(toys.size()>0){
            if(currentToy==0) currentToy = toys.size()-1;
            else currentToy = (currentToy-1)%toys.size();
        }
        else currentToy = 0;
        shape.bChanged = true;
    }
    else if(key==OF_KEY_UP || key==OF_KEY_RIGHT){
        currentToy = (currentToy+1)%toys.size();
        shape.bChanged = true;
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

//--------------------------------------------------------------
void ofApp::generateNewShape(){
    // Generate a drawing
    shape.beziers = {};
    glm::vec2 center = {ofGetWidth()*.5, ofGetHeight()*.5};
    const float numPts = 10., radius = 160., variance = 8.f, bezierSize=35, bezierVariance=0.3*TWO_PI;
    for(unsigned int i=0; i<numPts; i++){
        bool pair = i%2;
        float angle = i/numPts*TWO_PI;
        glm::vec2 ptCenter = center + glm::vec2(cos(angle)*radius*(.5+.5*pair), sin(angle)*radius*(.5+.5*pair));
        ptCenter += glm::vec2(ofRandom(0.f,variance), ofRandom(0.f,variance));
        unsigned int rand = roundf(ofRandom(-.5,2.4));
        glm::vec2 bhInOffset = {0,0}, bhOutOffset = {0,0};
        glm::vec2 tan = glm::rotate(glm::vec2(cos(angle), sin(angle)), glm::pi<float>() * -0.5f +ofRandom(-bezierVariance,bezierVariance));
        if(rand == 1 || rand == 2){ // Opposite
            bhInOffset = tan * ofRandom(bezierSize-variance,bezierSize+variance);
            bhOutOffset = bhInOffset*glm::vec2(-1,-1);

            if(rand ==2)
                bhOutOffset =  ofRandom(bezierSize-variance,bezierSize+variance) * glm::rotate(glm::vec2(cos(angle), sin(angle)), glm::pi<float>() * +0.5f +ofRandom(-bezierVariance,bezierVariance));
        }
        bezrsBezierHandle bh = {
            {ptCenter.x, ptCenter.y},
            {ptCenter.x+bhInOffset.x, ptCenter.y+bhInOffset.y},
            {ptCenter.x+bhOutOffset.x, ptCenter.y+bhOutOffset.y}
        };
        shape.beziers.push_back(bh);
    }
    shape.bChanged = true;
}

