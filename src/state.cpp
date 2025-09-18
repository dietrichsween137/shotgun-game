#include "state.h"
#include "godot_cpp/classes/animation_player.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;


void PState::_ready() {
	player = get_node<Player>("../..");
	animation_player = get_node<AnimationPlayer>("../../AnimationPlayer");
	sprite = get_node<Sprite2D>("../../Sprite2D");
}

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
}

void PStateIdle::set_animation_finish_time(float time) {
	float remaining_time = animation_player->get_current_animation_length() - animation_player->get_current_animation_position();
	if (remaining_time > time) {
		animation_player->set_speed_scale(remaining_time / time);
	}
}

void PStateIdle::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateIdle::enter(String last_state, Dictionary data) {
	UtilityFunctions::print(get_class());
	animation_player->clear_queue();
	
	String current_animation = animation_player->get_current_animation();

	if (current_animation == "walk_first_step") {
		if (animation_player->get_current_animation_position() < .2) {
			animation_player->seek(.3);
		}
		animation_player->queue("walk_first_step_to_idle");
		set_animation_finish_time(.1);
	} else if (current_animation == "walk_second_step") {
		if (animation_player->get_current_animation_position() < .2) {
			animation_player->seek(.3);
		}
		animation_player->queue("walk_second_step_to_idle");
		set_animation_finish_time(.1);
	}

	animation_player->queue("idle");

	player->set_velocity(Vector2(0, 0));
}

void PStateIdle::exit() {
	animation_player->set_speed_scale(1.0);
}

void PStateIdle::physics_update(double delta) {
	static Input* input = Input::get_singleton();
	Dictionary dict = Dictionary();
	dict["delta"] = delta;
	
	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 1:
			emit_signal("switch_state", get_class(), "PStateWalkRight", dict);
			return;
		case -1:
			emit_signal("switch_state", get_class(), "PStateWalkLeft", dict);
			return;
	}

	if (animation_player->get_current_animation() == "idle") {
		animation_player->set_speed_scale(1.0);
	}
}

void PStateWalkRight::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateWalkRight::enter(String last_state, Dictionary data) {
	UtilityFunctions::print(get_class());
	animation_player->play("walk_first_step");
	animation_player->queue("walk_second_step");
	sprite->set_flip_h(false);

	physics_update(data["delta"]);
}

void PStateWalkRight::physics_update(double delta) {
	static Input* input = Input::get_singleton();
	Dictionary dict = Dictionary();
	dict["delta"] = delta;
	
	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 0:
			emit_signal("switch_state", get_class(), "PStateIdle", dict); 
			return;
		case -1:
			emit_signal("switch_state", get_class(), "PStateWalkLeft", dict); 
			return;
	}

	if (animation_player->get_queue().size() == 0) {
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	Vector2 velocity = player->get_velocity();
	velocity.x = Math::min(velocity.x + player->get_ground_accel() * delta, player->get_ground_speed());
	player->set_velocity(velocity);
	player->move_and_slide();
}

void PStateWalkLeft::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateWalkLeft::enter(String last_state, Dictionary data) {
	UtilityFunctions::print(get_class());
	animation_player->play("walk_first_step");
	animation_player->queue("walk_second_step");
	sprite->set_flip_h(true);

	physics_update(data["delta"]);
}

void PStateWalkLeft::physics_update(double delta) {
	static Input* input = Input::get_singleton();
	Dictionary dict = Dictionary();
	dict["delta"] = delta;
	
	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 1:
			emit_signal("switch_state", get_class(), "PStateWalkRight", dict); 
			return;
		case 0:
			emit_signal("switch_state", get_class(), "PStateIdle", dict); 
			return;
	}

	if (animation_player->get_queue().size() == 0) {
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	Vector2 velocity = player->get_velocity();
	velocity.x = Math::max(velocity.x - player->get_ground_accel() * delta, -player->get_ground_speed());
	player->set_velocity(velocity);
	player->move_and_slide();
}
