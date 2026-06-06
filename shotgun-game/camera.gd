extends Camera2D

@export var target:Node2D
@export var vertical_scale:float = 1
@export var horizontal_scale:float = 1
@export var horizontal_lead_scale:float = 60

@onready var last_target_position:Vector2 = target.position


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	var velocity:Vector2 = Vector2(0, 0)
	var target_velocity:Vector2 = target.position - last_target_position
	last_target_position = target.position
	
	var adjusted_position:Vector2 = Vector2(0, 0)
	adjusted_position.x = horizontal_lead_scale * delta * target_velocity.x + target.position.x
	
	velocity.y = vertical_scale * (target.position.y - position.y)
	velocity.x = horizontal_scale * (adjusted_position.x - position.x)
	position += delta * velocity
