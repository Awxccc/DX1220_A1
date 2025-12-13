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

	m_humanResources = 0;
	m_hordeResources = 0;

	// Register PostOffice
	PostOffice::GetInstance()->Register("Scene", this);

	// --- SPAWN ENTITIES ---

	// Town Hall (West)
	GameObject* th = FetchGO(GameObject::GO_TOWNHALL);
	th->pos.Set(m_gridSize * 2, m_worldHeight / 2, 0);
	th->teamID = 1;
	th->active = true;
	th->maxHp = 500;
	th->hp = 500;

	// Horde Totem (East)
	GameObject* totem = FetchGO(GameObject::GO_TOTEM);
	totem->pos.Set(m_worldWidth - m_gridSize * 2, m_worldHeight / 2, 0);
	totem->teamID = 2;
	totem->active = true;

	// Resources
	for (int i = 0; i < 15; ++i) {
		GameObject* tree = FetchGO(GameObject::GO_RESOURCE_TREE);
		tree->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), Math::RandFloatMinMax(0, m_worldHeight), 0);
		tree->active = true;
	}

	// Units: 3 Peasants, 1 Knight, 1 Cleric
	for (int i = 0; i < 3; ++i) {
		GameObject* p = FetchGO(GameObject::GO_PEASANT);
		p->pos = th->pos + Vector3(5, float(i) * 5, 0);
		p->active = true;
	}
	GameObject* k = FetchGO(GameObject::GO_KNIGHT); k->pos = th->pos + Vector3(10, 0, 0); k->active = true;
	GameObject* c = FetchGO(GameObject::GO_CLERIC); c->pos = th->pos + Vector3(-5, 0, 0); c->active = true;

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
	go->sm = new StateMachine();
	go->scale.Set(m_gridSize * 0.8f, m_gridSize * 0.8f, 1);

	// FSM Assignment
	if (type == GameObject::GO_PEASANT) {
		go->sm->AddState(new StatePeasantIdle("Idle", go));
		go->sm->AddState(new StatePeasantGather("Gather", go));
		go->sm->AddState(new StatePeasantReturn("Return", go));
		go->sm->SetNextState("Idle");
		go->teamID = 1;
	}
	else if (type == GameObject::GO_KNIGHT) {
		go->sm->AddState(new StateKnightPatrol("Patrol", go));
		go->sm->AddState(new StateKnightChase("Chase", go));
		go->sm->AddState(new StateKnightAttack("Attack", go));
		go->sm->SetNextState("Patrol");
		go->maxHp = 100; go->hp = 100;
		go->teamID = 1;
	}
	else if (type == GameObject::GO_CLERIC) {
		go->sm->AddState(new StateClericHeal("Heal", go));
		go->sm->SetNextState("Heal");
		go->teamID = 1;
	}
	else if (type == GameObject::GO_GOBLIN) {
		go->sm->AddState(new StateGoblinProwl("Prowl", go));
		go->sm->AddState(new StateGoblinSteal("Steal", go));
		go->sm->SetNextState("Prowl");
		go->teamID = 2;
	}
	else if (type == GameObject::GO_ORC) {
		go->sm->AddState(new StateOrcSmash("Smash", go));
		go->sm->SetNextState("Smash");
		go->teamID = 2;
	}
	else if (type == GameObject::GO_SHAMAN) {
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
	if (m_timeLimit <= 0) return; // Simulation End

	// Update State Machines and Movement
	for (auto go : m_goList) {
		if (!go->active) continue;

		if (go->sm) go->sm->Update(dt);

		// Physics Movement clamped to Grid
		if ((go->target - go->pos).Length() > 0.1f) {
			Vector3 dir = (go->target - go->pos).Normalized();
			go->pos += dir * go->moveSpeed * (float)dt;
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

	// Simple Shape Mapping
	if (go->type == GameObject::GO_PEASANT) RenderMesh(meshList[GEO_BALL], false); // Green Ball
	else if (go->type == GameObject::GO_ORC) RenderMesh(meshList[GEO_CUBE], false); // Red Cube
	else if (go->type == GameObject::GO_TOWNHALL) RenderMesh(meshList[GEO_CUBE], false); // Big Cube
	else if (go->type == GameObject::GO_RESOURCE_TREE) RenderMesh(meshList[GEO_BALL], false); // Tree
	// Add other mappings...

	modelStack.PopMatrix();
}

void SceneFrontier::Exit()
{
	for (auto go : m_goList) delete go;
	m_goList.clear();
	SceneBase::Exit();
}