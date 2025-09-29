#ifndef STATE_H
#define STATE_H

#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "player.h"

namespace godot {

class State : public Node {
	GDCLASS(State, Node);
protected:
	static void _bind_methods() {}
public:
	State() {};
	~State() {};

	virtual void enter(String last_state, Dictionary data) {};
	virtual void exit() {};
	virtual void handle_input(const Ref<InputEvent> &event) {};
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
	void handle_input(const Ref<InputEvent> &event);
	void switch_state(String last_state, String next_state, Dictionary data);
	void physics_update(double delta);
};

}

#endif
