// Shaun McKinnon - 100642799 //
#pragma once

#include <GLM/glm.hpp>
#include <GLM\gtx\transform.hpp>
#include <GLM\gtc\type_ptr.hpp>
#include <string>

#include "TTK\QuadMesh.h"
#include <memory>

class GameObject
{
protected:
	float m_pScale;

	float m_pRotX, m_pRotY, m_pRotZ; // local rotation angles

	glm::vec3 m_pLocalPosition;

	glm::mat4 m_pLocalRotation;
	glm::mat4 m_pLocalToWorldMatrix;

public:
	GameObject();
	~GameObject();

	void setPosition(glm::vec3 newPosition);
	void setRotationAngleX(float newAngle);
	void setRotationAngleY(float newAngle);
	void setRotationAngleZ(float newAngle);
	void setScale(float newScale);

	glm::vec3 getPosition();
	float getRotationAngleY();

	glm::mat4 getLocalToWorldMatrix();

	virtual void update(float dt);	
	virtual void draw();

	// Other Properties
	std::string name;
	glm::vec4 colour; 
	std::shared_ptr<TTK::MeshBase> mesh;

	// Material* ...
};