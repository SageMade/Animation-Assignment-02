#define GLEW_STATIC
#include "glew/glew.h"
#include "TTK/GraphicsUtils.h"
#include <windows.h>
#include "GLUT/glut.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl.h"

uint32_t TTK::Graphics::ScreenWidth;
uint32_t TTK::Graphics::ScreenHeight;

void TTK::Graphics::InitImGUI()
{
	TTK::internal::imguiInit();

	ImGuiIO& io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_LeftArrow]  = 128 + GLUT_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = 128 + GLUT_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow]    = 128 + GLUT_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow]  = 128 + GLUT_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp]     = 128 + GLUT_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown]   = 128 + GLUT_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home]       = 128 + GLUT_KEY_HOME;
	io.KeyMap[ImGuiKey_End]        = 128 + GLUT_KEY_END;
	io.KeyMap[ImGuiKey_Escape]     = 128 + GLUT_KEY_PAGE_DOWN;

	io.KeyMap[ImGuiKey_Enter]      = '\n';
	io.KeyMap[ImGuiKey_Tab]        = '\t';
	io.KeyMap[ImGuiKey_Backspace]  = '\b';
	io.KeyMap[ImGuiKey_Delete]     = 127;
}

void TTK::Graphics::StartUI(int windowWidth, int windowHeight)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = windowWidth;
	io.DisplaySize.y = windowHeight;
	ImGui::NewFrame();
}

void TTK::Graphics::EndUI()
{
	ImGui::Render();
}

void TTK::Graphics::DrawText2D(std::string text, int posX, int posY)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
		glRasterPos2f(posX, posY+24); // Plus 24 because thats the size of the text
		for (unsigned int i = 0; i < text.length(); i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
		}
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void TTK::Graphics::FullscreenQuad(const GLuint texHandle) {

	GLboolean tex2DEnabled, texEnabled;
	glGetBooleanv(GL_TEXTURE_2D, &tex2DEnabled);
	glGetBooleanv(GL_TEXTURE, &texEnabled);
	glEnable(GL_TEXTURE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texHandle);

	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);

	glColor3f(1.0f, 1.0f, 1.0f);  // Green
	glVertex2f(-1.0f,  1.0f);  // Top-right of top face
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f( 1.0f,  1.0f);  // Top-left of top face
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(-1.0f, -1.0f);  // Bottom-left of top face
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f( 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);

	glEnd();

	glPopMatrix();

	if (!tex2DEnabled)
		glDisable(GL_TEXTURE_2D);
	if (!texEnabled)
		glDisable(GL_TEXTURE);
}

void TTK::Graphics::EnableAlphaBlend() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}

void TTK::Graphics::EnableAdditiveBlend() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
}

void TTK::Graphics::BeginPointSprites(const Texture2D& texture) {
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_POINT_SPRITE);
}

void TTK::Graphics::EndPointSprites() {
	glDisable(GL_POINT_SPRITE);
}

 void TTK::Graphics::DrawLine(glm::vec3 p0, glm::vec3 p1, float lineWidth, glm::vec4 colour)
 {
 	DrawLine(&p0[0], &p1[0], lineWidth, &colour[0]);
 }

void TTK::Graphics::DrawLine(float* p0, float* p1, float lineWidth, float *colour)
{
	GLboolean lightingEnabled;
	glGetBooleanv(GL_LIGHTING, &lightingEnabled);

	glDisable(GL_LIGHTING);

	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	if (colour != nullptr)
		glColor3fv(colour);
	else
		glColor3f(0.0f, 0.0f, 0.0f);

		glVertex3fv(p0);
		glVertex3fv(p1);
	glEnd();
	glLineWidth(1.0f);

	
	if (lightingEnabled)
		glEnable(GL_LIGHTING);
}

void TTK::Graphics::DrawVector(float* origin, float* vectorToDraw, float lineWidth, float *colour)
{
	GLboolean lightingEnabled;
	glGetBooleanv(GL_LIGHTING, &lightingEnabled);

	glDisable(GL_LIGHTING);

	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	if (colour != nullptr)
		glColor3fv(colour);
		glVertex3fv(origin);
		glVertex3f(origin[0] + vectorToDraw[0], origin[1] + vectorToDraw[1], origin[2] + vectorToDraw[2]);
	glEnd();
	glLineWidth(1.0f);

	if (lightingEnabled)
		glEnable(GL_LIGHTING);
}

void TTK::Graphics::DrawVector(glm::vec3 origin, glm::vec3 vectorToDraw, float lineWidth, glm::vec3 colour)
{
	DrawVector(&origin[0], &vectorToDraw[0], lineWidth, &colour[0]);
}

void TTK::Graphics::DrawPoint(glm::vec3 p0, float pointSize, glm::vec4 colour)
{
	DrawPoint(&p0[0], pointSize, &colour[0]);
}

void TTK::Graphics::DrawPoint(float *p0, float pointSize, float *colour)
{
	GLboolean lightingEnabled;
	glGetBooleanv(GL_LIGHTING, &lightingEnabled);
	glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);

	glPointSize(pointSize);
	glBegin(GL_POINTS);

	if (colour != nullptr)
		glColor4fv(colour);
	else
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

		glVertex3fv(p0);
	glEnd();

	//glEnable(GL_TEXTURE_2D);

	if (lightingEnabled)
		glEnable(GL_LIGHTING);
}

void TTK::Graphics::DrawCube(glm::vec3 p0, float size, glm::vec4 colour)
{
	DrawCube(&p0[0], size, &colour[0]);
}

void TTK::Graphics::DrawCube(float *p0, float size, float *colour)
{
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLboolean lightingEnabled;
	glGetBooleanv(GL_LIGHTING, &lightingEnabled); 

	glDisable(GL_LIGHTING);

	if (colour != nullptr)
		glColor4fv(colour);


	glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glTranslatef(*(p0), *(p0 + 1), *(p0 + 2));
			glutSolidCube(size);
		glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (lightingEnabled)
		glEnable(GL_LIGHTING);
}


void TTK::Graphics::DrawTeapot(glm::vec3 p0, float size, glm::vec4 colour)
{
	DrawTeapot(&p0[0], size, &colour[0]);
}

void TTK::Graphics::DrawTeapot(float *p0, float size /*= 1.0f*/, float *colour /*= nullptr*/)
{
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (colour != nullptr)
		glColor4fv(colour);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(*(p0), *(p0 + 1), *(p0 + 2));
	glutSolidTeapot(size);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void TTK::Graphics::DrawSphere(glm::mat4 p0, float size, glm::vec4 colour)
{
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4fv(&colour[0]);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(&p0[0][0]);
	glutSolidSphere(size, 4, 4);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void TTK::Graphics::SetCameraMode2D(int windowWidth, int windowHeight)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, windowWidth, windowHeight);
	//gluOrtho2D(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);

	glOrtho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -100.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
}

 void TTK::Graphics::SetCameraMode3D(int windowWidth, int windowHeight)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, windowWidth, windowHeight);
	gluPerspective(60.0f, (float)windowWidth / (float)windowHeight, 0.001f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
}

void TTK::Graphics::SetBackgroundColour(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}

void TTK::Graphics::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// some code i wrote back in second year...
// no idea whats happening but it works
void TTK::Graphics::DrawGrid()
{
	glPushMatrix();
	glScalef(0.1f, 0.1f, 0.1f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_LINES);

	float tempX = -100, tempY = 100, tempZ = 0;
	int xLines = 0, Ylines = 0, Zlines = 0;
	for (int i = 0; i <= 40; i++)
	{
		if (i <= 20)
		{
			if (i == 10)
			{
				glLineWidth(50.0);
				glColor3f(0, 0, 1);
			}
			else
			{
				glLineWidth(0.01);
				glColor3f(0, 0, 0);
			}
			tempY = 100;
			glVertex3f(tempX, tempY, tempZ);
			glVertex3f(tempX, -100, tempZ);
			tempX += 10;
		}
		if (i == 20)
		{
			tempX = 100;
			tempY = -100;
		}
		if (i >= 20)
		{
			if (i == 30)
			{
				glLineWidth(50.0);
				glColor3f(1, 0, 0);
			}
			else
			{
				glLineWidth(0.01);
				glColor3f(0, 0, 0);
			}
			glVertex3f(tempX, tempY, tempZ);
			glVertex3f(-100, tempY, tempZ);
			tempY += 10;
		}
	}

	// 	glColor3f(0, 1, 0);
	// 	glVertex3f(0, 0.0f, 0.0f);
	// 	glVertex3f(0, 0.0f, -50.0f);
	// 	glColor3f(0, 0, 0);

	glEnd();
	glPopMatrix();

}