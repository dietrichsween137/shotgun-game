#include "state.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

using namespace godot;

void StateMachine::_bind_methods() {
	ClassDB::bind_method(D_METHOD("switch_state", "next_state"), &StateMachine::switch_state);
}

void StateMachine::_ready() {
	int child_count {get_child_count()};

	for (int i {0}; i != child_count; i++) {
		Node* child = get_child(i);
		child->connect("switch_state", Callable(this, "switch_state"));

		// First state in the heirachy is default
		if (i == 0) {
			state = static_cast<State*>(child);
		}
	}
}

void StateMachine::physics_update(double delta) {
	state->physics_update(delta);
}

void StateMachine::switch_state(String last_state, String next_state, Dictionary data) {
	state->exit();
	state = get_node<State>(next_state);
	state->enter(last_state, data);
	UtilityFunctions::print(next_state);
}

