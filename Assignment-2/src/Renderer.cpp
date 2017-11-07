#include "Renderer.h"
#include <GLEW/glew.h>
#include <windows.h>
#include "GLUT/glut.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "GLM.h"

uint32_t Renderer::myTextureHandles[RENDERER_MAX_TEXTURES];
int Renderer::myViewUniformLoc, Renderer::myWorldUniformLoc, Renderer::myTexturesUniformLoc, Renderer::myProjectionLoc;

uint32_t Renderer::myPrimitiveBuffer = 0;
uint32_t Renderer::myVao = 0;
uint16_t Renderer::myActiveParticleCount = 0;;
uint16_t Renderer::myPrimitiveBufferSize = 0;
		
ParticleVertex *Renderer::myVertexBufferData;

uint32_t Renderer::myShaderProgram;

uint8_t Renderer::myActiveTexture;

glm::mat4 Renderer::WorldTransform = glm::mat4();
glm::mat4 Renderer::ViewMatrix = glm::mat4();
glm::mat4 Renderer::ProjectionMatrix = glm::mat4();

glm::mat4 Renderer::myActiveMatrix = glm::mat4();
std::stack<glm::mat4> Renderer::myMatrixStack;

Texture2D Renderer::myDefaultTexture;

void Renderer::Init() {
    myVertexBufferData = new ParticleVertex[PARTICLE_BATCH_SIZE];

	const char *vertex_shader = R"LIT(#version 410
		uniform mat4 xWorld;
		struct VsVert {
			vec4  Color;
			float Size;
            float Angle;
			float TexId;
		};
        layout (location = 0) in vec3 Position;
        layout (location = 1) in vec4 Color;
        layout (location = 2) in float Size;
        layout (location = 3) in float Angle;
        layout (location = 4) in float TexId;
		out VsVert VsToGs;
		void main() {
			VsToGs.Color = Color;
			VsToGs.TexId = TexId;
			VsToGs.Size = Size;
            VsToGs.Angle = Angle;
			gl_Position = xWorld * vec4(Position, 1);
		})LIT";

	const char* geometry_shader = R"LIT(#version 410
		layout (points) in;
		layout (triangle_strip, max_vertices = 4) out;
		uniform mat4 xWorld;
		uniform mat4 xView;
		uniform mat4 xProjection;
		struct VsVert {
			vec4  Color;
			float Size;
            float Angle;
			float TexId;
		};
		struct GsVert {
			vec2  TexCoord;
			vec4  Color;
			float TexId;
		};
		in VsVert VsToGs[];

		out GsVert GsToFS;

		mat4 rotationMatrixZ(float angle)
		{ 
			float s = sin(angle);
			float c = cos(angle);
			float oc = 1.0 - c;
    
			mat4 rotateZ;
			rotateZ[0].x    = c;
			rotateZ[0].y    = -s;
			rotateZ[0].z    = 0.0;
			rotateZ[0].w    = 0.0;
			rotateZ[1].x    = s;
			rotateZ[1].y    = c;
			rotateZ[1].z    = 0.0;
			rotateZ[1].w    = 0.0;
			rotateZ[2].x    = 0.0;
			rotateZ[2].y    = 0.0;
			rotateZ[2].z    = 1.0;
			rotateZ[2].w    = 0.0;
			rotateZ[3].x    = 0.0;
			rotateZ[3].y    = 0.0;
			rotateZ[3].z    = 0.0;
			rotateZ[3].w    = 1.0;

            return rotateZ;
		}

		void main() {
			// Calculate our shader stuff (basically where in the view it is)
			mat4 MV = rotationMatrixZ(VsToGs[0].Angle) * xView * xWorld;
			mat4 VP = xProjection * xView;
			vec3 right = vec3(MV[0][0], 
			        MV[1][0], 
			        MV[2][0]);

			vec3 up = vec3(MV[0][1], 
			        MV[1][1], 
			        MV[2][1]);

			vec3 pos = gl_in[0].gl_Position.xyz;

			vec3 tl = pos + (-right + up) * VsToGs[0].Size;
			gl_Position = VP * vec4(tl, 1.0);
			GsToFS.Color = VsToGs[0].Color;
			GsToFS.TexId = VsToGs[0].TexId;
			GsToFS.TexCoord = vec2(0.0, 0.0);

			// emit a single vertex
			EmitVertex();

			vec3 tr = pos + (right + up) * VsToGs[0].Size;
			gl_Position = VP * vec4(tr, 1.0);
			GsToFS.Color = VsToGs[0].Color;
			GsToFS.TexId = VsToGs[0].TexId;
			GsToFS.TexCoord = vec2(1.0, 0.0);

			// emit a single vertex
			EmitVertex();

			vec3 bl = pos + (-right - up) * VsToGs[0].Size;
			gl_Position = VP * vec4(bl, 1.0);
			GsToFS.Color = VsToGs[0].Color;
			GsToFS.TexId = VsToGs[0].TexId;
			GsToFS.TexCoord = vec2(0.0, 1.0);

			// emit a single vertex
			EmitVertex();

			vec3 br = pos + (right - up) * VsToGs[0].Size;
			gl_Position = VP * vec4(br, 1.0);
			GsToFS.Color = VsToGs[0].Color;
			GsToFS.TexId = VsToGs[0].TexId;
			GsToFS.TexCoord = vec2(1.0, 1.0);

			// emit a single vertex
			EmitVertex();

		})LIT";
	
	const char* fragment_shader = R"LIT(#version 410
		uniform sampler2D xSamplers[8];
		struct GsVert {
			vec2  TexCoord;
			vec4  Color;
			float TexId;
		};
		in GsVert GsToFS;
		out vec4 FinalColor;
		void main() {
			FinalColor = GsToFS.Color * texture2D(xSamplers[int(GsToFS.TexId)], GsToFS.TexCoord);
		})LIT";

	GLenum error = glGetError();
	
	myShaderProgram = glCreateProgram();
	GLuint g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
	GLuint g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint g_GeoHandle = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
	glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
	glShaderSource(g_GeoHandle, 1, &geometry_shader, 0);
	glCompileShader(g_VertHandle);
	glCompileShader(g_FragHandle);
	glCompileShader(g_GeoHandle);
	glAttachShader(myShaderProgram, g_VertHandle);
	glAttachShader(myShaderProgram, g_FragHandle);
	glAttachShader(myShaderProgram, g_GeoHandle);
	glLinkProgram(myShaderProgram);
    glDeleteShader(g_VertHandle);
    glDeleteShader(g_FragHandle);
	glDeleteShader(g_GeoHandle);
	
	myViewUniformLoc = glGetUniformLocation(myShaderProgram, "xView");
	myWorldUniformLoc = glGetUniformLocation(myShaderProgram, "xWorld");
	myProjectionLoc = glGetUniformLocation(myShaderProgram, "xProjection");
	myTexturesUniformLoc = glGetUniformLocation(myShaderProgram, "xSamplers");

	GLint success = 0;
	glGetShaderiv(g_VertHandle, GL_COMPILE_STATUS, &success);

	// If we failed to compile
	if (success == GL_FALSE) {
		// Get the size of the error log
		GLint logSize = 0;
		glGetShaderiv(g_VertHandle, GL_INFO_LOG_LENGTH, &logSize);

		// Create a new character buffer for the log
		char* log = new char[logSize];

		// Get the log
		glGetShaderInfoLog(g_VertHandle, logSize, &logSize, log);
		delete[] log;
	}
	glGetShaderiv(g_GeoHandle, GL_COMPILE_STATUS, &success);

	// If we failed to compile
	if (success == GL_FALSE) {
		// Get the size of the error log
		GLint logSize = 0;
		glGetShaderiv(g_GeoHandle, GL_INFO_LOG_LENGTH, &logSize);

		// Create a new character buffer for the log
		char* log = new char[logSize];

		// Get the log
		glGetShaderInfoLog(g_GeoHandle, logSize, &logSize, log);
		delete[] log;
	}

	glGetProgramiv(myShaderProgram, GL_LINK_STATUS, &success);

	// If not, we need to grab the log and throw an exception
	if (success == GL_FALSE) {
		// Get the length of the log
		GLint length = 0;
		glGetProgramiv(myShaderProgram, GL_INFO_LOG_LENGTH, &length);

		// Read the log from openGL
		char* log = new char[length];
		glGetProgramInfoLog(myShaderProgram, length, &length, log);
		delete[] log;
	}

	error = glGetError();

	GLint prevVbo{ 0 }, prevVao{ 0 };
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVbo);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);

	glGenVertexArrays(1, &myVao);
	glGenBuffers(1, &myPrimitiveBuffer);

	glBindVertexArray(myVao);
	glBindBuffer(GL_ARRAY_BUFFER, myPrimitiveBuffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_BATCH_SIZE * sizeof(ParticleVertex), myVertexBufferData, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(ParticleVertex), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(ParticleVertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, false, sizeof(ParticleVertex), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(ParticleVertex), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(ParticleVertex), (void*)(9 * sizeof(float)));

	error = glGetError();

	glBindVertexArray(0);

	glBindVertexArray(prevVao);
	glBindBuffer(GL_ARRAY_BUFFER, prevVbo);

	uint8_t data[4] = {
		255U, 255U, 255U, 255U
	};
	myDefaultTexture.createTexture(1, 1, GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data);
	//delete[] data;

	error = glGetError();

	myTextureHandles[0] = myDefaultTexture.id();
}

void Renderer::PushMatrix(glm::mat4 world) {
    myMatrixStack.push(world);
    myActiveMatrix = myActiveMatrix * world; 
}

void Renderer::PopMatrix() {
    if (myMatrixStack.size() > 0) {
        myActiveMatrix = myActiveMatrix * glm::inverse(myMatrixStack.top());
        myMatrixStack.pop();	       
    }
}

void Renderer::SetActiveTexture(const uint8_t slot) {
	myActiveTexture = slot;
}

void Renderer::Submit(const glm::vec3 &pos, const glm::vec4& color, const float angle,  const float size, uint8_t texture) {
	if (texture == 255)
		texture = myActiveTexture;

	myVertexBufferData[myActiveParticleCount].Position = (pos.xyzz * myActiveMatrix).xyz;
	myVertexBufferData[myActiveParticleCount].Color    = color;
	myVertexBufferData[myActiveParticleCount].Size     = size;
	myVertexBufferData[myActiveParticleCount].Angle    = angle;
	myVertexBufferData[myActiveParticleCount].TexId    = texture;

	myActiveParticleCount++;

	if (myActiveParticleCount == PARTICLE_BATCH_SIZE)
		Flush();
}

void Renderer::Flush() {

	GLint prevVbo{ 0 }, prevVao{ 0 }, prevShader{ 0 }, prevTexSlot{ 0 };
	GLboolean tex2DEnabled{ false }, texEnabled{ false };
	GLint prevTexBindings[RENDERER_MAX_TEXTURES];
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVbo);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
	glGetIntegerv(GL_CURRENT_PROGRAM, &prevShader);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &prevTexSlot);
	glGetBooleanv(GL_TEXTURE_2D, &tex2DEnabled);
	glGetBooleanv(GL_TEXTURE, &texEnabled);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE);

	// Buffer orphaning technique, most of the time this will get the same buffer, also should be pretty fast
	glBindBuffer(GL_ARRAY_BUFFER, myPrimitiveBuffer);
	glBufferData(GL_ARRAY_BUFFER, myActiveParticleCount * sizeof(ParticleVertex), NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, myActiveParticleCount * sizeof(ParticleVertex), myVertexBufferData, GL_DYNAMIC_DRAW);
	
	glUseProgram(myShaderProgram);
	glUniformMatrix4fv(myViewUniformLoc, 1, false, &ViewMatrix[0][0]);
	glUniformMatrix4fv(myWorldUniformLoc, 1, false, &WorldTransform[0][0]);
	glUniformMatrix4fv(myProjectionLoc, 1, false, &ProjectionMatrix[0][0]);

	for (int ix = 0; ix < RENDERER_MAX_TEXTURES; ix++) {
		glActiveTexture(GL_TEXTURE0 + ix);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexBindings[ix]);
		glBindTexture(GL_TEXTURE_2D, myTextureHandles[ix]);
		glUniform1i(myTexturesUniformLoc + ix, ix);
	}

	glBindVertexArray(myVao);

	glDrawArrays(GL_POINTS, 0, myActiveParticleCount);

	glBindVertexArray(prevVao);
	glBindBuffer(GL_ARRAY_BUFFER, prevVbo);

	glUseProgram(prevShader);

	for (int ix = 0; ix < RENDERER_MAX_TEXTURES; ix++) {
		glActiveTexture(GL_TEXTURE0 + ix);
		glBindTexture(GL_TEXTURE_2D, prevTexBindings[ix]);
	}
	glActiveTexture(prevTexSlot);

	if (!tex2DEnabled) glDisable(GL_TEXTURE_2D);
	if (!texEnabled) glDisable(GL_TEXTURE);

	myActiveParticleCount = 0;
}

void Renderer::SetTexture(const uint8_t slot, const uint32_t handle) {
	myTextureHandles[slot] = handle;
}

void Renderer::Cleanup() {}
