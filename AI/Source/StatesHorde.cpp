#include "StatesHorde.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

// --- GOBLIN ---
void StateGoblinProwl::Enter() { m_go->moveSpeed = 6.0f; }
void StateGoblinProwl::Update(double dt) {
	// Move West towards Town Hall
	MessageFindNearest msg(m_go, GameObject::GO_TOWNHALL);
	PostOffice::GetInstance()->Send("Scene", &msg);

	if (m_go->nearest) {
		m_go->target = m_go->nearest->pos;
		if ((m_go->pos - m_go->target).Length() < 2.0f) {
			m_go->sm->SetNextState("Steal");
		}
	}

	// Evasion Logic: Check for Knights
	MessageFindNearest msgRun(m_go, GameObject::GO_KNIGHT);
	PostOffice::GetInstance()->Send("Scene", &msgRun);
	if (m_go->nearest && (m_go->pos - m_go->nearest->pos).Length() < 8.0f) {
		// Run away!
		Vector3 dir = (m_go->pos - m_go->nearest->pos).Normalized();
		m_go->target = m_go->pos + dir * 5.0f;
	}
}
void StateGoblinProwl::Exit() {}

void StateGoblinSteal::Enter() { m_go->moveSpeed = 0.0f; }
void StateGoblinSteal::Update(double dt) {
	// Steal logic handled via timer or message
	m_go->attackCooldown -= dt;
	if (m_go->attackCooldown <= 0) {
		m_go->inventory += 10;
		// Teleport back to start for simplicity in simulation loop
		// Realistically, they would walk back (add StateReturn)
		MessageRaidSuccess msg(m_go->inventory);
		PostOffice::GetInstance()->Send("Scene", &msg);
		m_go->inventory = 0;
		m_go->attackCooldown = 5.0f; // Takes time to steal again
	}
}
void StateGoblinSteal::Exit() {}

// --- ORC ---
void StateOrcSmash::Enter() { m_go->moveSpeed = 3.0f; }
void StateOrcSmash::Update(double dt) {
	// Find nearest Human
	MessageFindNearest msg(m_go, GameObject::GO_KNIGHT);
	PostOffice::GetInstance()->Send("Scene", &msg);

	if (!m_go->nearest) {
		// If no knights, go for town hall
		MessageFindNearest msg2(m_go, GameObject::GO_TOWNHALL);
		PostOffice::GetInstance()->Send("Scene", &msg2);
	}

	if (m_go->nearest && m_go->nearest->active) {
		m_go->target = m_go->nearest->pos;
		if ((m_go->pos - m_go->target).Length() < 2.0f) {
			MessageAttack msgAtk(m_go, m_go->nearest, 15.0f);
			PostOffice::GetInstance()->Send("Scene", &msgAtk);
		}
	}
}
void StateOrcSmash::Exit() {}

// --- SHAMAN ---
void StateShamanRitual::Enter() { m_go->moveSpeed = 1.0f; }
void StateShamanRitual::Update(double dt) {
	// Spawns new Goblins every 10 seconds
	m_go->attackCooldown -= dt;
	if (m_go->attackCooldown <= 0) {
		MessageSpawnUnit msg(2, 0); // Team 2 (Horde), 0 Cost
		PostOffice::GetInstance()->Send("Scene", &msg);
		m_go->attackCooldown = 10.0f;
	}
}
void StateShamanRitual::Exit() {}