#ifndef SCENE_KNIGHT_H
#define SCENE_KNIGHT_H

#include <vector>
#include "SceneBase.h"

class SceneKnight : public SceneBase
{
public:
	SceneKnight();
	~SceneKnight();

	void PrintTour();
	void DFS(int index);
	


	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

protected:
	// Exercise Week 06


	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	//an array or vector container to store the knight's moves
	std::vector<int> m_grid;
	std::vector<int> m_grid_results;
	int m_numTours;
	int m_move;
	int m_move_results;
	int m_call;
	int m_startGrid;
	bool bStop;
};

#endif