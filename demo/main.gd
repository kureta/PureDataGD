extends Node

var t = 0.0

# Called when the node enters the scene tree for the first time.
func _ready():
	$PureDataGD.play()
	$PureDataGD.dsp_on = true
	
	var p = ProjectSettings.globalize_path("res://patches/test-2.pd")
	$PureDataGD.patch_path = p
	$PureDataGD.freq = 440.0

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	t += delta
	$PureDataGD.freq = $Sprite2D.position.y + 440
