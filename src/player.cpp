#include "player.h"
#include "state.h"
#include "godot_cpp/core/property_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Player::Player():ground_speed {100}, jump_speed {200}, gravity {500} {};

void Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ground_speed"), &Player::get_ground_speed);
	ClassDB::bind_method(D_METHOD("set_ground_speed", "p_ground_speed"), &Player::set_ground_speed);
	ClassDB::bind_method(D_METHOD("get_jump_speed"), &Player::get_jump_speed);
	ClassDB::bind_method(D_METHOD("set_jump_speed", "p_jump_speed"), &Player::set_jump_speed);
	ClassDB::bind_method(D_METHOD("get_gravity"), &Player::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Player::set_gravity);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ground_speed"), "set_ground_speed", "get_ground_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_speed"), "set_jump_speed", "get_jump_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
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
