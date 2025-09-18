#ifndef PLAYER_H	
#define PLAYER_H	

#include "godot_cpp/classes/character_body2d.hpp"
#include "godot_cpp/variant/vector2.hpp"

namespace godot {

class StateMachine;

class Player : public CharacterBody2D {
	GDCLASS(Player, CharacterBody2D)
private:
	StateMachine* state_machine;

	int ground_speed;
	int ground_accel;
protected:
	static void _bind_methods();
public:
	Player(): ground_speed {100}, ground_accel {10} {}
	~Player() {}
	
	void _ready() override;
	void _physics_process(double delta) override;

	void set_ground_speed(const double p_ground_speed);
	double get_ground_speed() const;

	void set_ground_accel(const double p_ground_accel);
	double get_ground_accel() const;
};

}

#endif
