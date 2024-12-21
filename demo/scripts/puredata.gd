extends AudioStreamPlayer3D

var t = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	play()

func _process(delta):
	if fmod(t, 1.0) < delta:
		stream.send_bang("trigger")
		print("sent bang")
	
	t += delta
