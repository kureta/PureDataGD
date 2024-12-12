extends Node

var t = 0.0
var f = 0.0

# Called when the node enters the scene tree for the first time.
func _ready():
	f = $PureDataGD.freq
	
	var patch_file = FileAccess.open($PureDataGD.patch_path, FileAccess.READ)
	var tmp_file = FileAccess.open('/tmp/{0}.pd'.format([randi()]), FileAccess.WRITE)
	var contents = patch_file.get_buffer(patch_file.get_length())
	patch_file.close()
	tmp_file.store_buffer(contents)
	tmp_file.close()
	
	$PureDataGD.patch_path = tmp_file.get_path_absolute()
	
	$PureDataGD.dsp_on = true
	$PureDataGD.play()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	t += delta
	$PureDataGD.send_float("fromGodot", $Sprite2D.position.y + f - 256)
