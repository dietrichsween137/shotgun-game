#ifndef PLAYER_H
#define PLAYER_H

#include "godot_cpp/classes/character_body2d.hpp"

namespace godot {

class StateMachine;

class Player : public CharacterBody2D {
	GDCLASS(Player, CharacterBody2D)
private:
	StateMachine* state_machine;

	int ground_speed;
	int jump_speed;
	int gravity;
protected:
	static void _bind_methods();
public:
	Player();
	~Player() {}

	void _ready() override;
	void _physics_process(double delta) override;

	void set_ground_speed(const int p_ground_speed);
	int get_ground_speed() const;

	void set_jump_speed(const int p_jump_speed);
	int get_jump_speed() const;

	void set_gravity(const int p_gravity);
	int get_gravity() const;
};

}

#endif
