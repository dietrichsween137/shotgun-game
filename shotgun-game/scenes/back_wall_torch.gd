extends Node2D

@export var Base_Energy:float = 5.0
@export var Energy_Scaling:float = 0.2
@export var Base_Radius:int = 256
@export var Radius_Scaling:int = 1

@onready var _animation_player = $AnimationPlayer
@onready var _point_light2d = $PointLight2D
@onready var _sprite2d = $Sprite2D

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var start_frame:int = randi() % 4
	_animation_player.play("burn")
	_animation_player.seek(0.1 * start_frame)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	var scalar:int = abs(_sprite2d.frame - 2)
	_point_light2d.energy = Base_Energy - Energy_Scaling * scalar
	
	var current_radius: int = Base_Radius - Radius_Scaling * scalar
	_point_light2d.texture.width = current_radius
	_point_light2d.texture.height = current_radius
