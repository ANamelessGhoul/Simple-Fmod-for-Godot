#include "event_player.h"
#include "core/os/input.h"
#include "godot_fmod.h"
#include "logging.h"

FmodEventPlayer::FmodEventPlayer(): pause_flags(0) {
    event_description = nullptr;
    event_instance = nullptr;
}

void FmodEventPlayer::play_one_shot() {
    FMOD::Studio::EventInstance * instance = create_instance();
    
    FMOD_ERR_COND_FAIL(instance->start());
    apply_parameters(instance);
    FMOD_ERR_COND_FAIL(instance->release());
    LOG_VERBOSE(vformat("Playing One-Shot: %s", event_path));
}

void FmodEventPlayer::play() {
    if (has_valid_instance()){
        if (is_playing()){
            FMOD_ERR_COND_FAIL(event_instance->stop(FMOD_STUDIO_STOP_IMMEDIATE));
        }
        FMOD_ERR_COND_FAIL(event_instance->release());
    }

    FMOD::Studio::EventInstance *instance = create_instance();
    apply_parameters(instance);
    event_instance = instance;
    FMOD_ERR_COND_FAIL(event_instance->start());
}

void FmodEventPlayer::stop(FMOD_STUDIO_STOP_MODE p_stop_mode) {
    if (!has_valid_instance()){
        LOG_VERBOSE("Can't call \"stop()\" because event instance is invalid. Cancelling...")
        return;
    }

    FMOD_ERR_COND_FAIL(event_instance->stop(p_stop_mode));
    FMOD_ERR_COND_FAIL(event_instance->release());
}

void FmodEventPlayer::stop_immediate() {
    stop(FMOD_STUDIO_STOP_IMMEDIATE);
}

void FmodEventPlayer::stop_with_fadeout() {
    stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}

void FmodEventPlayer::set_paused(bool p_paused) {
    if (!has_valid_instance()){
        LOG_VERBOSE("Can't pause event because event instance is invalid. Ignoring...")
        return;
    }
    set_pause_flag(PAUSE_MANUAL, p_paused);
}

bool FmodEventPlayer::get_paused() {
    if (!has_valid_instance()){
        LOG_VERBOSE("Can't get pause state of event because event instance is invalid. Returning false.")
        return false;
    }

    bool paused = false;
    FMOD_ERR_COND_PRINT(event_instance->getPaused(&paused));
	return paused;
}

FMOD_STUDIO_PLAYBACK_STATE FmodEventPlayer::get_playback_state() {
    if (!has_valid_instance()) {
        return FMOD_STUDIO_PLAYBACK_STOPPED;
    }

    FMOD_STUDIO_PLAYBACK_STATE state = FMOD_STUDIO_PLAYBACK_STOPPED;
    FMOD_ERR_COND_PRINT(event_instance->getPlaybackState(&state));
    return state;
}

bool FmodEventPlayer::is_playing() {
    return get_playback_state() != FMOD_STUDIO_PLAYBACK_STOPPED;
}

bool FmodEventPlayer::has_valid_instance() {
    return event_instance != nullptr && event_instance->isValid();
}

float FmodEventPlayer::set_parameter(String p_parameter_name, float p_value) {
    if (event_description){
        FMOD_STUDIO_PARAMETER_DESCRIPTION parameter_description;
        FMOD_ERR_COND_PRINT(event_description->getParameterDescriptionByName(p_parameter_name.ascii().get_data(), &parameter_description));
        p_value = CLAMP(p_value, parameter_description.minimum, parameter_description.maximum);
    }

    parameters[p_parameter_name] = p_value; 
    if (has_valid_instance()){
        apply_parameters(event_instance);
    }

    return p_value;
}

void FmodEventPlayer::clear_parameter(String p_parameter_name) {
    parameters.erase(p_parameter_name);
    if (has_valid_instance()){
        apply_parameters(event_instance);
    }
}

void FmodEventPlayer::set_event_path(const String &p_event_path) {
    event_path = p_event_path;
    event_description = nullptr;
}

void FmodEventPlayer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_event_path", "event_path"), &FmodEventPlayer::set_event_path);
    ClassDB::bind_method(D_METHOD("get_event_path"), &FmodEventPlayer::get_event_path);

    ClassDB::bind_method(D_METHOD("play_one_shot"), &FmodEventPlayer::play_one_shot);

    ClassDB::bind_method(D_METHOD("play"), &FmodEventPlayer::play);
    ClassDB::bind_method(D_METHOD("stop_immediate"), &FmodEventPlayer::stop_immediate);
    ClassDB::bind_method(D_METHOD("stop_with_fadeout"), &FmodEventPlayer::stop_with_fadeout);

    ClassDB::bind_method(D_METHOD("set_stream_paused"), &FmodEventPlayer::set_paused);
    ClassDB::bind_method(D_METHOD("get_stream_paused"), &FmodEventPlayer::get_paused);

    ClassDB::bind_method(D_METHOD("is_playing"), &FmodEventPlayer::is_playing);

    ClassDB::bind_method(D_METHOD("set_parameter", "parameter_name", "value"), &FmodEventPlayer::set_parameter);
    ClassDB::bind_method(D_METHOD("clear_parameter", "parameter_name"), &FmodEventPlayer::clear_parameter);


    ADD_PROPERTY(PropertyInfo(Variant::STRING, "event_path", PROPERTY_HINT_ENUM, "load_more_banks,"), "set_event_path", "get_event_path");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stream_paused"), "set_stream_paused", "get_stream_paused");
}

void FmodEventPlayer::_notification(int p_what) {
    switch (p_what)
    {
    case NOTIFICATION_ENTER_TREE:{
    } break;

    case NOTIFICATION_PROCESS:{
        if (has_valid_instance() && get_playback_state() == FMOD_STUDIO_PLAYBACK_STOPPED){
            FMOD_ERR_COND_FAIL(event_instance->release());
        }
    } break;

    case NOTIFICATION_PAUSED:{
        if (has_valid_instance()){
            set_pause_flag(PAUSE_TREE, true);
        }
    } break;

    case NOTIFICATION_UNPAUSED:{
        if (has_valid_instance()){
            set_pause_flag(PAUSE_TREE, false);
        }
    } break;

    case NOTIFICATION_PREDELETE:{
        if (has_valid_instance()){
            if (is_playing()){
                stop_immediate();
            }
            FMOD_ERR_COND_FAIL(event_instance->release());
        }
    } break;

    default:
        break;
    }
}

void FmodEventPlayer::_validate_property(PropertyInfo &property) const {
    if (property.name == "event_path")
    {
        property.hint_string = FmodInterface::get_singleton()->get_events_property_hint();
    }
}

void FmodEventPlayer::load_event_description() {
    event_description = FmodInterface::get_singleton()->get_event_description(event_path);
}

FMOD::Studio::EventInstance *FmodEventPlayer::create_instance() {
    if (event_description == nullptr){
        load_event_description();
    }

    FMOD::Studio::EventInstance *instance;
    FMOD_ERR_COND_PRINT(event_description->createInstance(&instance));
    return instance;
}

void FmodEventPlayer::apply_parameters(FMOD::Studio::EventInstance *instance) {
    int count;
    FMOD_ERR_COND_FAIL(event_description->getParameterDescriptionCount(&count));
    for(int i = 0; i < count; i++){
        FMOD_STUDIO_PARAMETER_DESCRIPTION description;
        FMOD_ERR_COND_FAIL(event_description->getParameterDescriptionByIndex(i, &description));

        String parameter_name = String(description.name);
        if (parameters.has(parameter_name)){
            float parameter_value = parameters[parameter_name];
            instance->setParameterByID(description.id, parameter_value);
            LOG_VERBOSE(vformat("Set parameter \"%s\" in event \"%s\" to \"%f\"", parameter_name, event_path, parameter_value))
        }

    }
}

void FmodEventPlayer::set_pause_flag(PauseFlag p_flag, bool p_paused) {
    if (p_paused)
    {
        pause_flags |= p_flag;
    }
    else
    {
        pause_flags &= ~p_flag;
    }

    set_paused_impl(pause_flags > 0);
}

void FmodEventPlayer::set_paused_impl(bool p_paused) {
    FMOD_ERR_COND_PRINT(event_instance->setPaused(p_paused));
}
