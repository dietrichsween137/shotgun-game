#include "pstate.h"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/viewport.hpp"
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

	#undef ANIMATION_FINISH_TIME
	#undef ANIMATION_FRAME

	animation_player->queue("idle");

}

void PStateIdle::exit() {
	animation_player->set_speed_scale(1.0);
	animation_player->clear_queue();
}

void PStateIdle::handle_input(const Ref<InputEvent> &event) {
	if (event.ptr()->is_action_pressed("jump")) {
		emit_signal("switch_state", get_class(), "PStateJumpRise", Dictionary());
	} else if (event.ptr()->is_action_pressed("click")) {
		Dictionary dict = Dictionary();
		dict["mouse_pos"] = player->get_viewport()->get_mouse_position();
		emit_signal("switch_state", get_class(), "PStateFire", dict);
	}
}

void PStateIdle::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	if (animation_player->get_current_animation() == "idle") {
		animation_player->set_speed_scale(1.0);
	}

	Vector2 velocity = player->get_velocity();

	int horiz = static_cast<int>(input->get_axis("left", "right"));

	switch (horiz) {
		case 0:
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity = Vector2(0, 0);
			} else if (player->get_ground_speed() < velocity.x) {
				velocity.x -= player->get_ground_accel() * delta;
			} else {
				velocity.x += player->get_ground_accel() * delta;
			}
			break;
		default:
			Dictionary dict = Dictionary();
			dict["delta"] = delta;
			emit_signal("switch_state", get_class(), "PStateWalk", dict);
			break;
	}

	player->set_velocity(velocity);

	player->move_and_slide();

	if (!player->is_on_floor()) {
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateJumpFall", dict);
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
}

void PStateWalk::exit() {
	animation_player->set_speed_scale(1.0);
	animation_player->clear_queue();
}

void PStateWalk::handle_input(const Ref<InputEvent> &event) {
	if (event->is_action_pressed("jump")) {
		emit_signal("switch_state", get_class(), "PStateJumpRise", Dictionary());
	} else if (event.ptr()->is_action_pressed("click")) {
		Dictionary dict = Dictionary();
		dict["mouse_pos"] = player->get_viewport()->get_mouse_position();
		emit_signal("switch_state", get_class(), "PStateFire", dict);
	}
}

void PStateWalk::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	if (animation_player->get_queue().size() == 0) {
		animation_player->queue("walk_first_step");
		animation_player->queue("walk_second_step");
	}

	Vector2 velocity = player->get_velocity();

	#define GROUND_INPUT_ACCEL_SCALE .5

	int horiz = static_cast<int>(input->get_axis("left", "right"));
	switch (horiz) {
		case -1:
			sprite->set_flip_h(true);
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x = horiz * player->get_ground_speed();
			} else if (player->get_ground_speed() < velocity.x ) {
				velocity.x -= (1.0 / GROUND_INPUT_ACCEL_SCALE) * player->get_ground_accel() * delta;
			} else {
				velocity.x += GROUND_INPUT_ACCEL_SCALE * player->get_ground_accel() * delta;
			}
			break;
		case 1:
			sprite->set_flip_h(false);
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x = horiz * player->get_ground_speed();
			} else if (player->get_ground_speed() < velocity.x ) {
				velocity.x -= GROUND_INPUT_ACCEL_SCALE * player->get_ground_accel() * delta;
			} else {
				velocity.x += (1.0 / GROUND_INPUT_ACCEL_SCALE) * player->get_ground_accel() * delta;
			}
			break;
		case 0:
			Dictionary dict = Dictionary();
			dict["delta"] = delta;
			emit_signal("switch_state", get_class(), "PStateIdle", dict);
			break;
	}

	#undef GROUND_INPUT_ACCEL_SCALE

	player->set_velocity(velocity);

	player->move_and_slide();

	if (!player->is_on_floor()) {
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateJumpFall", dict);
	}
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
	} else if (event.ptr()->is_action_pressed("click")) {
		Dictionary dict = Dictionary();
		dict["mouse_pos"] = player->get_viewport()->get_mouse_position();
		emit_signal("switch_state", get_class(), "PStateFire", dict);
	}
}

void PStateJumpRise::physics_update(double delta) {
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

void PStateJumpCrest::handle_input(const Ref<InputEvent> &event) {
	if (event.ptr()->is_action_pressed("click")) {
		Dictionary dict = Dictionary();
		dict["mouse_pos"] = player->get_viewport()->get_mouse_position();
		emit_signal("switch_state", get_class(), "PStateFire", dict);
	}
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

	#define AERIAL_INPUT_ACCEL_SCALE .5

	int horiz = static_cast<int>(input->get_axis("left", "right"));
	switch (horiz) {
		case -1:
			sprite->set_flip_h(true);
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x -= player->get_aerial_accel() * delta;
			} else if (velocity.x < -player->get_ground_speed()) {
				velocity.x += AERIAL_INPUT_ACCEL_SCALE * player->get_aerial_accel() * delta;
			} else {
				velocity.x -= (1.0 / AERIAL_INPUT_ACCEL_SCALE) * player->get_aerial_accel() * delta;
			}
			break;
		case 1:
			sprite->set_flip_h(false);
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x += player->get_aerial_accel() * delta;
			} else if (velocity.x < -player->get_ground_speed()) {
				velocity.x += (1.0 / AERIAL_INPUT_ACCEL_SCALE) * player->get_aerial_accel() * delta;
			} else {
				velocity.x -= AERIAL_INPUT_ACCEL_SCALE * player->get_aerial_accel() * delta;
			}
			break;
		case 0:
			if (velocity.x < 0) {
				velocity.x += player->get_aerial_accel() * delta;
			} else if (velocity.x > 0) {
				velocity.x -= player->get_aerial_accel() * delta;
			}
			break;
	}


	player->set_velocity(velocity);

	player->move_and_slide();

	#define PERCENTAGE_TERMINAL_VELOCITY .25

	if (velocity.y > 0 && velocity.y / player->get_terminal_velocity() > PERCENTAGE_TERMINAL_VELOCITY) {
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateJumpFall", dict);
	} else if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}

	#undef PERCENTAGE_TERMINAL_VELOCITY
}

void PStateJumpFall::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateJumpFall::enter(String last_state, Dictionary data) {
	animation_player->queue("jump_fall");
	if (last_state == "PStateWalk") {
		coyote_time = 0;
		initial_velocity = player->get_velocity();
	}
}

void PStateJumpFall::exit() {
	coyote_time = (player->get_coyote_time());
}

void PStateJumpFall::handle_input(const Ref<InputEvent> &event) {
	if (event.ptr()->is_action_pressed("jump") && coyote_time < player->get_coyote_time()) {
		UtilityFunctions::print("Coyote time used");
		player->set_velocity(initial_velocity);
		emit_signal("switch_state", get_class(), "PStateJumpRise", Dictionary());
	} else if (event.ptr()->is_action_pressed("click")) {
		Dictionary dict = Dictionary();
		dict["mouse_pos"] = player->get_viewport()->get_mouse_position();
		emit_signal("switch_state", get_class(), "PStateFire", dict);
	}
}

void PStateJumpFall::physics_update(double delta) {
	static Input* input = Input::get_singleton();

	if (coyote_time < player->get_coyote_time()) {
		coyote_time += delta;
	}

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
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x -= player->get_aerial_accel() * delta;
			} else if (velocity.x < -player->get_ground_speed()) {
				velocity.x += AERIAL_INPUT_ACCEL_SCALE * player->get_aerial_accel() * delta;
			} else {
				velocity.x -= (1.0 / AERIAL_INPUT_ACCEL_SCALE) * player->get_aerial_accel() * delta;
			}
			break;
		case 1:
			sprite->set_flip_h(false);
			if (-player->get_ground_speed() <= velocity.x && velocity.x <= player->get_ground_speed()) {
				velocity.x += player->get_aerial_accel() * delta;
			} else if (velocity.x < -player->get_ground_speed()) {
				velocity.x += (1.0 / AERIAL_INPUT_ACCEL_SCALE) * player->get_aerial_accel() * delta;
			} else {
				velocity.x -= AERIAL_INPUT_ACCEL_SCALE * player->get_aerial_accel() * delta;
			}
			break;
		case 0:
			if (velocity.x < 0) {
				velocity.x += player->get_aerial_accel() * delta;
			} else if (velocity.x > 0) {
				velocity.x -= player->get_aerial_accel() * delta;
			}
			break;
	}

	#undef AERIAL_INPUT_ACCEL_SCALE

	player->set_velocity(velocity);

	player->move_and_slide();

	if (player->is_on_floor()) {
		animation_player->play("jump_land");
		Dictionary dict = Dictionary();
		dict["delta"] = delta;
		emit_signal("switch_state", get_class(), "PStateIdle", dict);
	}
}
void PStateFire::_bind_methods() {
	ADD_SIGNAL(MethodInfo("switch_state",
		       PropertyInfo(Variant::STRING, "last_state"),
		       PropertyInfo(Variant::STRING, "next_state"),
		       PropertyInfo(Variant::DICTIONARY, "data")));
}

void PStateFire::enter(String last_state, Dictionary data) {

	// Calulate which fire angle mouse click location corresponds to
	Vector2 mouse_pos = data["mouse_pos"];
	Vector2 player_pos = player->get_global_transform_with_canvas().get_origin();

	const std::vector<double> fire_angles {0.0, 3.14/4.0, 3.14/2.0, 3.14*3.0/4.0};
	std::vector<double> distances {};

	for (double fire_angle : fire_angles) {
		distances.push_back(Math::abs(cos(fire_angle) * (player_pos.y - mouse_pos.y) - sin(fire_angle) * (player_pos.x - mouse_pos.x)));
	}

	int index_of_min = 0;
	double min = distances[0];
	for (int i = 1; i < distances.size(); i++) {
		if (min > distances[i]) {
			min = distances[i];
			index_of_min = i;
		}
	}

	// Instead of relying on sine and cosine signs, just manually make sure the kickback goes away from the shot (which is towards the click)
	int horiz;
	if (player_pos.x < mouse_pos.x) {
		horiz = -1;
	} else {
		horiz = 1;
	}
	int vert;
	if (player_pos.y < mouse_pos.y) {
		vert = -1;
	} else {
		vert = 1;
	}

	if (horiz == 1) {
		sprite->set_flip_h(true);
	} else {
		sprite->set_flip_h(false);
	}

	Vector2 velocity = player->get_velocity();
	velocity.x = horiz * Math::abs(cos(fire_angles[index_of_min])) * player->get_fire_speed();
	velocity.y = vert * Math::abs(sin(fire_angles[index_of_min])) * player->get_fire_speed();

	player->set_velocity(velocity);

	air_time = 0;
}

void PStateFire::physics_update(double delta) {
	air_time += delta;
	if (air_time >= player->get_max_fire_air_time()) {
		emit_signal("switch_state", get_class(), "PStateJumpCrest", Dictionary());
	}
	player->move_and_slide();
}
