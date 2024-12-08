extends Sprite2D

var t = 0.0
var is_toggled = false

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if is_toggled:
		t += delta
	
	position.y = sin(t * TAU) * 32 + 256

func _input(event):
	if event.is_action_pressed("ui_accept"):
		is_toggled = !is_toggled
		print("Toggled:", is_toggled)
