#include <GLEW/glew.h>
#include "TTK/Texture2D.h"
#include "FreeImage/FreeImage.h"

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
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	//pointer to the image, once loaded
	FIBITMAP *dib(0);

	//pointer to the image data
	BYTE* bits(0);

	//image width and height
	unsigned int width(0), height(0), bpp(0);

	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filePath.c_str(), 0);

	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filePath.c_str());
	//if still unknown, return failure
	if (fif == FIF_UNKNOWN)
	{
		std::cout << "Unable to load texture: " + filePath << std::endl;
		return;
	}

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filePath.c_str());
	
	//if the image failed to load, return failure
	if (!dib)
	{
		std::cout << "Unable to load texture: " + filePath << std::endl;
		return;
	}

	//retrieve the image data
	bits = FreeImage_GetBits(dib);

	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	bpp = FreeImage_GetBPP(dib);

	//if this somehow one of these failed (they shouldn't), return failure
	if ((bits == 0) || (width == 0) || (height == 0))
	{
		std::cout << "Unable to load texture: " + filePath << std::endl;
		return;
	}

	GLenum internal_format, image_format;

	switch (bpp)
	{
	case 24:
		internal_format = GL_RGB8;
		image_format = GL_RGB;
		break;

	case 32:
		internal_format = GL_RGBA8;
		image_format = GL_RGBA;
		break;

	default:
		std::cout << "Unsupported texture format. Texture may look strange." + filePath << std::endl;
		internal_format = GL_RGB8;
		image_format = GL_RGB;
		break;
	}
	
	createTexture(width, height, GL_TEXTURE_2D, GL_LINEAR, GL_CLAMP_TO_EDGE, internal_format, image_format, GL_UNSIGNED_BYTE, bits);

	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);
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
