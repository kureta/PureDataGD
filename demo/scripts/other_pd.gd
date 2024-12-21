extends AudioStreamPlayer3D

var f0 = 770
var lfo_f = 4
var lfo_amp = 100
var t = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	play()
	stream.send_float("fromGodot", 770)

func _process(delta):
	var f = f0 + lfo_amp * sin(TAU * t * lfo_f)
	stream.send_float("fromGodot", f)
	
	t += delta
