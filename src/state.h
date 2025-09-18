#ifndef STATE_H
#define STATE_H

#include "godot_cpp/classes/animation_player.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/sprite2d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "player.h"

namespace godot {

class State : public Node {
	GDCLASS(State, Node);
protected:
	static void _bind_methods() {}
public:
	State() {}
	~State() {}

	virtual void enter(String last_state, Dictionary data) {};
	virtual void exit() {};
	virtual void handle_input(InputEvent event) {};
	virtual void physics_update(double delta) = 0;
};

class StateMachine : public Node {
	GDCLASS(StateMachine, Node);
private:
	State* state;
protected:
	static void _bind_methods();
public:
	StateMachine(): state {nullptr} {}
	~StateMachine() {}

	void _ready() override;
	void switch_state(String last_state, String next_state, Dictionary data);
	void physics_update(double delta);
};

class PState : public State {
	GDCLASS(PState, State);
protected:
	Player* player;
	AnimationPlayer* animation_player;
	Sprite2D* sprite;

	static void _bind_methods() {};
public:
	PState(): player {nullptr}, animation_player {nullptr}, sprite {nullptr} {}
	~PState() {}

	void _ready() override;
};

class PStateIdle : public PState {
	GDCLASS(PStateIdle, PState);
private:
	void set_animation_finish_time(float time);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void exit() override;
	void physics_update(double delta) override;
};

class PStateWalkRight : public PState {
	GDCLASS(PStateWalkRight, PState);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void physics_update(double delta) override;
};

class PStateWalkLeft : public PState {
	GDCLASS(PStateWalkLeft, PState);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void physics_update(double delta) override;
};

}

#endif
