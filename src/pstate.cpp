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
	player->distance_fallen = -1;

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
		emit_signal("switch_state", get_class(), "PStateJumpRise", Dictionary());
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
		emit_signal("switch_state", get_class(), "PStateJumpRise", Dictionary());
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
	velocity.x = horiz * player->get_ground_speed();
	player->set_velocity(velocity);

	player->move_and_slide();
}

void PStateJumpRise::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateJumpRise::enter(String next_state, Dictionary data) {
	animation_player->play("jump_start");

	Vector2 velocity = player->get_velocity();
	velocity.y += -player->get_jump_speed();
	player->set_velocity(velocity);

	air_time = 0;
}

void PStateJumpRise::handle_input(const Ref<InputEvent> &event) {
	if (event->is_action_released("jump")) {
		air_time = player->get_max_jump_rise_time();
	}
}

void PStateJumpRise::physics_update(double delta) {
	#define JUMP_RISE_REMAINING_PROGRESS_THRES .1
	#define JUMP_FALL_PROGRESS_THRES .1
	static Input* input = Input::get_singleton();

	air_time += delta;


	if (air_time >= player->get_max_jump_rise_time()) {
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateJumpCrest", dict);
	}

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}

void PStateJumpCrest::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateJumpCrest::enter(String next_state, Dictionary data) {
	physics_update(data["delta"]);
}

void PStateJumpCrest::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	Vector2 velocity = player->get_velocity();
	velocity.y += player->get_gravity() * delta;
	velocity.y = Math::min((double) velocity.y, player->get_terminal_velocity());

	if (animation_player->get_current_animation() != "jump_crest" &&
		velocity.y >= 0) {
		animation_player->play("jump_crest");
		player->distance_fallen = 0;
	}

	if (player->distance_fallen >= 0) {
		player->distance_fallen += velocity.y * delta;
	}

	int horiz = static_cast<int>(input->get_axis("left", "right"));
	switch (horiz) {
		case -1:
			sprite->set_flip_h(true);
			break;
		case 1:
			sprite->set_flip_h(false);
			break;
	}

	if (horiz != 0) {
		velocity.x += horiz * player->get_aerial_accel() * delta;

		if (std::abs(velocity.x) >= player->get_ground_speed()) {
			velocity.x = horiz * player->get_ground_speed();
		}
	}

	player->set_velocity(velocity);

	if (velocity.y > 0 && velocity.y / player->get_terminal_velocity() > .25) {
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateJumpFall", dict);
	}

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}

void PStateJumpFall::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateJumpFall::enter(String next_state, Dictionary data) {
	animation_player->queue("jump_fall");
	physics_update(data["delta"]);
}

void PStateJumpFall::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	Vector2 velocity = player->get_velocity();
	velocity.y += player->get_gravity() * delta;
	velocity.y = Math::min((double) velocity.y, player->get_terminal_velocity());

	if (animation_player->get_current_animation() != "jump_fall_stretch" &&
		velocity.y >= player->get_terminal_velocity()) {

		animation_player->queue("jump_fall_stretch");
	}

	if (player->distance_fallen >= 0) {
		player->distance_fallen += velocity.y * delta;
	}

	int horiz = static_cast<int>(input->get_axis("left", "right"));
	switch (horiz) {
		case -1:
			sprite->set_flip_h(true);
			break;
		case 1:
			sprite->set_flip_h(false);
			break;
	}

	if (horiz != 0) {
		velocity.x += horiz * player->get_aerial_accel() * delta;

		if (std::abs(velocity.x) >= player->get_ground_speed()) {
			velocity.x = horiz * player->get_ground_speed();
		}
	}

	player->set_velocity(velocity);

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}
