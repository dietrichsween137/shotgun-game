#include "pstate.h"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/core/math.hpp"
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
	#define ANIMATION_FINISH_TIME .1
	#define ANIMATION_FRAME(x) x/10.0

	animation_player->clear_queue();

	String current_animation = animation_player->get_current_animation();

	if (current_animation == "walk_first_step") {
		if (animation_player->get_current_animation_position() < ANIMATION_FRAME(2)) {
			animation_player->seek(ANIMATION_FRAME(3));
		}
		animation_player->queue("walk_first_step_to_idle");
		set_animation_finish_time(ANIMATION_FINISH_TIME);
	} else if (current_animation == "walk_second_step") {
		if (animation_player->get_current_animation_position() < ANIMATION_FRAME(2)) {
			animation_player->seek(ANIMATION_FRAME(3));
		}
		animation_player->queue("walk_second_step_to_idle");
		set_animation_finish_time(ANIMATION_FINISH_TIME);
	} else if (current_animation == "jump_land") {
		animation_player->queue("jump_land_to_idle");
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
	#define WALK_ANIMATION_SCALE 1.3
	String current_animation = animation_player->get_current_animation();

	if (current_animation == "jump_land") {
		animation_player->queue("walk_second_step");
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	} else {
		animation_player->play("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	animation_player->set_speed_scale(WALK_ANIMATION_SCALE);

	physics_update(data["delta"]);
}

void PStateWalk::exit() {
	animation_player->set_speed_scale(1.0);
	animation_player->clear_queue();
}

void PStateWalk::handle_input(const Ref<InputEvent> &event) {
	if (event->is_action_pressed("jump")) {
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

	Vector2 velocity = player->get_velocity();
	velocity.y += -player->get_jump_speed();
	player->set_velocity(velocity);

	player->move_and_slide();

	air_time = 0;
}

void PStateJump::exit() {
	animation_player->clear_queue();
}

void PStateJump::handle_input(const Ref<InputEvent> &event) {
	if (event->is_action_released("jump")) {
		air_time = player->get_max_jump_rise_time();
	}
}

void PStateJump::physics_update(double delta) {
	#define JUMP_RISE_REMAINING_PROGRESS_THRES .1
	#define JUMP_FALL_PROGRESS_THRES .1
	static Input* input = Input::get_singleton();

	air_time += delta;

	Vector2 velocity = player->get_velocity();

	if (velocity.y < 0 && Math::abs(velocity.y / player->get_jump_speed()) < JUMP_RISE_REMAINING_PROGRESS_THRES) {
		animation_player->play("jump_crest");
	} else if (velocity.y > 0 && Math::abs(velocity.y / player->get_terminal_velocity()) > JUMP_FALL_PROGRESS_THRES) {
		animation_player->play("jump_fall");
	} else if (velocity.y == player->get_terminal_velocity()) {
		animation_player->play("jump_fall_stretch");
	}

	if (air_time >= player->get_max_jump_rise_time()) {
		velocity.y += player->get_gravity() * delta;
		velocity.y = Math::min((double) velocity.y, player->get_terminal_velocity());
		player->set_velocity(velocity);
	}

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}
