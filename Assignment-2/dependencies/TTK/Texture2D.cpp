#include <GLEW/glew.h>
#include "TTK/Texture2D.h"

#include <stb_image.h>

#include <iostream>

Texture2D::Texture2D()
	: m_pTexWidth(0),
	m_pTexHeight(0),
	m_pTexID(0)
{
}

Texture2D::Texture2D(int _id, int _width, int _height, GLenum target)
{
	m_pTexID = _id;
	m_pTexWidth = _width;
	m_pTexHeight = _height;
	m_pTarget = target;
}

Texture2D::~Texture2D()
{
	deleteTexture();
}

int Texture2D::width()
{
	return m_pTexWidth;
}

int Texture2D::height()
{
	return m_pTexHeight;
}

GLenum Texture2D::internalFormat()
{
	return m_pInternalFormat;
}

void Texture2D::bind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
	glActiveTexture(textureUnit);
	glBindTexture(m_pTarget, m_pTexID);
}

void Texture2D::unbind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
	glActiveTexture(textureUnit);
	glBindTexture(m_pTarget, 0);
}

void Texture2D::loadTextureFromFile(std::string filePath)
{
	// Declare outputs for the stbi function
	int width{ 0 }, height{ 0 }, numChannels{ 0 };
	// Simple enough, just use the stbi function, the important notes are that we are forcing 4 bytes of pixel data
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &numChannels, 4);
	
	// Check to see if we got any data
	if (data) {
		createTexture(width, height, GL_TEXTURE_2D, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data);
		delete data;
	}
}

void Texture2D::createTexture(int w, int h, GLenum target, GLenum filtering, GLenum edgeBehaviour, GLenum internalFormat, GLenum textureFormat, GLenum dataType, void* newDataPtr)
{
	m_pTexWidth = w;
	m_pTexHeight = h;
	m_pFiltering = filtering;
	m_pEdgeBehaviour = edgeBehaviour;
	m_pInternalFormat = internalFormat;
	m_pTextureFormat = textureFormat;
	m_pDataType = dataType;
	m_pDataPtr = newDataPtr;
	m_pTarget = target;

	GLenum error = 0;

	// Not necessary to enable GL_TEXTURE_* in modern context.
//	glEnable(m_pTarget);
//	error = glGetError();

	if (m_pTexID)
		deleteTexture();

	glGenTextures(1, &m_pTexID);
	glBindTexture(target, m_pTexID);
	error = glGetError();

	glTexParameteri(m_pTarget, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(m_pTarget, GL_TEXTURE_MAG_FILTER, filtering);
	glTexParameteri(m_pTarget, GL_TEXTURE_WRAP_S, edgeBehaviour);
	glTexParameteri(m_pTarget, GL_TEXTURE_WRAP_T, edgeBehaviour);
	error = glGetError();

	glTexImage2D(m_pTarget, 0, internalFormat, w, h, 0, textureFormat, dataType, newDataPtr);
	error = glGetError();

	if (error != 0)
		std::cout << "There was an error somewhere when creating texture. " << std::endl;

	glBindTexture(m_pTarget, 0);

}

void Texture2D::updateTexture(void* newDataPtr /*= nullptr*/)
{
	if (newDataPtr == nullptr && m_pDataPtr == nullptr)
		return;

	glBindTexture(m_pTarget, m_pTexID);

	if (newDataPtr == nullptr)
		glTexSubImage2D(m_pTarget, 0, 0, 0, m_pTexWidth, m_pTexHeight, m_pTextureFormat, m_pDataType, m_pDataPtr);
	else
		glTexSubImage2D(m_pTarget, 0, 0, 0, m_pTexWidth, m_pTexHeight, m_pTextureFormat, m_pDataType, newDataPtr);

	glBindTexture(m_pTarget, 0);
}

void Texture2D::deleteTexture()
{
	glDeleteTextures(1, &m_pTexID);
}

unsigned int Texture2D::id() const
{
	return m_pTexID;
}

void* Texture2D::data()
{
	return m_pDataPtr;
}
