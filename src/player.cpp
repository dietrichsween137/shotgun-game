#include "player.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include "state.h"
#include "godot_cpp/core/property_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Player::Player():ground_speed {150},
		 jump_speed {290},
		 gravity {1750},
		 max_jump_rise_time {.12},
		 terminal_velocity {260},
		 distance_fallen {-1} {};

void Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ground_speed"), &Player::get_ground_speed);
	ClassDB::bind_method(D_METHOD("set_ground_speed", "p_ground_speed"), &Player::set_ground_speed);
	ClassDB::bind_method(D_METHOD("get_jump_speed"), &Player::get_jump_speed);
	ClassDB::bind_method(D_METHOD("set_jump_speed", "p_jump_speed"), &Player::set_jump_speed);
	ClassDB::bind_method(D_METHOD("get_gravity"), &Player::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Player::set_gravity);
	ClassDB::bind_method(D_METHOD("get_max_jump_rise_time"), &Player::get_max_jump_rise_time);
	ClassDB::bind_method(D_METHOD("set_max_jump_rise_time", "p_max_jump_rise_time"), &Player::set_max_jump_rise_time);
	ClassDB::bind_method(D_METHOD("get_terminal_velocity"), &Player::get_terminal_velocity);
	ClassDB::bind_method(D_METHOD("set_terminal_velocity", "p_terminal_velocity"), &Player::set_terminal_velocity);
	ClassDB::bind_method(D_METHOD("get_aerial_accel"), &Player::get_aerial_accel);
	ClassDB::bind_method(D_METHOD("set_aerial_accel", "p_aerial_accel"), &Player::set_aerial_accel);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ground_speed"), "set_ground_speed", "get_ground_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_speed"), "set_jump_speed", "get_jump_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_jump_rise_time"), "set_max_jump_rise_time", "get_max_jump_rise_time");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terminal_velocity"), "set_terminal_velocity", "get_terminal_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "aerial_accel"), "set_aerial_accel", "get_aerial_accel");
}

void Player::_ready() {
	state_machine = get_node<StateMachine>("StateMachine");
}

void Player::_input(const Ref<InputEvent> &event) {
	state_machine->handle_input(event);
}

void Player::_physics_process(double delta) {
	state_machine->physics_update(delta);
}

void Player::set_ground_speed(double p_ground_speed) {
	ground_speed = p_ground_speed;
	UtilityFunctions::print(ground_speed);
}

double Player::get_ground_speed() const {
	return ground_speed;
}

void Player::set_jump_speed(const double p_jump_speed) {
	jump_speed = p_jump_speed;
}

double Player::get_jump_speed() const {
	return jump_speed;
}

void Player::set_gravity(const double p_gravity) {
	gravity = p_gravity;
}

double Player::get_gravity() const {
	return gravity;
}

void Player::set_max_jump_rise_time(const double p_max_jump_rise_time) {
	max_jump_rise_time = p_max_jump_rise_time;
}

double Player::get_max_jump_rise_time() const {
	return max_jump_rise_time;
}

void Player::set_terminal_velocity(const double p_terminal_velocity) {
	terminal_velocity = p_terminal_velocity;
}

double Player::get_terminal_velocity() const {
	return terminal_velocity;
}

void Player::set_aerial_accel(const double p_aerial_accel) {
	aerial_accel = p_aerial_accel;
}

double Player::get_aerial_accel() const {
	return aerial_accel;
}
