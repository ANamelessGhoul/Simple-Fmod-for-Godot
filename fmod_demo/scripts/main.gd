extends Node

onready var volume_slider = $CanvasLayer/VBoxContainer/VolumeSlider
onready var play_button = $CanvasLayer/VBoxContainer/PlayButton

func _ready():
	volume_slider.value = FmodInterface.get_bus_volume("bus:/")
	play_button.grab_focus()

func _on_VolumeSlider_value_changed(value):
	FmodInterface.set_bus_volume("bus:/", value)
