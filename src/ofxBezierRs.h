#pragma once
#include "bezier-rs-ffi.h"
//#include <glm/vec2.hpp>
#include <vector>
#include "ofGraphicsBaseTypes.h"

// Respect/use ofxImGui global enabler
#ifdef ofxAddons_ENABLE_IMGUI
#   ifndef OFXBEZRS_DEFINE_IMGUI_HELPERS
#       define OFXBEZRS_DEFINE_IMGUI_HELPERS
#   endif
#endif

// Glue
bezrsPos to_bezrsPos(const glm::vec2& _pos);
glm::vec2 to_glmVec2(const bezrsPos& _pos);
std::vector<bezrsBezierHandle> bezrs_beziers_from_rect(const bezrsRect& _rect);

// Overload glue (ofToString, etc)
std::ostream & operator<< (std::ostream& out, bezrsPos const& pos);
//inline glm::vec2::vec<2, float, glm::qualifier::defaultp>(const bezrsPos & v): x(v.x), y(v.y) {}

#ifdef OFXBEZRS_DEFINE_IMGUI_HELPERS
namespace ImGuiEx {
    void ofxBezierRsJointCombo(const char* _name, bezrsJoinType& _joinType, double* _mitter = nullptr);
    void ofxBezierRsOffsetOptions(const char* _name, double& _offset, bezrsJoinType& _joinType, double* _mitter = nullptr);
}
#endif
