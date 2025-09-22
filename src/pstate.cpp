#include "pstate.h"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/classes/input.hpp"

using namespace godot;

void PState::_ready() {
	player = get_node<Player>("../..");
	animation_player = get_node<AnimationPlayer>("../../AnimationPlayer");
	sprite = get_node<Sprite2D>("../../Sprite2D");
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
		case 0:
			break;
		default:
			emit_signal("switch_state", get_class(), "PStateWalk", dict);
			return;
	}

	if (animation_player->get_current_animation() == "idle") {
		animation_player->set_speed_scale(1.0);
	}
}

void PStateWalk::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateWalk::enter(String last_state, Dictionary data) {
	animation_player->play("walk_first_step");
	animation_player->queue("walk_second_step");

	physics_update(data["delta"]);
}

void PStateWalk::physics_update(double delta) {
	static Input* input = Input::get_singleton();
	Dictionary dict = Dictionary();
	dict["delta"] = delta;

	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 0:
			emit_signal("switch_state", get_class(), "PStateIdle", dict);
			return;
		case -1:
			sprite->set_flip_h(true);
			break;
		case 1:
			sprite->set_flip_h(false);
			break;
	}

	if (animation_player->get_queue().size() == 0) {
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	Vector2 velocity = player->get_velocity();
	if (sprite->is_flipped_h()) {
		velocity.x = -player->get_ground_speed();
	} else {
		velocity.x = player->get_ground_speed();
	}
	player->set_velocity(velocity);
	player->move_and_slide();
}

void PStateJump::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateJump::enter(String next_state, Dictionary data) {
	physics_update(data["delta"]);
}

void PStateJump::physics_update(double delta) {
}
