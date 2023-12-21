#pragma once
#include "bezier-rs-ffi.h"
//#include <glm/vec2.hpp>
#include "ofGraphicsBaseTypes.h"

bezrsPos to_bezrsPos(glm::vec2 _pos);


#ifdef OFXBEZRS_DEFINE_IMGUI_HELPERS
namespace ImGuiEx {
    void ofxBezierRsJointCombo(const char* _name, bezrsJoinType& _joinType, double* _mitter = nullptr);
    void ofxBezierRsOffsetOptions(const char* _name, double& _offset, bezrsJoinType& _joinType, double* _mitter = nullptr);
}
#endif
