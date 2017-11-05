#include "ParticleEffect.h"

#include <TTK/GraphicsUtils.h>

ParticleEffect::ParticleEffect() {
}

ParticleEffect::~ParticleEffect()
{
}

void ParticleEffect::Init() {

	for (int ix = 0; ix < Layers.size(); ix++)
		Layers[ix]->initialize();
	
	/*
	glGenFramebuffers(1, &myFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);

	glBindTexture(GL_TEXTURE_2D, myRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture(GL_FRAMEBUFFER, 0, myRenderTexture, 0);

	glGenRenderbuffers(1, &myDepthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, myDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TTK::Graphics::ScreenWidth, TTK::Graphics::ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, myDepthRenderBuffer);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	*/
}

void ParticleEffect::AddLayer(ParticleLayerSettings settings) {
	ParticleEmitter *emitter = new ParticleEmitter(settings);
	Layers.push_back(emitter);
	
}

void ParticleEffect::Update(float dt) {
	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->update(dt, Origin);
	}
}

void ParticleEffect::Draw() {

	/*
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);
	*/

	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->draw(Origin);
	}

	/*
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glPushMatrix();
	glLoadIdentity();

	// TODO: render fullscreen quad
	glBindTexture(GL_TEXTURE_2D, myRenderTexture);
	glEnable(GL_TEXTURE);

	glBegin(GL_TRIANGLES);

	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);

	glVertex2f(-1.0f,  1.0f);
	glTexCoord2f(0.0f, 1.0f);

	glVertex2f( 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);

	glVertex2f(-1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);

	glVertex2f( 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);

	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);

	glEnd();

	glPopMatrix();
	*/
}

void ParticleEffect::WriteToFile(std::fstream & stream) {
	stream << Layers.size();
	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->Settings.WriteToFile(stream);
	}
}

ParticleEffect ParticleEffect::ReadFromFile(std::fstream & stream) {
	ParticleEffect result = ParticleEffect();
	size_t count = 0;
	stream >> count;

	for (int ix = 0; ix < count; ix++) {
		result.Layers.push_back(new ParticleEmitter(ParticleLayerSettings::ReadFromFile(stream)));
	}
	return result;
}
