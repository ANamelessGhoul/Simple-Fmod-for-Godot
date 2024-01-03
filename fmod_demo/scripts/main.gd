extends Node

onready var volume_slider = $"%VolumeSlider"

func _ready():
	volume_slider.value = FmodInterface.get_bus_volume("bus:/")

func _on_VolumeSlider_value_changed(value):
	FmodInterface.set_bus_volume("bus:/", value)
