#ifndef PSTATE_H
#define PSTATE_H

#include "state.h"
#include "godot_cpp/classes/animation_player.hpp"
#include "godot_cpp/classes/sprite2d.hpp"

namespace godot {

class PState : public State {
	GDCLASS(PState, State);
protected:
	Player* player;
	AnimationPlayer* animation_player;
	Sprite2D* sprite;

	static void _bind_methods() {};
public:
	PState(): player {nullptr}, animation_player {nullptr}, sprite {nullptr} {}
	~PState() {}

	void _ready() override;
};

class PStateIdle : public PState {
	GDCLASS(PStateIdle, PState);
private:
	void set_animation_finish_time(float time);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void exit() override;
	void physics_update(double delta) override;
};

class PStateWalk : public PState {
	GDCLASS(PStateWalk, PState);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void physics_update(double delta) override;
};

class PStateJump : public PState {
	GDCLASS(PStateJump, PState);
protected:
	static void _bind_methods();
public:
	void enter(String next_state, Dictionary data) override;
	void physics_update(double delta) override;
};

}

#endif
