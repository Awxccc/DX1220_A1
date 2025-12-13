#include "StatesKingdom.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"
#include "SceneData.h" 

// --- PEASANT ---
void StatePeasantIdle::Enter() { m_go->moveSpeed = 0; }
void StatePeasantIdle::Update(double dt) {
	// Ask Scene for nearest Tree
	if (!m_go->nearest || !m_go->nearest->active) {
		MessageFindNearest msg(m_go, GameObject::GO_RESOURCE_TREE);
		PostOffice::GetInstance()->Send("Scene", &msg);
	}
	if (m_go->nearest && m_go->nearest->active) m_go->sm->SetNextState("Gather");
}
void StatePeasantIdle::Exit() {}

void StatePeasantGather::Enter() { m_go->moveSpeed = 5.0f; }
void StatePeasantGather::Update(double dt) {
	if (!m_go->nearest || !m_go->nearest->active) {
		m_go->sm->SetNextState("Idle");
		return;
	}
	m_go->target = m_go->nearest->pos;
	if ((m_go->pos - m_go->target).Length() < 2.0f) {
		m_go->inventory += 5;
		m_go->nearest->active = false; // Tree chopped
		m_go->sm->SetNextState("Return");
	}
}
void StatePeasantGather::Exit() {}

void StatePeasantReturn::Enter() { m_go->moveSpeed = 5.0f; }
void StatePeasantReturn::Update(double dt) {
	// Hardcoded TownHall check or via message
	MessageFindNearest msg(m_go, GameObject::GO_TOWNHALL);
	PostOffice::GetInstance()->Send("Scene", &msg);

	if (m_go->nearest) {
		m_go->target = m_go->nearest->pos;
		if ((m_go->pos - m_go->target).Length() < 3.0f) {
			// Drop resources
			MessageRaidSuccess msgDrop(m_go->inventory); // Reusing message for generic resource drop
			PostOffice::GetInstance()->Send("Scene", &msgDrop); // Scene handles adding to score
			m_go->inventory = 0;
			m_go->sm->SetNextState("Idle");
		}
	}
}
void StatePeasantReturn::Exit() {}

// --- KNIGHT ---
void StateKnightPatrol::Enter() {
	m_go->moveSpeed = 4.0f;
	if (waypoints.empty()) {
		waypoints.push_back(Vector3(20, 20, 0));
		waypoints.push_back(Vector3(20, 80, 0));
		waypoints.push_back(Vector3(50, 50, 0));
	}
}
void StateKnightPatrol::Update(double dt) {
	// Patrol Logic
	m_go->target = waypoints[currentWP];
	if ((m_go->pos - m_go->target).Length() < 1.0f) {
		currentWP = (currentWP + 1) % waypoints.size();
	}

	// Scan for enemies
	MessageFindNearest msg(m_go, GameObject::GO_ORC);
	PostOffice::GetInstance()->Send("Scene", &msg);

	if (m_go->nearest && m_go->nearest->active && (m_go->pos - m_go->nearest->pos).Length() < 15.0f) {
		m_go->sm->SetNextState("Chase");
	}
}
void StateKnightPatrol::Exit() {}

void StateKnightChase::Enter() { m_go->moveSpeed = 8.0f; }
void StateKnightChase::Update(double dt) {
	if (!m_go->nearest || !m_go->nearest->active) {
		m_go->sm->SetNextState("Patrol");
		return;
	}
	m_go->target = m_go->nearest->pos;
	if ((m_go->pos - m_go->target).Length() < 2.0f) {
		m_go->sm->SetNextState("Attack");
	}
}
void StateKnightChase::Exit() {}

void StateKnightAttack::Enter() { m_go->moveSpeed = 0.0f; }
void StateKnightAttack::Update(double dt) {
	if (!m_go->nearest || !m_go->nearest->active) {
		m_go->sm->SetNextState("Patrol");
		return;
	}
	m_go->attackCooldown -= dt;
	if (m_go->attackCooldown <= 0) {
		MessageAttack msg(m_go, m_go->nearest, 10.0f);
		PostOffice::GetInstance()->Send("Scene", &msg);
		m_go->attackCooldown = 1.0f;
	}
	// Simple distance check to see if enemy fled
	if ((m_go->pos - m_go->nearest->pos).Length() > 3.0f) {
		m_go->sm->SetNextState("Chase");
	}
}
void StateKnightAttack::Exit() {}

// --- CLERIC ---
void StateClericHeal::Enter() { m_go->moveSpeed = 3.0f; }
void StateClericHeal::Update(double dt) {
	// Find injured knight
	MessageFindNearest msg(m_go, GameObject::GO_KNIGHT);
	PostOffice::GetInstance()->Send("Scene", &msg);

	if (m_go->nearest && m_go->nearest->hp < m_go->nearest->maxHp) {
		m_go->target = m_go->nearest->pos;
		if ((m_go->pos - m_go->target).Length() < 4.0f) {
			m_go->nearest->hp += 10.0f * dt; // Heal over time
		}
	}
	else {
		// Follow Town Hall if no one needs healing
		MessageFindNearest msgTH(m_go, GameObject::GO_TOWNHALL);
		PostOffice::GetInstance()->Send("Scene", &msgTH);
		if (m_go->nearest) m_go->target = m_go->nearest->pos;
	}
}
void StateClericHeal::Exit() {}