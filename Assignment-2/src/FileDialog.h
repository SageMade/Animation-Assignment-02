#pragma once

#include <string>
#include <imgui\imgui.h>
#include <imgui\imgui_user.h>

#include <filesystem>
namespace fs = std::experimental::filesystem;

#include "FileHelpers.h"

enum FileDialogMode {
	OpenFileMode = 0,
	SaveFileMode = 1
};

struct FileDialog {
	FileDialogMode Mode;
	std::string    Filter;
	bool           Show;
	std::string    CurrentDirectory;

	int DrawDialog() {
		int hasPicked = 0;

		if (Show) {
			if (CurrentDirectory == "")
				CurrentDirectory = fs::current_path().string();

			fs::directory_entry& myPath = fs::directory_entry(fs::current_path());

			if (ImGui::Begin(Mode == OpenFileMode ? "Open File" : "Save File", &Show, ImGuiWindowFlags_AlwaysAutoResize)) {

				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

				ImGui::BeginChild((ImGuiID)0, ImVec2(200, 1));
				ImGui::EndChild();
				RenderDirectoryNode(myPath);

				ImGui::Separator();

				ImGui::Text("File: ");
				ImGui::SameLine();
				if (Mode == OpenFileMode) {
					ImGui::Text(fs::path(mySelectedFile).filename().string().c_str());
				}
				else {
					ImGui::InputText("", myBuffer, 255);
				}
				if (ImGui::ButtonEx(Mode == OpenFileMode ? "Open" : "Save")) {
					if (Mode == SaveFileMode) {
						std::string path = myBuffer;
						fs::path path_t = fs::path(path);
						if (!path_t.has_extension()) {
							path = path + Filter;
						}
						else if (strcmp(path_t.extension().string().c_str(), Filter.c_str()) != 0) {
							path = path_t.string() + Filter;
						}
						mySelectedFile = path;
					}
					hasPicked = 1;
					Show = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					hasPicked = 2;
					Show = false;
					mySelectedFile = "";
				}

				ImGui::PopStyleColor(4);

				ImGui::End();
			}
		}

		return hasPicked;
	}

	const std::string& GetFile() const {
		return mySelectedFile;
	}

private:
	std::string mySelectedFile;
	char        myBuffer[255];

	void RenderDirectoryNode(const fs::directory_entry& path) {

		for (auto &fPtr : fs::directory_iterator(path)) {
			if (fPtr.status().type() != fs::file_type::directory) {
				if (strcmp(fPtr.path().extension().string().c_str(), Filter.c_str()) == 0) {
					if (ImGui::Button(fPtr.path().filename().string().c_str())) {
						mySelectedFile = fPtr.path().string();
						if (Mode == SaveFileMode)
							memcpy(myBuffer, mySelectedFile.c_str(), mySelectedFile.size() + 1);
					}
				}
			}
			else {
				if (ImGui::TreeNode(fPtr.path().filename().string().c_str())) {
					RenderDirectoryNode(fPtr);
					ImGui::TreePop();
				}
			}
		}
	}
};