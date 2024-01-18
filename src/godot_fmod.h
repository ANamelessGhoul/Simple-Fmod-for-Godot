#ifndef GODOT_FMOD_H
#define GODOT_FMOD_H

#include "scene/main/node.h"
#include "core/object.h"
#include "core/ustring.h"
#include "core/error_macros.h"

#include "fmod.hpp"
#include "fmod_errors.h"
#include "fmod_studio.hpp"

#include "file_operations.h"

#include "core/map.h"



class FmodInterface : public Object
{
public:
	
	FmodInterface();
	~FmodInterface();
	
private:
	GDCLASS(FmodInterface, Object);

	static FmodInterface *singleton;

protected:
	static void _bind_methods();

public:
	static FmodInterface *get_singleton() { return singleton; }


	// Status
	void init(int p_num_of_channels, FMOD_STUDIO_INITFLAGS p_studio_flags, FMOD_INITFLAGS p_core_flags);
	void init_from_settings(FMOD_STUDIO_INITFLAGS p_studio_flags, FMOD_INITFLAGS p_core_flags);
	bool is_initialized();
	void update();
	void shutdown();

	// Banks
	void load_banks_from_settings();
	void load_bank(const String& p_filepath, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
	bool has_bank(const String& p_path);

	// Busses
	void set_bus_volume(const String& p_bus_path, float p_volume);
	float get_bus_volume(const String& p_bus_path);


	// Event Data
	Vector<String> get_loaded_event_paths() { return event_paths; }
	String get_events_property_hint() { return event_property_hint; }

	// Bus Data
	Vector<String> get_loaded_bus_paths() { return bus_paths; }

	FMOD::Studio::EventDescription *get_event_description(String p_event_path);

private:
	
	FMOD::Studio::System *studio_system;
	FMOD::System *core_system;

	Map<String, FMOD::Studio::Bank*> filepath_bank_map;

	Vector<String> event_paths;
	String event_property_hint;

	Vector<String> bus_paths;


	void reload_bank_metadata();
	void add_event_paths(FMOD::Studio::Bank* p_bank);
	void add_bus_paths(FMOD::Studio::Bank* p_bank);

	void bind_to_game_loop();

};

#endif // GODOT_FMOD_H



