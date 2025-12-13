#include "SceneFrontier.h"
#include "GL\glew.h"
#include "Application.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "StatesKingdom.h"
#include "StatesHorde.h"
#include <sstream>

SceneFrontier::SceneFrontier() : m_timeLimit(0), m_worldWidth(0), m_worldHeight(0), m_gridSize(0), m_humanResources(0), m_hordeResources(0)
{
}
SceneFrontier::~SceneFrontier() {}

void SceneFrontier::Init()
{
	SceneBase::Init();
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	m_gridSize = m_worldHeight / 20.0f; // 20x20 Grid
	m_timeLimit = 300.0f; // 5 Minutes

	float halfGrid = m_gridSize * 0.5f;

	m_humanResources = 0;
	m_hordeResources = 0;

	// Register PostOffice
	PostOffice::GetInstance()->Register("Scene", this);

	// --- SPAWN ENTITIES ---

	// Town Hall (West)
	GameObject* th = FetchGO(GameObject::GO_TOWNHALL);
	th->pos.Set(2 * m_gridSize + halfGrid, 10 * m_gridSize + halfGrid, 0); 
	th->teamID = 1;
	th->active = true;
	th->maxHp = 500; th->hp = 500;

	// Horde Totem (East)
	int totemGridX = (int)(m_worldWidth / m_gridSize) - 3;
	GameObject* totem = FetchGO(GameObject::GO_TOTEM);
	totem->pos.Set(totemGridX * m_gridSize + halfGrid, 10 * m_gridSize + halfGrid, 0);
	totem->teamID = 2;
	totem->active = true;

	// Resources
	for (int i = 0; i < 15; ++i) {
		GameObject* tree = FetchGO(GameObject::GO_RESOURCE_TREE);
		int gridX = Math::RandIntMinMax(0, 19);
		int gridY = Math::RandIntMinMax(0, 19);
		tree->pos.Set(gridX * m_gridSize + halfGrid, gridY * m_gridSize + halfGrid, 0);
		tree->active = true;
	}

	// Units: 3 Peasants, 1 Knight, 1 Cleric
	for (int i = 0; i < 3; ++i) {
		GameObject* p = FetchGO(GameObject::GO_PEASANT);
		p->pos = th->pos + Vector3(m_gridSize, float(i) * m_gridSize, 0); // Offset by 1 tile
		p->active = true;
	}
	GameObject* k = FetchGO(GameObject::GO_KNIGHT);
	k->pos = th->pos + Vector3(2 * m_gridSize, 0, 0);
	k->active = true;
	GameObject* c = FetchGO(GameObject::GO_CLERIC); c->pos = th->pos + Vector3(-5 * m_gridSize, 0, 0); c->active = true;

	// Units: 3 Goblins, 1 Orc, 1 Shaman
	for (int i = 0; i < 3; ++i) {
		GameObject* g = FetchGO(GameObject::GO_GOBLIN);
		g->pos = totem->pos + Vector3(-5, float(i) * 5, 0);
		g->active = true;
	}
	GameObject* o = FetchGO(GameObject::GO_ORC); o->pos = totem->pos + Vector3(-10, 0, 0); o->active = true;
	GameObject* s = FetchGO(GameObject::GO_SHAMAN); s->pos = totem->pos + Vector3(5, 0, 0); s->active = true;
}

GameObject* SceneFrontier::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	GameObject* go = new GameObject(type);
	go->scale.Set(m_gridSize * 0.8f, m_gridSize * 0.8f, 1);

	// FSM Assignment
	if (type == GameObject::GO_PEASANT) {
		go->sm = new StateMachine();
		go->sm->AddState(new StatePeasantIdle("Idle", go));
		go->sm->AddState(new StatePeasantGather("Gather", go));
		go->sm->AddState(new StatePeasantReturn("Return", go));
		go->sm->SetNextState("Idle");
		go->teamID = 1;
	}
	else if (type == GameObject::GO_KNIGHT) {
		go->sm = new StateMachine();
		go->sm->AddState(new StateKnightPatrol("Patrol", go));
		go->sm->AddState(new StateKnightChase("Chase", go));
		go->sm->AddState(new StateKnightAttack("Attack", go));
		go->sm->SetNextState("Patrol");
		go->maxHp = 100; go->hp = 100;
		go->teamID = 1;
	}
	else if (type == GameObject::GO_CLERIC) {
		go->sm = new StateMachine();
		go->sm->AddState(new StateClericHeal("Heal", go));
		go->sm->SetNextState("Heal");
		go->teamID = 1;
	}
	else if (type == GameObject::GO_GOBLIN) {
		go->sm = new StateMachine();
		go->sm->AddState(new StateGoblinProwl("Prowl", go));
		go->sm->AddState(new StateGoblinSteal("Steal", go));
		go->sm->SetNextState("Prowl");
		go->teamID = 2;
	}
	else if (type == GameObject::GO_ORC) {
		go->sm = new StateMachine();
		go->sm->AddState(new StateOrcSmash("Smash", go));
		go->sm->SetNextState("Smash");
		go->teamID = 2;
	}
	else if (type == GameObject::GO_SHAMAN) {
		go->sm = new StateMachine();
		go->sm->AddState(new StateShamanRitual("Ritual", go));
		go->sm->SetNextState("Ritual");
		go->teamID = 2;
	}

	if (m_timeLimit < 299.0f) {
		m_goList_Add.push_back(go);
	}
	else {
		m_goList.push_back(go);
	}
	return go;
}

void SceneFrontier::Update(double dt)
{
	SceneBase::Update(dt);
	m_timeLimit -= (float)dt;
	if (m_timeLimit <= 0) return;

	for (auto go : m_goList) {
		if (!go->active) continue;
		if (go->sm) go->sm->Update(dt);

		// --- UPDATED MOVEMENT LOGIC (WEEK 05 STYLE) ---
		// Calculate distance to target
		Vector3 dir = go->target - go->pos;
		float dist = dir.Length();

		// If we can reach the target in this frame (or are very close)
		if (dist < go->moveSpeed * (float)dt) {
			// SNAP to exact position
			go->pos = go->target;
		}
		else {
			// Move towards target
			// Use try-catch or check length > 0 to avoid divide by zero
			if (dist > 0) {
				dir.Normalize();
				go->pos += dir * go->moveSpeed * (float)dt;
			}
		}
	}
	if (m_goList_Add.size() > 0)
	{
		for (auto go : m_goList_Add) {
			m_goList.push_back(go);
		}
		m_goList_Add.clear();
	}
}

bool SceneFrontier::Handle(Message* message)
{
	// 1. Find Nearest Handler (Optimized: O(N) linear scan)
	MessageFindNearest* msgFN = dynamic_cast<MessageFindNearest*>(message);
	if (msgFN) {
		float minDist = 999999.f;
		GameObject* best = nullptr;
		for (auto go : m_goList) {
			if (!go->active) continue;
			if (go->type == msgFN->targetType) {
				float d = (go->pos - msgFN->requestor->pos).LengthSquared(); // Use LengthSq for speed
				if (d < minDist) {
					minDist = d;
					best = go;
				}
			}
		}
		msgFN->requestor->nearest = best;
		return true;
	}

	// 2. Attack Handler
	MessageAttack* msgAtk = dynamic_cast<MessageAttack*>(message);
	if (msgAtk) {
		if (msgAtk->target && msgAtk->target->active) {
			msgAtk->target->hp -= msgAtk->damage;
			if (msgAtk->target->hp <= 0) msgAtk->target->active = false;
		}
		return true;
	}

	// 3. Raid/Resource Handler
	MessageRaidSuccess* msgRaid = dynamic_cast<MessageRaidSuccess*>(message);
	if (msgRaid) {
		// Just a simple team check or based on sender
		// Here assuming simple addition to Human for demo
		m_humanResources += msgRaid->amount;
		return true;
	}

	// 4. Spawn Handler
	MessageSpawnUnit* msgSpawn = dynamic_cast<MessageSpawnUnit*>(message);
	if (msgSpawn) {
		if (msgSpawn->teamID == 2) { // Horde Spawn
			GameObject* g = FetchGO(GameObject::GO_GOBLIN);
			// Spawn near Totem (approx position)
			g->pos.Set(m_worldWidth - 20, m_worldHeight / 2, 0);
			g->active = true;
		}
		return true;
	}

	return false;
}

void SceneFrontier::Render()
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

	// Render loop
	for (auto go : m_goList) {
		if (go->active) RenderGO(go);
	}

	// UI Text
	std::ostringstream ss;
	ss << "Human Res: " << m_humanResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 2, 55);

	ss.str(""); ss << "Horde Loot: " << m_hordeResources;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 2, 50);

	ss.str(""); ss << "Time: " << (int)m_timeLimit;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, 30, 55);
}

void SceneFrontier::RenderGO(GameObject* go)
{
	modelStack.PushMatrix();
	modelStack.Translate(go->pos.x, go->pos.y, 0);
	modelStack.Scale(go->scale.x, go->scale.y, 1);

	// UPDATED MAPPING
	if (go->type == GameObject::GO_PEASANT)
		RenderMesh(meshList[GEO_GREEN_BALL], false); // Now actually Green
	else if (go->type == GameObject::GO_ORC)
		RenderMesh(meshList[GEO_RED_CUBE], false);   // Now actually Red
	else if (go->type == GameObject::GO_TOWNHALL)
		RenderMesh(meshList[GEO_GREY_CUBE], false);  // Grey Base
	else if (go->type == GameObject::GO_RESOURCE_TREE)
		RenderMesh(meshList[GEO_GREEN_BALL], false); // Green Tree
	else if (go->type == GameObject::GO_TOTEM)
		RenderMesh(meshList[GEO_CUBE], false);       // White Totem (default)
	else
		RenderMesh(meshList[GEO_BALL], false);       // Default Red Ball for others

	modelStack.PopMatrix();
}

void SceneFrontier::Exit()
{
	for (auto go : m_goList) delete go;
	m_goList.clear();
	SceneBase::Exit();
}