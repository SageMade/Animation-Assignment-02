
#define GLEW_STATIC
#include "glew/glew.h"
#pragma comment(lib, "glew32s.lib")


// Core Libraries (std::)
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <conio.h>

// 3rd Party Libraries
#include <GLUT/freeglut.h>
#include <TTK/GraphicsUtils.h>
#include <TTK/Camera.h>
#include "imgui/imgui.h"
#include "imgui/imgui_user.h"
#include "GLM/glm.hpp"

#include "TTK\Texture2D.h"

#include "AnimationMath.h"
#include "TextureCollection.h"
#include "GameObject.h"

#include "ParticleEffect.h"
#include "ParticleLayer.h"
#include "ParticleLayerSettings.h"
#include "ParticleEffectSettings.h"

#include "Renderer.h"

#include "AdaptiveCurve.h"

#include "FileDialog.h"
#include "TextureCollectionEditor.h"

#include "EditorSettings.h"

#include <sstream>

// Defines and Core variables
#define FRAMES_PER_SECOND 60
const int FRAME_DELAY = 1000 / FRAMES_PER_SECOND; // Milliseconds per frame

#define MAX_PARTICLES_PER_LAYER 500

// Window size
int windowWidth = 800;
int windowHeight = 600;

// Angle conversions
const float degToRad = 3.14159f / 180.0f;
const float radToDeg = 180.0f / 3.14159f;

float deltaTime = 0.0f; // amount of time since last update (set every frame in timer callback)

// Mouse position in pixels
glm::vec3 mousePosition; // x, y, 0
glm::vec3 mousePositionFlipped; // x, windowHeight - y, 0

GameObject gameObject;

ParticleEffect particleEffect;

char inFileName[256];
char outFileName[256];

FileDialog OpenFileDlg;
FileDialog SaveFileDlg;

TextureCollectionEditor TexManager;

AdaptiveCurve<glm::vec2> curve;

void WindowReshapeCallbackFunction(int w, int h);

void InitializeScene()
{	
	OpenFileDlg.Show = false;
	OpenFileDlg.Filter = ".dat";
	OpenFileDlg.Mode = OpenFileMode;
	SaveFileDlg.Show = false;
	SaveFileDlg.Filter = ".dat";
	SaveFileDlg.Mode = SaveFileMode;

	TexManager.IsVisible = false;

	// Dont't forget the null terminator!
	memcpy(inFileName, "test.dat", 9);
	memcpy(outFileName, "test.dat", 9);
	
	EditorSettings::EffectSettings = ParticleEffectSettings();

	LayerConfig config;
	memcpy(config.Name, "Default", 8);

	ParticleLayerSettings settings = ParticleLayerSettings();
	settings.Config = config;

	particleEffect.AddLayer(settings);

	EditorSettings::EffectSettings.Layers.push_back(particleEffect.Layers.back()->Settings);
			
	//std::fstream stream;
	//stream.open("test.dat");
	//particleEffect.WriteToFile(stream);
	//particleEffect = ParticleEffect::ReadFromFile(stream);
	//stream.close();

	particleEffect.Init();

	TextureCollection::LoadTexture(1, "textures/snow.png");
	TextureCollection::LoadTexture(2, "textures/flare.png");
	
	//Renderer::SetTexture(0, TextureCollection::Get(1).id());
	Renderer::SetTexture(1, TextureCollection::Get(1).id());
	Renderer::SetTexture(2, TextureCollection::Get(2).id());

    WindowReshapeCallbackFunction(TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);
}

// These values are controlled by imgui
bool applySeekingForce = true;
float seekingForceScale    = 100.0f;
float minSeekingForceScale = -200.0f;
float maxSeekingForceScale = 200.0f; 

void SaveEffect(const char* fileName) {
	if (fileName[0] != '\0') {
		std::fstream stream;
		stream.open(fileName, std::ios::out | std::ios::binary);
		if (stream.good()) {
			TextureCollection::WriteToFile(stream);
			particleEffect.WriteToFile(stream);
		}
		else {
			std::cout << "Failed to save file" << std::endl;
		}
		stream.close();
	}
}

void LoadEffect(const char* fileName) {
	if (fileName[0] != '\0') {
		std::fstream stream;
		stream.open(fileName, std::ios::in | std::ios::binary);
		if (stream.good()) {
			TextureCollection::ReadFromFile(stream);
			particleEffect = *ParticleEffect::ReadFromFile(stream);
			particleEffect.Init();

			EditorSettings::EffectSettings = ParticleEffectSettings();
			EditorSettings::EffectSettings.Layers.reserve(particleEffect.Layers.size());
			memcpy(EditorSettings::EffectSettings.Name, particleEffect.Name, EFFECT_NAME_MAX_LENGTH);
			for (int ix = 0; ix < particleEffect.Layers.size(); ix++) {
				EditorSettings::EffectSettings.Layers.push_back(particleEffect.Layers[ix]->Settings);
			}
		}
		else {
			std::cout << "Failed to read file" << std::endl;
		}
		stream.close();
	}
}

/*
void applyForcesToParticleSystem(ParticleLayer* e, glm::vec3 target)
{
	// TODO: implement seeking
	// Loop through each particle in the emitter and apply a seeking for to them
	for (int i = 0; i < e->getNumParticles(); i++) {
		glm::vec3 seekVector = target - e->getParticlePosition(i);
		glm::vec3 seekDirection = glm::normalize(seekVector);
		glm::vec3 seekForce = seekDirection * seekingForceScale;
		e->applyForceToParticle(i, seekForce);
	}
}
*/

struct BezierCurve {
	
	glm::vec3 P0;
	glm::vec3 T0;
	glm::vec3 T1;
	glm::vec3 P1;

	glm::vec3 Resolve(float t) {
		return Math::SolveBezier(P0, T0, T1, P1, t);
	}

};

void DisplayEditablePointList(std::vector<glm::vec3> &list) {
	ImGui::Text("Path:");

	for (int ix = 0; ix < list.size(); ix++) {
		ImGui::PushID(ix);
		ImGui::DragFloat3("", &list[ix][0]);
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			list.erase(list.begin() + ix);
			ix--;
			ImGui::PopID();
			continue;
		}
		ImGui::PopID();
	}

	if (ImGui::Button("+")) {
		list.push_back(glm::vec3());
	}
}

void DisplaySteeringBehaviour(SteeringBehaviour& behaviour) {
	ImGui::InputText(" Name", behaviour.Name, BEHAVIOUR_NAME_SIZE);
	ImGui::DragFloat(" Weight", &behaviour.Weight, 0.1f, 0.0f, 1.0f);
	if (ImGui::Combo(" Method", (int*)&behaviour.Method, "None\0Seek\0Flee\0Repel\0Attract\0Path\0\0")) {
		free(behaviour.MetaData);
		behaviour.MetaData = nullptr;
	}
	
	switch (behaviour.Method) {
		case Seek: {
			if (behaviour.MetaData == nullptr) {
				behaviour.SetData(new SeekFleeData());
			}
			SeekFleeData& data = *reinterpret_cast<SeekFleeData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Force", &data.Force);
		}
		break;
		case Flee: {
			if (behaviour.MetaData == nullptr) {
				behaviour.SetData(new SeekFleeData());
			}
			SeekFleeData& data = *reinterpret_cast<SeekFleeData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Force", &data.Force);
		}
		break;
		case Repel: {
			if (behaviour.MetaData == nullptr) {
				behaviour.SetData(new MagnetData());
			}
			MagnetData& data = *reinterpret_cast<MagnetData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Radius", &data.Radius);
			ImGui::DragFloat(" Force", &data.Force);
		}
		break;
		case Attract: {
			if (behaviour.MetaData == nullptr) {
				behaviour.SetData(new MagnetData());
			}
			MagnetData& data = *reinterpret_cast<MagnetData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Radius", &data.Radius);
			ImGui::DragFloat(" Force", &data.Force);
		}
		break;
		case Path: {
			if (behaviour.MetaData == nullptr) {
				behaviour.SetData(new PathData());
			}
			PathData& data = *reinterpret_cast<PathData*>(behaviour.MetaData);		
			ImGui::Combo(" Mode", (int*)&data.LoopMode, "Loop\0Reverse\0Stop\0\0");
			ImGui::DragFloat(u8" Node Radius²", &data.NodeRadius, 0.1f, 0.0f, 5.0f, "%.3f", 0.5f);
			DisplayEditablePointList(data.Points);
		}
		break;
		default:
			break;
	}
}

void DisplayLayerConfig(ParticleLayerSettings *settingsPtr) {

	bool show = EditorSettings::ActiveEditLayer != nullptr;

	if (show) {
		ParticleLayerSettings &settings = *settingsPtr;

		static char* buffer = new char[EFFECT_NAME_MAX_LENGTH + 6];
		sprintf(buffer, "%s###lr", settings.Config.Name);

		if (ImGui::Begin(buffer, &show, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::BeginChild((ImGuiID)0, ImVec2(280, 1));
			ImGui::EndChild();

			if (ImGui::CollapsingHeader("Physics:")) {
				ImGui::DragFloat3(" Gravity", &settings.Config.Gravity[0]);
				ImGui::DragFloat3(" Velocity Radius", &settings.Config.VelocityRadius[0]);
				ImGui::DragFloat3(" Velocity Offset", &settings.Config.VelocityOffset[0]);
				ImGui::DragFloat2(" Velocity Range", &settings.Config.VelocityRange[0]);
				ImGui::DragFloat2(" Angular Range", &settings.Config.AngularSpeedRange[0]);
				ImGui::DragFloat2(" Mass Range", &settings.Config.MassRange[0], 0.1f, 0.01f, 1000.0f);
			}

			if (ImGui::CollapsingHeader("Emitter:")) {
				ImGui::DragFloat3(" Position", &settings.Config.Position[0]);
				//static CurveData emissionOverTime = CurveData();
				//ImGui::Curve("Emission", ImVec2(250, 150), CurveData::CONTROL_COUNT, emissionOverTime.Controls);
				ImGui::SliderFloat(" Emission Rate", &settings.Config.EmissionRate, 0, settings.Config.MaxParticles, "%.1f", 1.0f, 500);
				ImGui::DragFloat(" Duration", &settings.Config.Duration);
				ImGui::DragFloat2(" Life Range", &settings.Config.LifeRange[0]);
				ImGui::DragFloat2(" Size Range", &settings.Config.SizeRange[0]);
				ImGui::Combo(" Emitter Type", (int*)&settings.Config.BoundsType, "Point\0Box\0Circle\0Line\0\0");
				ImGui::DragFloat3(" Emitter Meta", &settings.Config.BoundsMeta[0]);
				int maxPart = settings.Config.MaxParticles;
				ImGui::DragInt(" Max Particles", &maxPart, 1, 100, 5000);
				settings.Config.MaxParticles = maxPart;
				ImGui::SameLine();
				if (ImGui::Button("Apply")) {
					particleEffect.Layers[settings.Config.Index]->freeMemory();
					particleEffect.Layers[settings.Config.Index]->initialize();
				}
			}

			if (ImGui::CollapsingHeader("Visuals:")) {
				if (ImGui::TreeNode("Initial Color")) {
					ImGui::ColorPicker("", &settings.Config.InitColor[0]);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Final Color")) {
					ImGui::ColorPicker("", &settings.Config.FinalColor[0]);
					ImGui::TreePop();
				}
				ImGui::Checkbox(" Interpolate Color", &settings.Config.InterpolateColor);
				ImGui::Combo(" Blend Mode", (int*)&settings.Config.BlendMode, "Multiply\0Additive\0\0");


				std::stringstream stream;
				stream << "Default";
				stream.write("\0", 1);

				for (int ix = 1; ix < 32; ix++) {
					if (TextureCollection::Get(ix).id() != 0) {
						stream << TextureCollection::Get(ix).Name;
						stream.write("\0", 1);
					}
					else {
						stream << "-none-";
						stream.write("\0", 1);
					}
				}

				int foo = settings.Config.TextureID;
				ImGui::Combo(" Texture Mode", &foo, stream.str().c_str());
				settings.Config.TextureID = foo;
			}

			if (ImGui::CollapsingHeader("Behaviours:")) {
				ImGui::Text("Add new: ");
				ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));
				if (ImGui::Button("+")) {
					SteeringBehaviour toAdd = SteeringBehaviour();
					sprintf(toAdd.Name, "Behaviour %i", settings.Behaviours.size());
					settings.Behaviours.push_back(toAdd);
				}
				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar(1);

				for (int ix = 0; ix < settings.Behaviours.size(); ix++) {
					ImGui::Separator();

					static char* buffer = new char[BEHAVIOUR_NAME_SIZE + 12];
					sprintf(buffer, "%s###bhv%u", settings.Behaviours[ix].Name, ix);

					if (ImGui::TreeNode(buffer)) {
						if (ImGui::Button("Remove")) {
							settings.Behaviours.erase(settings.Behaviours.begin() + ix);
							ix--;
							ImGui::TreePop();
							ImGui::PopID();
							continue;
						}
						ImGui::Separator();
						DisplaySteeringBehaviour(settings.Behaviours[ix]);
						ImGui::TreePop();
					}
				}
			}

			ImGui::End();
		}
	}

	if (!show)
		EditorSettings::ActiveEditLayer = nullptr;
}

void DisplayEffectConfig(ParticleEffectSettings& settings) {
	ImGui::Text("Effect Config");
	ImGui::InputText(" Name", particleEffect.Name, 32);
	ImGui::Separator();
	ImGui::Text("Layers: ");
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));
	if (ImGui::Button("+###addLayer")) {
		ParticleLayerSettings toAdd = ParticleLayerSettings();
		toAdd.Config.TextureID = 1;
		sprintf(toAdd.Config.Name, "Layer %i", settings.Layers.size());

		particleEffect.AddLayer(toAdd);
			
		settings.Layers.push_back(toAdd);
		// Since vectors reallocate, we need to refresh all our pointers
		for (int ix = 0; ix < settings.Layers.size(); ix++)
			settings.Layers[ix] = particleEffect.Layers[ix]->Settings;
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(1);

	for (int ix = 0; ix < settings.Layers.size(); ix++) {
		ImGui::PushID(ix);
		ImGui::Separator();
		ImGui::InputText("Name", particleEffect.Layers[ix]->Settings.Config.Name, MAX_LAYER_NAME_SIZE);
		memcpy(settings.Layers[ix].Config.Name, particleEffect.Layers[ix]->Settings.Config.Name, MAX_LAYER_NAME_SIZE);
		if (ImGui::Button("Edit")) {
			EditorSettings::ActiveEditLayer = &particleEffect.Layers[ix]->Settings;
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			settings.Layers.erase(settings.Layers.begin() + ix);
			particleEffect.ReplaceSettings(EditorSettings::EffectSettings);
			ix--;
		}
		if (ix > 0) {
			ImGui::SameLine();
			if (ImGui::Button("^")) {
				ParticleLayerSettings temp = settings.Layers[ix - 1];
				settings.Layers[ix - 1] = settings.Layers[ix];
				settings.Layers[ix] = temp;

				ParticleLayer * tempLayer = particleEffect.Layers[ix -1];
				particleEffect.Layers[ix - 1] = particleEffect.Layers[ix];
				particleEffect.Layers[ix] = tempLayer;
			}
		}
		if (ix < settings.Layers.size() - 1) {
			ImGui::SameLine();
			if (ImGui::Button("v")) {
				ParticleLayerSettings temp = settings.Layers[ix + 1];
				settings.Layers[ix + 1] = settings.Layers[ix];
				settings.Layers[ix] = temp;

				ParticleLayer * tempLayer = particleEffect.Layers[ix + 1];
				particleEffect.Layers[ix + 1] = particleEffect.Layers[ix];
				particleEffect.Layers[ix] = tempLayer;
			}
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Text("Behaviours: ");
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));
	if (ImGui::Button("+###addBehaviour")) {
		SteeringBehaviour toAdd = SteeringBehaviour();
		sprintf(toAdd.Name, "Behaviour %i", particleEffect.Behaviours.size());

		particleEffect.AddBehaviour(toAdd);
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(1); 

	for (int ix = 0; ix < particleEffect.Behaviours.size(); ix++) {
		ImGui::Separator();
		static char label[23] = "                      ";
		ImGui::PushID(ix);
		int count = strlen(particleEffect.Behaviours[ix].Name);
		memcpy(label, particleEffect.Behaviours[ix].Name, count);
		memcpy(label + count, "###foo", 7);

		if (ImGui::TreeNode(label)) {
			if (ImGui::Button("Remove")) {
				particleEffect.Behaviours.erase(particleEffect.Behaviours.begin() + ix);
				ix--;
				ImGui::TreePop();
				ImGui::PopID();
				continue;
			}
			DisplaySteeringBehaviour(particleEffect.Behaviours[ix]);
			ImGui::TreePop();
		}

		ImGui::PopID();
	}
}

TTK::Camera camera;
glm::vec3 clearColor = glm::vec3(0.5f);

// This is where we draw stuff
void DisplayCallbackFunction(void)
{
	// Set up scene
	TTK::Graphics::SetBackgroundColour(clearColor.x, clearColor.y, clearColor.z);
	TTK::Graphics::ClearScreen();
	TTK::Graphics::SetCameraMode3D(TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);
	camera.update();

	Renderer::ViewMatrix = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.forwardVector, camera.upVector);
	Renderer::ProjectionMatrix = glm::perspectiveFov(70.0f, (float)TTK::Graphics::ScreenWidth, (float)TTK::Graphics::ScreenHeight, 0.01f, 1000.0f);
	Renderer::WorldTransform = glm::mat4(1.0f);

	TTK::Graphics::DrawGrid();

	// perform physics calculations for each particle
	if (EditorSettings::IsPlaying)
		particleEffect.Update(deltaTime * EditorSettings::PlaybackSpeed * 0.01);

	// draw the particles
	particleEffect.Draw();

	Renderer::Flush();
	
	// You must call this prior to using any imgui functions
	TTK::Graphics::StartUI(windowWidth, windowHeight);

	ImGuiIO& io = ImGui::GetIO();

	if (io.KeysDown['w'])
		camera.moveForward();
	if (io.KeysDown['s'])
		camera.moveBackward();
	if (io.KeysDown['a'])
		camera.moveRight();
	if (io.KeysDown['d'])
		camera.moveLeft();
	if (io.KeysDown[128 + 114])
		camera.moveDown();
	if (io.KeysDown[' '])
		camera.moveUp();
	if (io.KeysDown[128 + GLUT_KEY_LEFT])
		camera.applyYaw(800.0f / TTK::Graphics::ScreenWidth);
	if (io.KeysDown[128 + GLUT_KEY_RIGHT])
		camera.applyYaw(-800.0f / TTK::Graphics::ScreenWidth);
	if (io.KeysDown[128 + GLUT_KEY_UP])
		camera.applyPitch(600.0f / TTK::Graphics::ScreenHeight);
	if (io.KeysDown[128 + GLUT_KEY_DOWN])
		camera.applyPitch(-600.0f / TTK::Graphics::ScreenHeight);

	camera.recalculateVectors();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load")) {
				OpenFileDlg.Show = true;
				if (SaveFileDlg.Show)
					SaveFileDlg.Show = false;
			}
			if (ImGui::MenuItem("Save")) {
				SaveFileDlg.Show = true;
				if (OpenFileDlg.Show)
					OpenFileDlg.Show = false;
			}
			if (ImGui::MenuItem("Create new effect")) {
				EditorSettings::EffectSettings = ParticleEffectSettings();
				particleEffect.ReplaceSettings(EditorSettings::EffectSettings);
				EditorSettings::ActiveEditLayer = nullptr;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Texture Manager")) {
				TexManager.IsVisible = true;
			}
			ImGui::Separator();
			ImGui::Text("Background:");
			ImGui::ColorEdit3("", &clearColor[0]);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	// Display the load effect path
	if (ImGui::Begin("Effect Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){

		/*
		ImGui::BeginChild("Foo", ImVec2(100, 100));
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(100, 100));
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, NULL))
			std::cout << "-_-" << std::endl;
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		for (int ix = 0; ix < 100; ix++) {
			glm::vec2 pos0 = 
				//Math::SolveBezier(glm::vec2(0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(1.5f, 0.0f), glm::vec2(1.0f, 1.0f), (1.0f / 10.0f) * ix)
				curve.Solve((1.0f / 100.0f) * ix) 
				* 50.0f + glm::vec2(50.0f);
			glm::vec2 pos1 =
				//Math::SolveBezier(glm::vec2(0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(1.5f, 0.0f), glm::vec2(1.0f, 1.0f), (1.0f / 10.0f) * (ix + 1))
				curve.Solve((1.0f / 100.0f) * (ix + 1)) 
				* 50.0f + glm::vec2(50.0f);
			uint32_t col = 0xFFFFFFFF;
			drawList->AddLine(
				ImVec2(bb.Min.x + pos0.x, bb.Max.y - pos0.y), 
				ImVec2(bb.Min.x + pos1.x, bb.Max.y - pos1.y), col, 1.0f);
		}
		ImGui::EndChild();
		*/
		
		ImGui::Separator();

		// Editor controls
		if (ImGui::CollapsingHeader("Editor Controls")) {
			ImGui::Checkbox("Debug Paths", &EditorSettings::DebugPaths);
			ImGui::Checkbox("Playback Enabled", &EditorSettings::IsPlaying);
			ImGui::SameLine();
			if (ImGui::Button("Restart")) {
				particleEffect.Restart();
			}
			ImGui::SliderFloat("Playback Speed", &EditorSettings::PlaybackSpeed, 0.0f, 200.0f, "%.2f%%", 1.0f, 100.0f);
		}

		/*
		ImGui::Separator();

		static ImVec2 points[5];
		ImGui::Curve("", ImVec2(300, 150), 10, points);
		float foo = ImGui::CurveValueSmooth(0.5f, 10, points);
		*/

		ImGui::Separator();

		DisplayEffectConfig(EditorSettings::EffectSettings);

		ImGui::End();
	}

	TexManager.Display();
	
	DisplayLayerConfig(EditorSettings::ActiveEditLayer);

	if (OpenFileDlg.DrawDialog() == 1) {
		LoadEffect(OpenFileDlg.GetFile().c_str());
	}
	if (SaveFileDlg.DrawDialog() == 1) {
		SaveEffect(SaveFileDlg.GetFile().c_str());
	}
	
	// You must call this once you are done doing UI stuff
	// This is what actually draws the ui on screen
	TTK::Graphics::EndUI();

	// Swap buffers
	// This is how we tell the program to put the things we just drew on the screen
	glutSwapBuffers();
}

/* function void KeyboardCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is pressed
*/
void KeyboardCallbackFunction(unsigned char key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = true;
	
	io.AddInputCharacter(key);

	switch (key)
	{
	case 27: // the escape key
		glutExit();
		break;

	case 'q': // the 'q' key
	case 'Q':
		// ...
		break;
	}
}

/* function void KeyboardUpCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is lifted
*/
void KeyboardUpCallbackFunction(unsigned char key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = false;

	switch (key) 
	{
	default:
		break;
	}
}

/* function TimerCallbackFunction(int value)
* Description:
*  - this is called many times per second
*  - this enables you to animate things
*  - no drawing, just changing the state
*  - changes the frame number and calls for a redisplay
*  - FRAME_DELAY is the number of milliseconds to wait before calling the timer again
*/
void TimerCallbackFunction(int value)
{
	// Calculate the amount of time since the last frame
	static int elapsedTimeAtLastTick = 0;
	int totalElapsedTime = glutGet(GLUT_ELAPSED_TIME);

	deltaTime = (float)totalElapsedTime - elapsedTimeAtLastTick;
	deltaTime /= 1000.0f;
	elapsedTimeAtLastTick = totalElapsedTime;

	// Re-trigger the display event
	glutPostRedisplay();

	/* this call gives it a proper frame delay to hit our target FPS */
	glutTimerFunc(FRAME_DELAY, TimerCallbackFunction, 0);
}

/* function WindowReshapeCallbackFunction()
* Description:
*  - this is called whenever the window is resized
*  - and sets up the projection matrix properly
*/
void WindowReshapeCallbackFunction(int w, int h)
{
	/* Update our Window Properties */
	windowWidth = w;
	windowHeight = h;

	TTK::Graphics::ScreenWidth = w;
	TTK::Graphics::ScreenHeight = h;
	
	particleEffect.ResizeFbo(TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);

	//Renderer::WorldTransform = glm::translate(glm::vec3(0, 0, 0.5f));
	//Renderer::ProjectionMatrix = glm::orthoLH(0.0f, (float)TTK::Graphics::ScreenWidth, 0.0f, (float)TTK::Graphics::ScreenHeight, 0.0f, 1000.0f);
	Renderer::ProjectionMatrix = glm::perspectiveFov(glm::radians(70.0f), (float)TTK::Graphics::ScreenWidth, (float)TTK::Graphics::ScreenHeight, 0.1f, 1000.0f);
}

// This is called when a mouse button is clicked
void MouseClickCallbackFunction(int button, int state, int x, int y)
{
	switch (button) {
		case 0: // Left mouse
			ImGui::GetIO().MouseDown[0] = !state;
			break;
		case 1: // Middle mouse
			ImGui::GetIO().MouseDown[2] = !state;
			break;
		case 2: // Right mouse
			ImGui::GetIO().MouseDown[1] = !state;
			break;
		case 3: // Scroll up
			ImGui::GetIO().MouseWheel = 0.5f;
			break;
		case 4: // Scroll down
			ImGui::GetIO().MouseWheel = -0.5f;
			break;
		default:
			break;

	}

	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;

}

void SpecialInputCallbackFunction(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[128 + key] = true;
}

void SpecialInputUpCallbackFunction(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[128 + key] = false;
}

// Called when the mouse is clicked and moves
void MouseMotionCallbackFunction(int x, int y)
{
	ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);
	
	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;
}

// Called when the mouse is moved inside the window
void MousePassiveMotionCallbackFunction(int x, int y)
{
	ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;
}

/* function main()
* Description:
*  - this is the main function
*  - does initialization and then calls glutMainLoop() to start the event handler
*/
int main(int argc, char **argv)
{
	/* initialize the window and OpenGL properly */

	TTK::Graphics::ScreenWidth = windowWidth;
	TTK::Graphics::ScreenHeight = windowHeight;

	// Request an OpenGL 4.5 compatibility
	// A compatibility context is needed to use the provided rendering utilities 
	// which are written in OpenGL 1.1
	glutInitContextVersion(4, 4); 
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Tutorial");

	//Init GLEW
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "TTK::InitializeTTK Error: GLEW failed to init" << std::endl;
	}
	printf("OpenGL version: %s, GLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	/* set up our function callbacks */
	glutDisplayFunc(DisplayCallbackFunction);
	//glutDisplayFunc(TestDisplayCallback);	
	glutKeyboardFunc(KeyboardCallbackFunction);
	glutKeyboardUpFunc(KeyboardUpCallbackFunction);
	glutReshapeFunc(WindowReshapeCallbackFunction);
	glutMouseFunc(MouseClickCallbackFunction);
	glutMotionFunc(MouseMotionCallbackFunction);
	glutPassiveMotionFunc(MousePassiveMotionCallbackFunction);
	glutSpecialFunc(SpecialInputCallbackFunction);	
	glutSpecialUpFunc(SpecialInputUpCallbackFunction);
	glutTimerFunc(1, TimerCallbackFunction, 0);

	curve = AdaptiveCurve<glm::vec2>([](float t) {
		return Math::SolveBezier(glm::vec2(0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(1.5f, 0.0f), glm::vec2(1.0f, 1.0f), t);
	});

	curve.Bake(0.000001f);

	glm::vec2 t = curve.Solve(0.3f);

	camera = TTK::Camera();
	camera.cameraPosition = glm::vec3(5.0f);
	camera.forwardVector = glm::normalize(glm::vec3(0.0f) - glm::vec3(5.0f));

	// Init GL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/* Start Game Loop */
	deltaTime = (float)glutGet(GLUT_ELAPSED_TIME);
	deltaTime /= 1000.0f;

	InitializeScene();

	// Init IMGUI
	TTK::Graphics::InitImGUI();

	Renderer::Init(); 

	glutMainLoop();

	getch();

	return 0;
}