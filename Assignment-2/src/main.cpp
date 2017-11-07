
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

#include <filesystem>
namespace fs = std::experimental::filesystem;

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


struct EditorSettings {
	ParticleLayerSettings  *ActiveEditLayer = nullptr;
	ParticleEffectSettings  EffectSettings;

	bool                    IsPlaying = true;
	float                   PlaybackSpeed = 100.0f;
} Settings;

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

FileDialog OpenFileDlg;
FileDialog SaveFileDlg;

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

	// Dont't forget the null terminator!
	memcpy(inFileName, "test.dat", 9);
	memcpy(outFileName, "test.dat", 9);

	Settings.EffectSettings = ParticleEffectSettings();

	LayerConfig config;
	memcpy(config.Name, "Default", 8);

	ParticleLayerSettings settings = ParticleLayerSettings();
	settings.Config = config;

	particleEffect.AddLayer(settings);

	Settings.EffectSettings.Layers.push_back(particleEffect.Layers.back()->Settings);

	particleEffect.Origin = glm::vec3(TTK::Graphics::ScreenWidth / 2.0f, TTK::Graphics::ScreenHeight / 2.0f, 0.0f);
		
	//std::fstream stream;
	//stream.open("test.dat");
	//particleEffect.WriteToFile(stream);
	//particleEffect = ParticleEffect::ReadFromFile(stream);
	//stream.close();

	particleEffect.Init();

	TextureCollection::LoadTexture(1, "textures/snow.png");
	TextureCollection::LoadTexture(2, "textures/flare.png");
	
	//Renderer::SetTexture(0, TextureCollection::Get(1).id());
	Renderer::SetTexture(1, TextureCollection::Get(2).id());

    WindowReshapeCallbackFunction(TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);
}

// These values are controlled by imgui
bool applySeekingForce = true;
bool followMouse = false;
float seekingForceScale = 100.0f;
float minSeekingForceScale = -200.0f;
float maxSeekingForceScale = 200.0f; 

void SaveEffect(const char* fileName) {
	if (fileName[0] != '\0') {
		std::fstream stream;
		stream.open(fileName, std::ios::out | std::ios::binary);
		if (stream.good()) {
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
			particleEffect = ParticleEffect::ReadFromFile(stream);
			particleEffect.Init();

			Settings.EffectSettings = ParticleEffectSettings();
			Settings.EffectSettings.Layers.reserve(particleEffect.Layers.size());
			memcpy(Settings.EffectSettings.Name, particleEffect.Name, EFFECT_NAME_MAX_LENGTH);
			for (int ix = 0; ix < particleEffect.Layers.size(); ix++) {
				Settings.EffectSettings.Layers.push_back(particleEffect.Layers[ix]->Settings);
			}

			if (!followMouse)
				particleEffect.Origin = glm::vec3(TTK::Graphics::ScreenWidth / 2.0f, TTK::Graphics::ScreenHeight / 2.0f, 0.0f);
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

struct CurveData {
	static const int CONTROL_COUNT = 10;

	ImVec2 Controls[10];

	CurveData() {
		Controls[0].x = -1;
	}

	float Evaluate(float t) {
		return ImGui::CurveValueSmooth(t, 10, Controls);
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
	ImGui::Combo(" Method", (int*)&behaviour.Method, "None\0Seek\0Flee\0Repel\0Attract\0Path\0\0");

	if (behaviour.Method != Unknown)
		ImGui::Separator();

	switch (behaviour.Method) {
		case Seek: {
			if (behaviour.MetaData == nullptr) {
				behaviour.MetaData = new SeekFleeData();
			}
			SeekFleeData& data = *reinterpret_cast<SeekFleeData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::Checkbox(" Local Space", &data.LocalSpace);
		}
		break;
		case Flee: {
			if (behaviour.MetaData == nullptr) {
				behaviour.MetaData = new SeekFleeData();
			}
			SeekFleeData& data = *reinterpret_cast<SeekFleeData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::Checkbox(" Local Space", &data.LocalSpace);

		}
		break;
		case Repel: {
			if (behaviour.MetaData == nullptr) {
				behaviour.MetaData = new MagnetData();
			}
			MagnetData& data = *reinterpret_cast<MagnetData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Point", &data.Force);
			ImGui::Checkbox(" Local Space", &data.LocalSpace);
		}
		break;
		case Attract: {
			if (behaviour.MetaData == nullptr) {
				behaviour.MetaData = new MagnetData();
			}
			MagnetData& data = *reinterpret_cast<MagnetData*>(behaviour.MetaData);
			ImGui::DragFloat3(" Point", &data.Point[0]);
			ImGui::DragFloat(" Point", &data.Force);
			ImGui::Checkbox(" Local Space", &data.LocalSpace);
		}
		break;
		case Path: {
			if (behaviour.MetaData == nullptr) {
				behaviour.MetaData = new PathData();
			}
			PathData& data = *reinterpret_cast<PathData*>(behaviour.MetaData);			
			ImGui::Checkbox(" Local Space", &data.LocalSpace);
			ImGui::Combo(" Mode", (int*)&data.LoopMode, "Loop\0Reverse\0Stop\0\0");
			DisplayEditablePointList(data.Points);
		}
		break;
		default:
			break;
	}
}

void DisplayLayerConfig(ParticleLayerSettings *settingsPtr) {

	bool show = Settings.ActiveEditLayer != nullptr;

	if (show) {
		ParticleLayerSettings &settings = *settingsPtr;

		if (ImGui::Begin(settings.Config.Name, &show, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::BeginChild((ImGuiID)0, ImVec2(280, 1));
			ImGui::EndChild();

			if (ImGui::CollapsingHeader("Physics:")) {
				ImGui::DragFloat3(" Position", &settings.Config.Position[0]);
				ImGui::DragFloat3(" Gravity", &settings.Config.Gravity[0]);
				ImGui::Combo(" Velocity Type", (int*)&settings.Config.VelocityType, "Cone/Line\0Box\0\0");
				ImGui::DragFloat3(" Velocity 0", &settings.Config.Velocity0[0]);
				ImGui::DragFloat3(" Velocity 1", &settings.Config.Velocity1[0]);
				ImGui::DragFloat2(" Angular Range", &settings.Config.AngularSpeedRange[0]);
				ImGui::DragFloat2(" Velocity Range", &settings.Config.VelocityRange[0]);
				ImGui::DragFloat2(" Mass Range", &settings.Config.MassRange[0], 0.1f, 0.01f, 1000.0f);
			}

			if (ImGui::CollapsingHeader("Emission:")) {
				static CurveData emissionOverTime = CurveData();
				ImGui::Curve("Emission", ImVec2(250, 150), CurveData::CONTROL_COUNT, emissionOverTime.Controls);
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

				// TODO: textures
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
					if (ImGui::TreeNode(settings.Behaviours[ix].Name)) {
						if (ImGui::Button("Remove")) {
							settings.Behaviours.erase(settings.Behaviours.begin() + ix);
							ix--;
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
		Settings.ActiveEditLayer = nullptr;
}

void DisplayEffectConfig(ParticleEffectSettings& settings) {
	ImGui::Text("Effect Config");
	ImGui::InputText(" Name", settings.Name, 32);
	ImGui::Separator();
	ImGui::Text("Layers: ");
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));
	if (ImGui::Button("+")) {
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
		ImGui::InputText("Name", settings.Layers[ix].Config.Name, MAX_LAYER_NAME_SIZE);
		if (ImGui::Button("Edit")) {
			Settings.ActiveEditLayer = &particleEffect.Layers[ix]->Settings;
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			settings.Layers.erase(settings.Layers.begin() + ix);
			particleEffect.ReplaceSettings(Settings.EffectSettings);
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
}

// This is where we draw stuff
void DisplayCallbackFunction(void)
{
	// Set up scene
	TTK::Graphics::SetBackgroundColour(0.0f, 0.0f, 0.0f);
	TTK::Graphics::ClearScreen();
	TTK::Graphics::SetCameraMode2D(windowWidth, windowHeight);

	// Apply forces on the particle system
    //if (applySeekingForce)
	//	applyForcesToParticleSystem(&emitter, glm::vec3(windowHeight*0.5f, windowWidth*0.5, 0.0f));
	
	// perform physics calculations for each particle
	if (Settings.IsPlaying)
		particleEffect.Update(deltaTime * Settings.PlaybackSpeed * 0.01);

	// draw the particles
	particleEffect.Draw();

	Renderer::Flush();

	/*
	for (int ix = 0; ix < curve.SampleCount() - 1; ix++) {
		TTK::Graphics::DrawLine(glm::vec3(400, 400, 0) + curve.SampleAtIndex(ix).xyy * 30.0f, 
			glm::vec3(400, 400, 0) + curve.SampleAtIndex(ix + 1).xyy * 30.0f, 1.0f, glm::vec4(1.0f));
	}
	*/

	// You must call this prior to using any imgui functions
	TTK::Graphics::StartUI(windowWidth, windowHeight);
	
	// Display the load effect path
	if (ImGui::Begin("Effect Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
		
		// Editor controls
		if (ImGui::CollapsingHeader("Editor Controls")) {
			ImGui::Checkbox("Playback Enabled", &Settings.IsPlaying);
			ImGui::SameLine();
			if (ImGui::Button("Restart")) {
				particleEffect.Restart();
			}
			ImGui::SliderFloat("Playback Speed", &Settings.PlaybackSpeed, 0.0f, 200.0f, "%.2f%%", 1.0f, 100.0f);

			ImGui::Checkbox("Follow Mouse", &followMouse);
			ImGui::DragFloat3("Effect Pos", &particleEffect.Origin[0]);
		}

		ImGui::Separator();

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.328f, 0.328f, 0.33f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.402f, 0.402f, 0.402f, 1.0f));


		if (ImGui::Button("Load")) {
			OpenFileDlg.Show = true;
			if (SaveFileDlg.Show)
				SaveFileDlg.Show = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			SaveFileDlg.Show = true;
			if (OpenFileDlg.Show)
				OpenFileDlg.Show = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Create new effect")) {
			Settings.EffectSettings = ParticleEffectSettings();
			particleEffect.ReplaceSettings(Settings.EffectSettings);

			if (!followMouse)
				particleEffect.Origin = glm::vec3(TTK::Graphics::ScreenWidth / 2.0f, TTK::Graphics::ScreenHeight / 2.0f, 0.0f);
		}

		ImGui::PopStyleColor(2);

		/*
		ImGui::Separator();

		static ImVec2 points[5];
		ImGui::Curve("", ImVec2(300, 150), 10, points);
		float foo = ImGui::CurveValueSmooth(0.5f, 10, points);
		*/

		ImGui::Separator();

		DisplayEffectConfig(Settings.EffectSettings);

		ImGui::End();
	}
	
	DisplayLayerConfig(Settings.ActiveEditLayer);

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

	if (!followMouse)
		particleEffect.Origin = glm::vec3(TTK::Graphics::ScreenWidth / 2.0f, TTK::Graphics::ScreenHeight / 2.0f, 0.0f);

	//Renderer::WorldTransform = glm::translate(glm::vec3(0, 0, 0.5f));
	Renderer::ProjectionMatrix = glm::orthoLH(0.0f, (float)TTK::Graphics::ScreenWidth, 0.0f, (float)TTK::Graphics::ScreenHeight, 0.0f, 1000.0f);
	//Renderer::ProjectionMatrix = glm::perspectiveFov(glm::radians(70.0f), (float)TTK::Graphics::ScreenWidth, (float)TTK::Graphics::ScreenHeight, 0.1f, 1000.0f);
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

	if (followMouse)
		particleEffect.Origin = mousePositionFlipped;
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
		return glm::vec2(sin(t * 3.14), cos(t * 3.14));
	});

	curve.Bake(0.0001f);

	glm::vec2 t = curve.Solve(0.3f);

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