extends Node

var t = 0.0
var f = 0.0

# Called when the node enters the scene tree for the first time.
func _ready():
	$PureDataGD.play()
	$PureDataGD.dsp_on = true
	f = $PureDataGD.freq

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	t += delta
	#$PureDataGD.freq = $Sprite2D.position.y + f - 256
	$PureDataGD.send_float("fromGodot", $Sprite2D.position.y + f - 256)
