#ifndef SCENE_MOVEMENT_WEEK04_H
#define SCENE_MOVEMENT_WEEK04_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
// Exercise Week 04

class SceneMovement_Week04 : public SceneBase, public ObjectBase /* Exercise Week 04 */
{
public:
	SceneMovement_Week04();
	~SceneMovement_Week04();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	// Exercise Week 04
	
	virtual bool Handle(Message* message);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
protected:

	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;	
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL];
	float zOffset;
};

#endif