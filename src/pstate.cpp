#include "pstate.h"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/vector2.hpp"

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
	animation_player->clear_queue();
}

void PStateIdle::handle_input(const Ref<InputEvent> &event) {
	if (event.ptr()->is_action_pressed("jump")) {
		emit_signal("switch_state", get_class(), "PStateJump", Dictionary());
	}
}

void PStateIdle::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 0:
			break;
		default:
			Dictionary dict = Dictionary();
			dict["delta"] = delta;
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
	String current_animation = animation_player->get_current_animation();

	if (current_animation == "jump_land") {
		animation_player->queue("walk_second_step");
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	} else {
		animation_player->play("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	physics_update(data["delta"]);
}

void PStateWalk::handle_input(const Ref<InputEvent> &event) {
	if (event.ptr()->is_action_pressed("jump")) {
		emit_signal("switch_state", get_class(), "PStateJump", Dictionary());
	}
}

void PStateWalk::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	int horiz = static_cast<int>(input->get_axis("left", "right"));
	switch (horiz) {
		case -1:
			sprite->set_flip_h(true);
			break;
		case 1:
			sprite->set_flip_h(false);
			break;
		case 0:
			Dictionary dict = Dictionary();
			dict["delta"] = delta;
			emit_signal("switch_state", get_class(), "PStateIdle", dict);
			return;
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
	animation_player->play("jump_start");
	animation_player->queue("jump_rise");

	Vector2 velocity = player->get_velocity();
	velocity.y += -player->get_jump_speed();
	player->set_velocity(velocity);

	player->move_and_slide();
}

void PStateJump::physics_update(double delta) {
	static String last_queued_animation = "";

	Vector2 velocity = player->get_velocity();

	if (last_queued_animation != "jump_crest" &&
		0 > velocity.y &&
		velocity.y > -player->get_jump_speed()) {

		animation_player->queue("jump_crest");
		last_queued_animation = "jump_crest";
		UtilityFunctions::print("here");

	} else if (last_queued_animation != "jump_fall" &&
		0 < velocity.y) {

		animation_player->queue("jump_fall");
		last_queued_animation = "jump_fall";
		UtilityFunctions::print("here2");
	}

	velocity.y += player->get_gravity() * delta;
	player->set_velocity(velocity);

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}
