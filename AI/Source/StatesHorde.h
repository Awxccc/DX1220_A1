#pragma once
#include "State.h"
#include "GameObject.h"
// --- Goblin ---
class StateGoblinProwl : public State {
	GameObject* m_go;
public:
	StateGoblinProwl(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateGoblinSteal : public State {
	GameObject* m_go;
public:
	StateGoblinSteal(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// --- Orc ---
class StateOrcSmash : public State {
	GameObject* m_go;
public:
	StateOrcSmash(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// --- Shaman ---
class StateShamanRitual : public State {
	GameObject* m_go;
public:
	StateShamanRitual(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};