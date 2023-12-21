#pragma once

#include "ofMain.h"
#include <vector>
#include "ofxBezierRs.h"

struct ofxBezierShape {
	std::vector<bezrsBezierHandle> beziers = {};
	bool bChanged = false;

	void draw(bool connectLast=true, ofColor lineColor=ofColor::black, ofColor bezierColor=ofColor::blue);
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofxBezierShape shape;
		ofxBezierShape offsetShape;

		glm::vec2 clickedPos = {0,0};
		glm::vec2 bezierPreview = {0,0};
};
