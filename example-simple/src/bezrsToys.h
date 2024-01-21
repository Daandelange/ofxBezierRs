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

class boundingBoxToy : public bezrsToy {
	public:
	boundingBoxToy() : bezrsToy("Bounding Box"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	bezrsRect bb;
};

class hitTestToy : public bezrsToy {
	public:
	hitTestToy() : bezrsToy("Hit test"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	glm::vec2 simPos = {0,0};
	const float simSpeed = 3;
	glm::vec2 simVec = {simSpeed,simSpeed};
	glm::vec2 mousePos = {0,0};
	bool simPosHit = false;
	bool mousePosHit = false;
	glm::vec2 mouseProjection, simProjection;
};

class evaluateToy : public bezrsToy {
	public:
	evaluateToy() : bezrsToy("Evaluate TValue"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	double tval = 0.;
	bezrsPos tPos;
	bezrsPos tTangent;
	bezrsPos tNormal;
	float tCurvature;
};

class selfIntersectToy : public offsetToy {
	public:
	selfIntersectToy() : offsetToy("Offset Self Intersect"){};
	void applyFX(const bezierShape& _inShape, bezierShape& _outShape) override;
	void drawParams(const bezierShape& _sh) override;

	protected:
	std::vector<bezrsPos> selfIntersects;
	std::vector<double> floatsVec;
};

// Todo : Global vs Euclidean tvalues
