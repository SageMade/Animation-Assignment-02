// Shaun McKinnon - 100642799 //

#include "GameObject.h"
#include <iostream>
#include "TTK\GraphicsUtils.h"

GameObject::GameObject()
	: m_pScale(1.0f), colour(glm::vec4(1.0)), mesh(nullptr)
{
}

GameObject::~GameObject() {}

void GameObject::setPosition(glm::vec3 newPosition)
{
	m_pLocalPosition = newPosition;
}

void GameObject::setRotationAngleX(float newAngle)
{
	m_pRotX = newAngle;
}

void GameObject::setRotationAngleY(float newAngle)
{
	m_pRotY = newAngle;
}

void GameObject::setRotationAngleZ(float newAngle)
{
	m_pRotZ = newAngle;
}

void GameObject::setScale(float newScale)
{
	m_pScale = newScale;
}

glm::vec3 GameObject::getPosition()
{
	return m_pLocalPosition;
}

float GameObject::getRotationAngleY()
{
	return m_pRotY;
}

glm::mat4 GameObject::getLocalToWorldMatrix()
{
	return m_pLocalToWorldMatrix;
}

void GameObject::update(float dt)
{
	// Create 4x4 transformation matrix

	// Create rotation matrix
	glm::mat4 rx = glm::rotate(glm::radians(m_pRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 ry = glm::rotate(glm::radians(m_pRotY), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rz = glm::rotate(glm::radians(m_pRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

	// Note: pay attention to rotation order, ZYX is not the same as XYZ
	m_pLocalRotation = rz * ry * rx;

	// Create translation matrix
	glm::mat4 tran = glm::translate(m_pLocalPosition);

	// Create scale matrix
	glm::mat4 scal = glm::scale(glm::vec3(m_pScale, m_pScale, m_pScale));

	// Combine all above transforms into a single matrix
	// This is the local transformation matrix, ie. where is this game object relative to it's parent
	// If a game object has no parent (it is a root node) then its local transform is also it's global transform
	// If a game object has a parent, then we must apply the parent's transform
	m_pLocalToWorldMatrix = tran * m_pLocalRotation * scal;
}

void GameObject::draw()
{
	// Draw a coordinate frame for the object
	glm::vec3 wPos = m_pLocalPosition;
	glm::mat4 wRot = m_pLocalRotation;

	glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 blue(0.0f, 0.0f, 1.0f, 1.0f);

	TTK::Graphics::DrawVector(wPos, wRot[0], 3.0f, red);
	TTK::Graphics::DrawVector(wPos, wRot[1], 3.0f, green);
	TTK::Graphics::DrawVector(wPos, wRot[2], 3.0f, blue);

	// Draw node
	if (mesh == nullptr)
		TTK::Graphics::DrawSphere(m_pLocalToWorldMatrix, 0.5f, colour);
	else
	{
		mesh->setAllColours(colour);
		mesh->draw(m_pLocalToWorldMatrix);
	}
}