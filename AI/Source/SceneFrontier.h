#pragma once

#include "SceneBase.h"
#include "GameObject.h"
#include "ObjectBase.h"
#include <vector>

class SceneFrontier : public SceneBase, public ObjectBase
{
public:
	SceneFrontier();
	~SceneFrontier();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	// Message Handler
	virtual bool Handle(Message* message);

private:
	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	void RenderGO(GameObject* go);

	// Simulation Variables
	float m_timeLimit;
	float m_worldWidth;
	float m_worldHeight;
	float m_gridSize;

	int m_humanResources;
	int m_hordeResources;

	std::vector<GameObject*> m_goList;
	std::vector<GameObject*> m_goList_Add;
};