/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/

#include "ParticleEffect.h"

#include <TTK/GraphicsUtils.h>

#include <iostream>
#include "FileHelpers.h"
#include <stb_image_write.h>

#include "Renderer.h"

#define USE_FBO 1

ParticleEffect::ParticleEffect() {
	memset(Name, 0, EFFECT_NAME_MAX_LENGTH);
	memcpy(Name, "Default", 8);

	SourceBlend = GL_ONE;
	DestBlend   = GL_DST_ALPHA;
}

ParticleEffect::~ParticleEffect()
{
}
void ParticleEffect::BakeFboToBmp(const char * filename)
{
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);

	unsigned char *pixels = (unsigned char*)malloc(myFboWidth * myFboHeight * 4);

	/// READ THE CONTENT FROM THE FBO
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, myFboWidth, myFboHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	stbi_write_png(filename, myFboWidth, myFboHeight, 0, pixels, 4);

	free(pixels);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ParticleEffect::ResizeFbo(uint32_t width, uint32_t height) {

#ifdef USE_FBO

	myFboWidth = width;
	myFboHeight = height;

	if (myFramebuffer)
		glDeleteFramebuffers(1, &myFramebuffer);

	if (myRenderTexture)
		glDeleteTextures(1, &myRenderTexture);

	if (myDepthRenderBuffer)
		glDeleteRenderbuffers(1, &myDepthRenderBuffer);

	glGenFramebuffers(1, &myFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);

	glGenTextures(1, &myRenderTexture);

	glBindTexture(GL_TEXTURE_2D, myRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, myFboWidth, myFboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &myDepthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, myDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, myFboWidth, myFboHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, myDepthRenderBuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, myRenderTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Failed to build FBO" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#endif
}

void ParticleEffect::Init() {

	for (int ix = 0; ix < Layers.size(); ix++)
		Layers[ix]->initialize();

	ResizeFbo(TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);
	
}

void ParticleEffect::AddLayer(ParticleLayerSettings settings) {
	ParticleLayer *emitter = new ParticleLayer(this, settings);
	emitter->initialize();
	emitter->Settings.Config.Index = (uint8_t)Layers.size();
	Layers.push_back(emitter);
	
}

void ParticleEffect::AddBehaviour(SteeringBehaviour behaviour) {
	Behaviours.push_back(behaviour);
}

void ParticleEffect::Restart() {
	for (int ix = 0; ix < Layers.size(); ix++)
		Layers[ix]->restart();
}

void ParticleEffect::Update(float dt) {
	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->update(dt, Origin);
	}
}

void ParticleEffect::Draw() {

#ifdef USE_FBO	
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->draw(Origin);
	}

#ifdef USE_FBO	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glDisable(GL_BLEND);
	TTK::Graphics::EnableAlphaBlend();
	Renderer::FullscreenQuad(myRenderTexture);

#endif
}

glm::vec3 ParticleEffect::ApplyBehaviours(Particle * particle, float & totalWeight) {
	glm::vec3 result = glm::vec3(0.0f);
	for (int ix = 0; ix < Behaviours.size(); ix++)
		result += Behaviours[ix].Apply(particle, totalWeight);
	return result;
}

void ParticleEffect::WriteToFile(std::fstream & stream) {
	Write(stream, Name, EFFECT_NAME_MAX_LENGTH);
	uint64_t loc = stream.tellg();
	uint32_t size = Layers.size();
	Write(stream, size);

	loc = stream.tellg();

	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->Settings.WriteToFile(stream);
	} 

	Write(stream, (uint32_t)Behaviours.size());

	for (int ix = 0; ix < Behaviours.size(); ix++) {
		Behaviours[ix].WriteToFile(stream);
	}
} 

ParticleEffect* ParticleEffect::ReadFromFile(std::fstream & stream) {
	ParticleEffect *result = new ParticleEffect();
	Read(stream, result->Name, EFFECT_NAME_MAX_LENGTH);
	uint64_t loc = stream.tellg();
	uint32_t count = 0;
	Read(stream, &count);

	loc = stream.tellg();

	for (int ix = 0; ix < count; ix++) {
		result->Layers.push_back(new ParticleLayer(result, ParticleLayerSettings::ReadFromFile(stream)));
	}

	Read(stream, count);

	for (int ix = 0; ix < count; ix++) {
		result->Behaviours.push_back(SteeringBehaviour::ReadFromFile(stream));
	}

	return result;
}

void ParticleEffect::ReplaceSettings(const ParticleEffectSettings& settings) {
	while(Layers.size() > 0) {
		Layers.back()->freeMemory();
		delete Layers.back();
		Layers.pop_back();
	}

	for (int ix = 0; ix < settings.Layers.size(); ix++) {
		ParticleLayer *layer = new ParticleLayer(this, settings.Layers[ix]);
		layer->initialize();
		Layers.push_back(layer);
	}
}
