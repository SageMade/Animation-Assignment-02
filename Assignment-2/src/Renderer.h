#pragma once

#include <cstdint>
#include <stack>
#include "GLM.h"

#include <TTK/Texture2D.h>

#define PARTICLE_BATCH_SIZE 1000
#define RENDERER_MAX_TEXTURES 8

struct ParticleVertex {
	glm::vec3 Position;
	glm::vec4 Color;
	float     Size;
	float     Angle;
	float     TexId;
};

class Renderer {
	public:
		static glm::mat4 WorldTransform;
		static glm::mat4 ViewMatrix;
		static glm::mat4 ProjectionMatrix;

		static void Init();

		static void PushMatrix(glm::mat4 world);
		static void PopMatrix();

		static void SetActiveTexture(const uint8_t slot);

		static void Submit(const glm::vec3 &pos, const glm::vec4& color, const float angle = 0.0f, const float size = 1.0f, uint8_t texture = 255); 

		static void FullscreenQuad(uint32_t texHandle);

		static void Flush();
		
		static void SetTexture(const uint8_t slot, const uint32_t handle);

		static void Cleanup();

	private:
		static uint32_t myTextureHandles[RENDERER_MAX_TEXTURES];
		static int myViewUniformLoc, myWorldUniformLoc, myTexturesUniformLoc, myProjectionLoc;

		static uint32_t myPrimitiveBuffer;
		static uint32_t myVao;
		static uint16_t myActiveParticleCount;
		static uint16_t myPrimitiveBufferSize;

		static uint32_t myScreenSpaceBuffer;
		static uint32_t myFullscreenQuad;
		static uint32_t myScreenSpaceShader;
		
		static ParticleVertex *myVertexBufferData;

		static uint32_t myShaderProgram;

		static uint8_t  myActiveTexture;

		static Texture2D myDefaultTexture;
		
		static glm::mat4 myActiveMatrix;
		static std::stack<glm::mat4> myMatrixStack;
};