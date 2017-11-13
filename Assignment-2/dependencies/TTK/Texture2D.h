//////////////////////////////////////////////////////////////////////////
//
// This is a simple OpenGL Texture2D class
//
// Michael Gharbharan 2015 - 2017
//
//////////////////////////////////////////////////////////////////////////

#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include <string>
#define GLEW_STATIC
#include "glew/glew.h"
#include <memory>
#include <fstream>

class  Texture2D
{
public:
	typedef std::shared_ptr<Texture2D> Ptr;
	
	char Name[16];
	char FileName[256];

	 Texture2D();
	 Texture2D::Texture2D(int _id, int _width, int _height, GLenum target);
	 ~Texture2D();

	// Returns width / height of texture in pixels
	 int width();
	 int height();
	
	// Returns texture pixel internal format
	 GLenum internalFormat();

	// Binds / unbinds texture
	 void bind(GLenum textureUnit = GL_TEXTURE0);
	 void unbind(GLenum textureUnit = GL_TEXTURE0);

	 void loadTextureFromFile(std::string filePath);
	 
	// Description:
	// Creates the texture, allocates memory and uploads data to GPU
	// If you do not want to upload data to the GPU pass in a nullptr for the dataPtr.
	// For a description on filtering and edgeBehaviour see https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
	// For a description on internalFormat, textureFormat and dataType see https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
	 void createTexture(int w, int h, GLenum target, GLenum filtering, GLenum edgeBehaviour, GLenum internalFormat, GLenum textureFormat, GLenum dataType, void* newDataPtr);

	// Description:
	// Used to re-upload texture data to the GPU
	// if newDataPtr is null then the function will use the
	// stored dataDtr.
	// Returns: true if successful
	 void updateTexture(void* newDataPtr = nullptr);

	// Description:
	// Deletes texture allocated on the GPU
	 void deleteTexture();
	 
	 // Writes all relevant data into a file, does not compress the texture data
	 void writeToFile(std::fstream& stream);

	 // reads all relevent data from a flat file stream 
	 void readFromFile(std::fstream& stream);

	// Returns OpenGL texture id
	 unsigned int id() const;

	// Returns pointer to texture data
	 void* data();

private:
	unsigned int m_pTexWidth;
	unsigned int m_pTexHeight;
	unsigned int m_pTexID;

	GLenum m_pFiltering;
	GLenum m_pEdgeBehaviour;
	GLenum m_pInternalFormat;
	GLenum m_pTextureFormat;
	GLenum m_pDataType;

	GLenum m_pTextureUnit;

	GLenum m_pTarget; // usually GL_TEXTURE_2D

	void* m_pDataPtr;
};

#endif