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

ParticleEmitter emitter;

Texture2D textureHandle;

void InitializeScene()
{
	LayerConfig config;

	// Physics properties
	config.Velocity0   = glm::vec3(-120.0f,  120.0f, 0.0f);
	config.Velocity1   = glm::vec3( 120.0f,  120.0f, 0.0f);
	config.MassRange   = glm::vec2(0.5f, 0.75f);
	config.Position    = mousePositionFlipped;

	// Visuals
	config.InitColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	config.FinalColor = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	config.LifeRange = glm::vec2(0.0f, 10.0f);
	config.SizeRange = glm::vec2(15.0f, 25.0f);

	emitter.Settings.Config = config;

	// Create the particles
	emitter.initialize(MAX_PARTICLES_PER_LAYER);

	textureHandle = Texture2D();
	textureHandle.loadTextureFromFile("res/snow.png");
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

// This is where we draw stuff
void DisplayCallbackFunction(void)
{
	// Set up scene
	TTK::Graphics::SetBackgroundColour(0.5f, 0.5f, 0.5f);
	TTK::Graphics::ClearScreen();
	TTK::Graphics::SetCameraMode2D(windowWidth, windowHeight);

	// Apply forces on the particle system
    if (applySeekingForce)
		applyForcesToParticleSystem(&emitter, glm::vec3(windowHeight*0.5f, windowWidth*0.5, 0.0f));

	// perform physics calculations for each particle
	emitter.update(deltaTime);

	TTK::Graphics::BeginPointSprites(textureHandle);
	// draw the particles
	emitter.draw();
	TTK::Graphics::EndPointSprites();

	// IMGUI EXAMPLE  -----

	// You must call this prior to using any imgui functions
	TTK::Graphics::StartUI(windowWidth, windowHeight);
	
	// Draw a simple label, this is the same as a "printf" call
	ImGui::Text("Particle Options. Number of particles = %d", emitter.getNumParticles());

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

	if (ImGui::Bezier("Foo", dataPoints)) {

	}

	ImGui::Checkbox("Playback Enabled", &emitter.playing);
	ImGui::Checkbox("Toggle steering force", &applySeekingForce);

	// Color control
	// Tip: You can click and drag the numbers in the UI to change them
	ImGui::ColorEdit4("Start Color", &emitter.Settings.Config.InitColor[0]);
	ImGui::ColorEdit4("End Color", &emitter.Settings.Config.FinalColor[0]);

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
	ImGui::SliderFloat("Particle Life", &emitter.Settings.Config.LifeRange.x, 0.0f, 50.0f);
	int numParticles = emitter.getNumParticles();
	ImGui::SliderInt("Emission Rate", &numParticles, 0.0f, MAX_PARTICLES_PER_LAYER);
	emitter.setNumParticles((unsigned int)numParticles);
	
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

	emitter.Settings.Config.Position = mousePositionFlipped;
}

/* function main()
* Description:
*  - this is the main function
*  - does initialization and then calls glutMainLoop() to start the event handler
*/
int main(int argc, char **argv)
{
	/* initialize the window and OpenGL properly */

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