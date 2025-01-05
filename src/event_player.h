#ifndef FMOD_EVENT_SPAWNER_H
#define FMOD_EVENT_SPAWNER_H

#include "scene/main/node.h"

#include "fmod.hpp"
#include "fmod_errors.h"
#include "fmod_studio.hpp"

class FmodEventPlayer : public Node
{
	GDCLASS(FmodEventPlayer, Node);

public:
    FmodEventPlayer();

    void play_one_shot();

    void play();
    void stop(FMOD_STUDIO_STOP_MODE p_stop_mode);
    void stop_immediate();
    void stop_with_fadeout();

    void set_paused(bool p_paused);
    bool get_paused();


    FMOD_STUDIO_PLAYBACK_STATE get_playback_state();
    bool is_playing();
    bool has_valid_instance();

    float set_parameter(String p_parameter_name, float p_value);
    void clear_parameter(String p_parameter_name);

    void set_event_path(const String& p_event_path);
    String get_event_path() { return event_path; }

protected:
    static void _bind_methods();
	void _notification(int p_what);

	virtual void _validate_property(PropertyInfo &property) const;


private:
    Map<String, float> parameters;

    String event_path;
    FMOD::Studio::EventDescription *event_description = nullptr;
	FMOD::Studio::EventInstance *event_instance = nullptr;

    void load_event_description();
    FMOD::Studio::EventInstance* create_instance();
    void apply_parameters(FMOD::Studio::EventInstance *instance);

    enum PauseFlag {
        PAUSE_TREE   = 1 << 0,
        PAUSE_MANUAL = 1 << 1
    };

    int pause_flags;

    void set_pause_flag(PauseFlag p_flag, bool p_paused);
    void set_paused_impl(bool p_paused);
};


#endif // FMOD_EVENT_SPAWNER_H

