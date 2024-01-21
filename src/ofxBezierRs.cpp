#include "ofxBezierRs.h"

bezrsPos to_bezrsPos(const glm::vec2& _pos){
    bezrsPos ret;
    ret.x=_pos.x;
    ret.y=_pos.y;
    return ret;
}

glm::vec2 to_glmVec2(const bezrsPos& _pos){
    return glm::vec2(_pos.x, _pos.y);
}

std::vector<bezrsBezierHandle> bezrs_beziers_from_rect(const bezrsRect& _rect){
    std::vector<bezrsBezierHandle> ret;
    ret.push_back({{_rect.pos},{_rect.pos},{_rect.pos}});
    bezrsPos TR = {_rect.pos.x+_rect.size.x, _rect.pos.y};
    ret.push_back({TR,TR,TR});
    bezrsPos BR = {_rect.pos.x+_rect.size.x, _rect.pos.y+_rect.size.y};
    ret.push_back({BR,BR,BR});
    bezrsPos BL = {_rect.pos.x, _rect.pos.y+_rect.size.y};
    ret.push_back({BL,BL,BL});
    return ret;
}

std::ostream & operator<< (std::ostream &out, bezrsPos const &pos){
    out << "[" << pos.x << ", "<< pos.y << "]";
    return out;
}

#ifdef OFXBEZRS_DEFINE_IMGUI_HELPERS
#include "imgui.h"
#include <map>
#include <string>
void ofxBezierImGuiHelpMarker(const char* desc){
    ImGui::SameLine();
    ImGui::TextDisabled("[?]");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGuiEx::ofxBezierRsJointCombo(const char* _name, bezrsJoinType& _joinType, double* _mitter){
    const static std::map<bezrsJoinType, const std::string> joinMap = {
        {bezrsJoinType::Bevel, "Bevel"},
        {bezrsJoinType::Mitter, "Mitter"},
        {bezrsJoinType::Round, "Round"}
    };

    if (ImGui::BeginCombo(_name, joinMap.at((const bezrsJoinType)_joinType).c_str(), ImGuiComboFlags_None)) {
        for(auto j : joinMap) {
            const bool is_selected = (_joinType == j.first);
            if (ImGui::Selectable(j.second.c_str(), is_selected))
                _joinType = j.first;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();

        switch(_joinType){
            case bezrsJoinType::Bevel :
                ofxBezierImGuiHelpMarker("Slices off joins.");
                break;
            case bezrsJoinType::Mitter :
                ofxBezierImGuiHelpMarker("Extends joins outwards using a given treshold.");
                break;
            case bezrsJoinType::Round :
                ofxBezierImGuiHelpMarker("Rounds the joins.");
                break;
        }
    }
    if(_mitter != nullptr && _joinType==bezrsJoinType::Mitter){
        ImGui::Indent();
        const static double mitter_min = 0;
        ImGui::DragScalar("Mitter", ImGuiDataType_Double, _mitter, 0.1f, &mitter_min);
        ofxBezierImGuiHelpMarker("Sets the maximum distance of the created mitter.");
        ImGui::Unindent();
    }
}

void ImGuiEx::ofxBezierRsOffsetOptions(const char* _name, double& _offset, bezrsJoinType& _joinType, double* _mitter){
    ImGui::PushID(_name);
    ImGui::DragScalar("Offset", ImGuiDataType_Double, &_offset, 0.1);
    ofxBezierImGuiHelpMarker("Distance of the offset compared to the original path.\nPositive value goes inside the shape, negative value goes outside the shape.");
    ImGuiEx::ofxBezierRsJointCombo("Join Type", _joinType, _mitter);
    ImGui::PopID();
}
#endif
