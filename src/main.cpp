// Shaun McKinnon - 100642799 //

#define GLEW_STATIC
#include "glew/glew.h"
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "FreeImage.lib")

#include "GameObject.h"
#include "AnimationMath.h"
#include "ParticleEmitter.h"

// Core Libraries (std::)
#include <iostream>
#include <string>
#include <math.h>
#include <vector>

// 3rd Party Libraries
#include <GLUT/freeglut.h>
#include <TTK/GraphicsUtils.h>
#include <TTK/Camera.h>
#include "imgui/imgui.h"
#include "imgui/imgui_bezier.h"
#include "GLM/glm.hpp"

#include "TTK\Texture2D.h"

#include "ParticleEffect.h"
#include "TextureCollection.h"

#include <fstream>
#include <iostream>

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

void InitializeScene()
{	
	LayerConfig config;
	// Physics properties
	config.Velocity0     = glm::vec3(-120.0f,  120.0f, 0.0f);
	config.Velocity1     = glm::vec3( 120.0f,  120.0f, 0.0f);
	config.VelocityRange = glm::vec2(75.0f, 100.0f);
	config.MassRange     = glm::vec2(0.5f, 0.75f);
	config.Position      = mousePositionFlipped;

	// Visuals
	config.BlendMode     = ParticleBlend::BlendAdditive;
	config.InitColor  = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	config.FinalColor = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	config.LifeRange  = glm::vec2(0.0f, 10.0f);
	config.SizeRange  = glm::vec2(25.0f, 40.0f);
	config.TextureID  = 1;

	ParticleLayerSettings settings = ParticleLayerSettings();
	settings.Config = config;
	
	particleEffect.AddLayer(settings);
	
	//std::fstream stream;
	//stream.open("test.dat");
	//particleEffect.WriteToFile(stream);
	//particleEffect = ParticleEffect::ReadFromFile(stream);
	//stream.close();

	particleEffect.Init();

	TextureCollection::LoadTexture(1, "res/snow.png");
}

// These values are controlled by imgui
bool applySeekingForce = true;
float seekingForceScale = 100.0f;
float minSeekingForceScale = -200.0f;
float maxSeekingForceScale = 200.0f; 

void applyForcesToParticleSystem(ParticleEmitter* e, glm::vec3 target)
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

void DisplayLayerConfig(ParticleLayerSettings& settings) {
	static bool isWindowShowing = true;

	if (ImGui::Begin("Layer Config", &isWindowShowing, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize)) {
		
		ImGui::Text("Physics:");
		ImGui::DragFloat3(" Position", &settings.Config.Position[0]);
		ImGui::DragFloat3(" Gravity", &settings.Config.Gravity[0]);
		ImGui::DragFloat(" Duration", &settings.Config.Duration);
		ImGui::Combo(" Velocity Type", (int*)&settings.Config.VelocityType, "Cone/Line\0Box\0\0");
		ImGui::DragFloat3(" Velocity 0", &settings.Config.Velocity0[0]);
		ImGui::DragFloat3(" Velocity 1", &settings.Config.Velocity1[0]);
		ImGui::DragFloat2(" Velocity Range", &settings.Config.VelocityRange[0]);
		ImGui::DragFloat2(" Mass Range", &settings.Config.MassRange[0]);

		ImGui::Separator();
		ImGui::Text("Emission:");
		ImGui::SliderFloat(" Emission Rate", &settings.Config.EmissionRate, 0, settings.Config.MaxParticles);
		ImGui::DragFloat2(" Life Range", &settings.Config.LifeRange[0]);
		ImGui::DragFloat2(" Size Range", &settings.Config.SizeRange[0]);
		ImGui::Combo(" Emitter Type", (int*)&settings.Config.BoundsType, "Point\0Box\0Circle\0Line\0\0");
		ImGui::DragFloat3(" Emitter Meta", &settings.Config.BoundsMeta[0]);
		int maxPart = settings.Config.MaxParticles;
		ImGui::DragInt(" Max Particles", &maxPart, 100, 5000);
		settings.Config.MaxParticles = maxPart;

		// TODO: textures

		ImGui::Separator();
		ImGui::Text("Visuals:");
		ImGui::ColorEdit4("Start Color", &settings.Config.InitColor[0]);
		ImGui::ColorEdit4("Final Color", &settings.Config.FinalColor[0]);
		ImGui::Checkbox(" Interpolate Color", &settings.Config.InterpolateColor);
		ImGui::Combo(" Blend Mode", (int*)&settings.Config.BlendMode, "Multiply\0Additive\0\0");
	
		ImGui::End();
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
	particleEffect.Update(deltaTime);

	// draw the particles
	particleEffect.Draw();

	// IMGUI EXAMPLE  -----
	

	// You must call this prior to using any imgui functions
	TTK::Graphics::StartUI(windowWidth, windowHeight);
	
	// Draw a simple label, this is the same as a "printf" call
	//ImGui::Text("Particle Options. Number of particles = %d", emitter.getNumParticles());

	// Button, when button is clicked the code in the block is executed
	//if (ImGui::Button("Toggle emitter playback"))
	//{
	//	std::cout << "playback clicked. " << std::endl;
	//	emitter.playing = !emitter.playing;
	//}

	static float dataPoints[4] = {
		0.0f, 1.0f,
		1.0f, 0.0f
	};

	DisplayLayerConfig(particleEffect.Layers[0]->Settings);

	if (ImGui::Bezier("Foo", dataPoints)) {

	}

	//ImGui::Checkbox("Playback Enabled", &emitter.playing);
	ImGui::Checkbox("Toggle steering force", &applySeekingForce);

	// Color control
	// Tip: You can click and drag the numbers in the UI to change them
	//ImGui::ColorEdit4("Start Color", &emitter.Settings.Config.InitColor[0]);
	//ImGui::ColorEdit4("End Color", &emitter.Settings.Config.FinalColor[0]);

	// Example of a slider
	// As you drag the slider the variable passed by reference gets modified
	if (applySeekingForce)
		ImGui::SliderFloat("Slider", &seekingForceScale, minSeekingForceScale, maxSeekingForceScale);

	// imgui has TONS of UI functions
	// Uncomment these two lines if you want to see a full imgui demo
	// This is about the best documentation available for imgui.
	// If you see some functionality in the demo you would like to use, go into 
	// the ShowTestWindow function and pick it apart. 
	// Note: keyboard input is currently broken, if you get it working please let the TA know :)
	//static bool drawWindow = true;
	//ImGui::ShowTestWindow(&drawWindow);
	//ImGui::SliderFloat("Particle Life", &emitter.Settings.Config.LifeRange.x, 0.0f, 50.0f);
	//int numParticles = emitter.getNumParticles();
	//ImGui::SliderInt("Emission Rate", &numParticles, 0.0f, MAX_PARTICLES_PER_LAYER);
	//emitter.setNumParticles((unsigned int)numParticles);
	
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
}

// This is called when a mouse button is clicked
void MouseClickCallbackFunction(int button, int state, int x, int y)
{
	if (button < 3)
		ImGui::GetIO().MouseDown[button] = !state;
	else if (button == 3)
		ImGui::GetIO().MouseWheel =  0.5f;
	else if (button == 4)
		ImGui::GetIO().MouseWheel = -0.5f;

	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;

}

void SpecialInputCallbackFunction(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		// ...
		break;
	case GLUT_KEY_DOWN:
		// ...
		break;
	case GLUT_KEY_LEFT:
		// ...
		break;
	case GLUT_KEY_RIGHT:
		// ...
		break;
	}
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

	// Request an OpenGL 4.4 compatibility
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
	glutKeyboardFunc(KeyboardCallbackFunction);
	glutKeyboardUpFunc(KeyboardUpCallbackFunction);
	glutReshapeFunc(WindowReshapeCallbackFunction);
	glutMouseFunc(MouseClickCallbackFunction);
	glutMotionFunc(MouseMotionCallbackFunction);
	glutPassiveMotionFunc(MousePassiveMotionCallbackFunction);
	glutTimerFunc(1, TimerCallbackFunction, 0);
	glutSpecialFunc(SpecialInputCallbackFunction);

	// Init GL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/* Start Game Loop */
	deltaTime = (float)glutGet(GLUT_ELAPSED_TIME);
	deltaTime /= 1000.0f;

	InitializeScene();

	// Init IMGUI
	TTK::Graphics::InitImGUI();

	glutMainLoop();

	return 0;
}