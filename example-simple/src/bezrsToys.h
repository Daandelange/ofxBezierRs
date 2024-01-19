#pragma once

//#include "ofMain.h"
#include "ofxBezierRs.h"
#include "bezierShape.h"
#include <vector>


class bezrsToy {
	const std::string name;

	public:
	const char* name_cstr();

	bezrsToy(std::string _name="Unknown") : name(_name) {};

	virtual void applyFX(const bezierShape& _inShape, bezierShape& _outShape) = 0;
	//virtual void renderShape(const bezrsShape& _sh);
	virtual void drawParams(const bezierShape& _sh);
};


class offsetToy : public bezrsToy {
	public:
	offsetToy() : bezrsToy("Offset Toy"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	float offset = 0.f;
	bezrsJoinType join = bezrsJoinType::Bevel;
};

class rotationToy : public bezrsToy {
	public:
	rotationToy() : bezrsToy("Rotation Toy"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	bezrsPos center;
	float rotation;
};

class reverseWindingToy : public bezrsToy {
	public:
	reverseWindingToy() : bezrsToy("Reverse Winding Toy"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;
};
