#pragma once

struct TextureCollectionEditor {
	bool IsVisible;
	int  EditingTexture;

	TextureCollectionEditor() {
		myDialog.Mode = OpenFileMode;
		myDialog.Filter = ".png";
	}

	void Display() {
		if (IsVisible) {
			if (ImGui::Begin("Textures", &IsVisible, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));

				int count = 0;
				for (int ix = 1; ix < 32; ix++) {
					const Texture2D& tex = TextureCollection::Get(ix);
					if (tex.id() != 0) {
						count++;

						if (ImGui::ImageButton((ImTextureID)&tex, ImVec2(100, 100))) {
							EditingTexture = ix;
							memcpy(myTexturePath, tex.FileName, 256);
						}
						if ((count % 3))
							ImGui::SameLine();
					}
				}

				if (ImGui::Button("+", ImVec2(108, 106))) {
					myDialog.Show = true;
				}

				if (EditingTexture != 0) {
					ImGui::Separator();

					Texture2D& tex = TextureCollection::Get(EditingTexture);

					ImGui::Text("ID: %i", EditingTexture);
					ImGui::InputText("Name", tex.Name, 16);
					ImGui::InputText("Path", tex.FileName, 256);
					ImGui::SameLine();
					if (ImGui::Button("Browse")) {
						myDialog.Show = true;
					}
				}

				ImGui::PopStyleColor(2);

				ImGui::End();
			}
		}
		else {
			EditingTexture = 0;
		}

		if (myDialog.DrawDialog() == 1) {

			for (int ix = 1; ix < 32; ix++) {
				if (TextureCollection::Get(ix).id() == 0) {
					std::string name = myDialog.GetFile();
					relativitify(name);
					TextureCollection::LoadTexture(ix, name.c_str());
					Renderer::SetTexture(ix, TextureCollection::Get(ix).id());
					break;
				}
			}

		}
	}

private:
	char       myTextureName[16];
	char       myTexturePath[256];
	FileDialog myDialog;
};