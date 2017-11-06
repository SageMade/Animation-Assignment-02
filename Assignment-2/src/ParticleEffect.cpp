#include "ParticleEffect.h"

#include <TTK/GraphicsUtils.h>

#include <iostream>
#include "FileHelpers.h"

ParticleEffect::ParticleEffect() {
}

ParticleEffect::~ParticleEffect()
{
}
void ParticleEffect::FBO_2_PPM_file()
{
	FILE    *output_image;
	int     output_width, output_height;

	output_width = myFboWidth;
	output_height = myFboHeight;

	/// READ THE PIXELS VALUES from FBO AND SAVE TO A .PPM FILE
	int             i, j, k;
	unsigned char   *pixels = (unsigned char*)malloc(output_width*output_height * 3);

	/// READ THE CONTENT FROM THE FBO
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, output_width, output_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	output_image = fopen("output.ppm", "wt");
	fprintf(output_image, "P3\n");
	fprintf(output_image, "%d %d\n", output_width, output_height);
	fprintf(output_image, "255\n");

	k = 0;
	for (i = 0; i<output_width; i++)
	{
		for (j = 0; j<output_height; j++)
		{
			fprintf(output_image, "%u %u %u ", (unsigned int)pixels[k], (unsigned int)pixels[k + 1],
				(unsigned int)pixels[k + 2]);
			k = k + 3;
		}
		fprintf(output_image, "\n");
	}
	free(pixels);
}

void ParticleEffect::Init() {

	for (int ix = 0; ix < Layers.size(); ix++)
		Layers[ix]->initialize();
	
	glGenFramebuffers(1, &myFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, myFramebuffer);

	myFboWidth = TTK::Graphics::ScreenWidth;
	myFboHeight = TTK::Graphics::ScreenHeight;

	glBindTexture(GL_TEXTURE_2D, myRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, myFboWidth, myFboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture(GL_FRAMEBUFFER, 0, myRenderTexture, 0);

	glGenRenderbuffers(1, &myDepthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, myDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, myFboWidth, myFboHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, myDepthRenderBuffer);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Failed to build FBO" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

void ParticleEffect::AddLayer(ParticleLayerSettings settings) {
	ParticleLayer *emitter = new ParticleLayer(settings);
	emitter->initialize();
	emitter->Settings.Config.Index = (uint8_t)Layers.size();
	Layers.push_back(emitter);
	
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

	TTK::Graphics::FullscreenQuad(myRenderTexture);

	glPushMatrix();
	glLoadIdentity();

	// TODO: render fullscreen quad
	glBindTexture(GL_TEXTURE_2D, myRenderTexture);
	glEnable(GL_TEXTURE_2D);

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
#endif
}

void ParticleEffect::WriteToFile(std::fstream & stream) {
	size_t size = Layers.size();
	Write(stream, &size);
	for (int ix = 0; ix < Layers.size(); ix++) {
		Layers[ix]->Settings.WriteToFile(stream);
	}
}

ParticleEffect ParticleEffect::ReadFromFile(std::fstream & stream) {
	ParticleEffect result = ParticleEffect();
	size_t count = 0;
	Read(stream, &count);

	for (int ix = 0; ix < count; ix++) {
		result.Layers.push_back(new ParticleLayer(ParticleLayerSettings::ReadFromFile(stream)));
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
		ParticleLayer *layer = new ParticleLayer(settings.Layers[ix]);
		layer->initialize();
		Layers.push_back(layer);
	}
}
