#pragma once

#include "ofColor.h"
#include "ofxBezierRs.h"
#include <vector>


struct bezierShape {
	std::vector<bezrsBezierHandle> beziers = {};
	bool bChanged = false;
	static bool bShowNumbers;

	void draw(bool connectLast=true, ofColor lineColor=ofColor::black, ofColor bezierColor=ofColor::blue);
};
