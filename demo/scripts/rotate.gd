extends CSGBox3D

var t = 0
var r = 10
var f = 0.125


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	position.x = r * sin(TAU * t * f)
	#position.z = r * cos(TAU * t * f)
	
	t += delta
