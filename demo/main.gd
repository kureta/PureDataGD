extends Node3D

@onready var stream = $VariableFreq.stream
# Called when the node enters the scene tree for the first time.
func _ready():
	stream.send_float("fromGodot", 770)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	stream.send_float("fromGodot", 770)
