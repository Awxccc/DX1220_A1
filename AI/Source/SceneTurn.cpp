#include "SceneTurn.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneTurn::SceneTurn()
	: m_turn{}, m_goList{}, m_speed{}, m_worldWidth{},
	  m_worldHeight{}, m_objectCount{}, m_noGrid{},
	  m_gridSize{}, m_gridOffset{}, m_maze{},
	  m_start{}, m_end{}, m_myGrid{},
	  m_visited{}, m_queue{}, m_previous{},
	  m_shortestPath{}, m_mazeKey{}, m_wallLoad{}
{
}

SceneTurn::~SceneTurn()
{
}

void SceneTurn::Init()
{
	SceneBase::Init();
	bLightEnabled = true; //week 8: have to do this for material to work

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_noGrid = 12;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_start.Set(0, 0);
	m_mazeKey = 0;
	m_wallLoad = 0.3f;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
	//DFS(m_start);
}

GameObject* SceneTurn::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			go->type = type;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_NONE);
		m_goList.push_back(go);
	}
	return FetchGO(type);
}

void SceneTurn::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if(curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_UP) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_DOWN);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_DOWN) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_UP);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_LEFT) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_RIGHT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_RIGHT) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_LEFT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
}

bool SceneTurn::BFS(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_queue.empty())
		m_queue.pop();
	m_shortestPath.clear();
	m_queue.push(start);
	m_maze.SetNumMove(0);
	while (!m_queue.empty())
	{
		MazePt curr = m_queue.front();
		m_maze.SetCurr(curr);
		m_queue.pop();
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = m_previous[curr.y * m_noGrid + curr.x];
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//RIGHT
		if (curr.x < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
	}
	return false;
}

void SceneTurn::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	if (Application::IsKeyPressed('N'))
	{
	}
	if (Application::IsKeyPressed('M'))
	{
	}
	if (Application::IsKeyPressed(VK_RETURN))
	{
	}
	if (Application::IsKeyPressed('R'))
	{
		//Exercise: Implement Reset button
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize) //ensure we're clicking within the board
		{
			m_end.Set(static_cast<int>(posX / m_gridSize), static_cast<int>(posY / m_gridSize));
			for (GameObject* go : m_goList)
				if (go->active && go->type == GameObject::GO_NPC)
					BFSLimit(go, m_end, 50); //setting limit to 50 instead of 20. 20 feels pretty crap.
		}
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;

		// Exercise Week 08
		//7.	SceneTurn::Update, spawn a GO_NPC with spacebar at any empty tile



	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	// Exercise Week 08
	static constexpr float TURN_TIME = 0.5f;
	static float timer = 0.f;
	// 6.	SceneTurn::Update - add code to support a turn-based system. Read this pseudo codes and try to implement on your own. The solution will be provided during lesson time.



}

void SceneTurn::RenderGO(GameObject *go)
{
	// Exercise Week 8
	// d.	Render each go based on its curr position - edit RenderGO(GameObject::GO_NPC)
	switch (go->type)
	{
		case GameObject::GO_NPC: //Render GO_NPC
		{
			modelStack.PushMatrix();
			modelStack.Translate(	m_gridOffset + m_gridSize * go->curr.x, 
									m_gridOffset + m_gridSize * go->curr.y, 
									0.5f);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			RenderMesh(meshList[GEO_AGENT], false);
			modelStack.PopMatrix();
		}
		break;
	}
}

void SceneTurn::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
		);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_WHITEQUAD], false);
	modelStack.PopMatrix();

	//Render tiles 
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * col, m_gridOffset + m_gridSize * row, 0);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			// Exercise Week 08
			// b.Render the tiles using m_maze.m_grid instead of m_myGrid

			modelStack.PopMatrix();
		}
	}

	int i = 0;
	for (bool b : m_visited)
	{
		if (b)
		{
			int cellX = i % m_noGrid;
			int cellY = i / m_noGrid;
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * cellX, m_gridOffset + m_gridSize * cellY, 0.05f);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			meshList[GEO_FLOOR]->material.kAmbient.Set(0.25f, 0.25f, 1);
			RenderMesh(meshList[GEO_FLOOR], true);
			modelStack.PopMatrix();
		}
		i++;
	}

	// Exercise Week 8
	//c.	Render each go based on its curr position - edit RenderGO(GameObject::GO_NPC)

	

	//On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Turn:" << m_turn;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn Maze " << m_mazeKey;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);

	// Exercise Week 08
	// 10.	Print out the GameObject's ID and its target position



}

void SceneTurn::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}

//helper function to check if given index is within boundary
int SceneTurn::IsWithinBoundary(int x) const
{
	return x >= 0 && x < m_noGrid;
}

//helper function to convert 2d indices to 1d index
int SceneTurn::Get1DIndex(int x, int y) const
{
	return y * m_noGrid + x;
}

// Exercise Week 08
// 8.	Implement DFSOnce() - this method will not be recursive. We will use a go->stack to help us with the depth first search



// Exercise Week 08
// 9.	Implement BFSLimit() - BFS but with limit (e.g. 20 tiles to visit). If end is found, return path to end, else return path to tile that is closest with end



