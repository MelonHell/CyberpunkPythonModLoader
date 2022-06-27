#include "ImGuiCP.h"

namespace ImGuiCP {
	ImFont *rajdhani32 = nullptr;

	void Init() {
		ImGuiIO &io = ImGui::GetIO();
		rajdhani32 = io.Fonts->AddFontFromFileTTF(R"(E:\Downloads\New folder (18)\test\source\raw\base\gameplay\gui\fonts\foreign\russian\rajdhani_medium_neu.ttf)", 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
	}

	bool Button(const char *label) {
		ImGui::PushFont(rajdhani32);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.929411768913269, 0.3019607961177826, 0.2784313857555389, 1.0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3686274588108063, 0.9607843160629272, 1.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0235294122248888, 0.0235294122248888, 0.05098039284348488, 0.6));
		bool result = ImGui::Button(label);
		ImGui::PopFont();
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(3);

		return result;
	}

	bool InputText(const char *label, char *buf, size_t buf_size) {
		ImGui::PushFont(rajdhani32);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.929411768913269, 0.3019607961177826, 0.2784313857555389, 1.0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3686274588108063, 0.9607843160629272, 1.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0235294122248888, 0.0235294122248888, 0.05098039284348488, 0.6));
		bool result = ImGui::InputText(label, buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopFont();
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(3);
		return result;
	}
}