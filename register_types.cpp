#include "register_types.h"
#include "core/class_db.h"
#include "core/engine.h"

#include "src/godot_fmod.h"
#include "src/event_player.h"
#include "src/event_listener.h"

void register_fmod_types() {
	ClassDB::register_class<FmodEventPlayer>();
	ClassDB::register_class<FmodEventListener>();
	ClassDB::register_class<FmodInterface>();
	memnew(FmodInterface);
	Engine::get_singleton()->add_singleton(Engine::Singleton("FmodInterface", FmodInterface::get_singleton()));
}

void unregister_fmod_types() {
	memdelete<FmodInterface>(FmodInterface::get_singleton());
}
