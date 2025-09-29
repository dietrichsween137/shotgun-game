#ifndef PLAYER_H
#define PLAYER_H

#include "godot_cpp/classes/character_body2d.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/ref.hpp"

namespace godot {

class StateMachine;

class Player : public CharacterBody2D {
	GDCLASS(Player, CharacterBody2D)
private:
	StateMachine* state_machine;

	double ground_speed;
	double jump_speed;
	double gravity;
protected:
	static void _bind_methods();
public:
	Player();
	~Player() {}

	void _ready() override;
	void _input(const Ref<InputEvent> &event) override;
	void _physics_process(double delta) override;

	void set_ground_speed(const double p_ground_speed);
	double get_ground_speed() const;

	void set_jump_speed(const double p_jump_speed);
	double get_jump_speed() const;

	void set_gravity(const double p_gravity);
	double get_gravity() const;
};

}

#endif
