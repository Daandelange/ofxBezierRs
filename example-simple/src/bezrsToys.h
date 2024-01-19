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
	offsetToy(std::string _name="Offset") : bezrsToy(_name){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	float offset = 0.f;
	bezrsJoinType join = bezrsJoinType::Bevel;
	void updateParams();
	unsigned int lastTime = 0;
};

class outlineToy : public offsetToy {
	public:
	outlineToy() : offsetToy("Outline"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	bezierShape outlineShapeBis;
};

class rotationToy : public bezrsToy {
	public:
	rotationToy() : bezrsToy("Rotation"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	bezrsPos center;
	float rotation;
};

class reverseWindingToy : public bezrsToy {
	public:
	reverseWindingToy() : bezrsToy("Reverse Winding"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;
};
