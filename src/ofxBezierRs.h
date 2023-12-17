#pragma once
#include "bezier-rs-ffi.h"
//#include <glm/vec2.hpp>
#include "ofGraphicsBaseTypes.h"

bezrsPos to_bezrsPos(glm::vec2 _pos);


// Todo: make this an opt-in compile flag for non-ImGui users
namespace ImGuiEx {
    void ofxBezierRsJointCombo(const char* _name, bezrsJoinType& _joinType, double* _mitter = nullptr);
    void ofxBezierRsOffsetOptions(const char* _name, double& _offset, bezrsJoinType& _joinType, double* _mitter = nullptr);
}
