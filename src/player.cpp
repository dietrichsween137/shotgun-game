#include "player.h"
#include "state.h"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/property_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ground_accel"), &Player::get_ground_accel);
	ClassDB::bind_method(D_METHOD("set_ground_accel", "p_ground_accel"), &Player::set_ground_accel);
	ClassDB::bind_method(D_METHOD("get_ground_speed"), &Player::get_ground_speed);
	ClassDB::bind_method(D_METHOD("set_ground_speed", "p_ground_speed"), &Player::set_ground_speed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "ground_accel"), "set_ground_accel", "get_ground_accel");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ground_speed"), "set_ground_speed", "get_ground_speed"); 
}

void Player::_ready() {
	state_machine = get_node<StateMachine>("StateMachine");
}

void Player::_physics_process(double delta) {
	state_machine->physics_update(delta);
}

void Player::set_ground_accel(double p_ground_accel) {
	ground_accel = p_ground_accel;
}

double Player::get_ground_accel() const {
	return ground_accel;
}

void Player::set_ground_speed(double p_ground_speed) {
	ground_speed = p_ground_speed;
}

double Player::get_ground_speed() const {
	return ground_speed;
}
