#pragma once
#include "State.h"
#include "GameObject.h"

// --- Peasant States ---
class StatePeasantIdle : public State {
	GameObject* m_go;
public:
	StatePeasantIdle(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePeasantGather : public State {
	GameObject* m_go;
public:
	StatePeasantGather(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePeasantReturn : public State {
	GameObject* m_go;
public:
	StatePeasantReturn(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// --- Knight States ---
class StateKnightPatrol : public State {
	GameObject* m_go;
	std::vector<Vector3> waypoints;
	int currentWP;
public:
	StateKnightPatrol(const std::string& stateID, GameObject* go) : State(stateID), m_go(go), currentWP(0) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateKnightChase : public State {
	GameObject* m_go;
public:
	StateKnightChase(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateKnightAttack : public State {
	GameObject* m_go;
public:
	StateKnightAttack(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

// --- Cleric States ---
class StateClericHeal : public State {
	GameObject* m_go;
public:
	StateClericHeal(const std::string& stateID, GameObject* go) : State(stateID), m_go(go) {}
	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};