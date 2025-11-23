#include "event_player.h"
#include "core/os/input.h"
#include "godot_fmod.h"
#include "logging.h"

FmodEventPlayer::FmodEventPlayer(): pause_flags(0) {
    event_description = nullptr;
    event_instance = nullptr;
	set_process(true);
}

void FmodEventPlayer::play_one_shot() {
    FMOD_STUDIO_EVENTINSTANCE * instance = create_instance();
    
    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Start(instance));
    apply_parameters(instance);
    apply_attributes(instance);
    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Release(instance));
    LOG_VERBOSE(vformat("Playing One-Shot: %s", event_path));
}

void FmodEventPlayer::play() {
    if (event_path.empty()) {
        LOG_VERBOSE("No event_path set. Ignoring...");
        return;
    }

    if (is_valid_instance(event_instance)){
        if (is_playing()){
            FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Stop(event_instance, FMOD_STUDIO_STOP_IMMEDIATE));
        }
        FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Release(event_instance));
    }

    FMOD_STUDIO_EVENTINSTANCE *instance = create_instance();
    event_instance = instance;
    
    apply_parameters(instance);
    apply_attributes(instance);
    
    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Start(event_instance));
    set_paused_impl(get_paused());
}

void FmodEventPlayer::stop(FMOD_STUDIO_STOP_MODE p_stop_mode) {
    if (event_path.empty()) {
        LOG_VERBOSE("No event_path set. Ignoring...");
        return;
    }

    if (!is_valid_instance(event_instance)){
        LOG_VERBOSE("Can't call \"stop()\" because event instance is invalid. Cancelling...")
        return;
    }

    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Stop(event_instance, p_stop_mode));
    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Release(event_instance));
}

void FmodEventPlayer::stop_immediate() {
    stop(FMOD_STUDIO_STOP_IMMEDIATE);
}

void FmodEventPlayer::stop_with_fadeout() {
    stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}

void FmodEventPlayer::set_paused(bool p_paused) {
    if (!has_valid_instance()) {
        LOG_VERBOSE("Can't pause event because event instance is invalid. Ignoring...")
        return;
    }
    set_pause_flag(PAUSE_USER, p_paused);
}

bool FmodEventPlayer::get_paused() {
    return pause_flags > 0;
}

FMOD_STUDIO_PLAYBACK_STATE FmodEventPlayer::get_playback_state() {
    if (!is_valid_instance(event_instance)) {
        return FMOD_STUDIO_PLAYBACK_STOPPED;
    }

    FMOD_STUDIO_PLAYBACK_STATE state = FMOD_STUDIO_PLAYBACK_STOPPED;
    FMOD_ERR_COND_PRINT(FMOD_Studio_EventInstance_GetPlaybackState(event_instance, &state));
    return state;
}

bool FmodEventPlayer::is_playing() {
    return get_playback_state() != FMOD_STUDIO_PLAYBACK_STOPPED;
}

bool FmodEventPlayer::is_valid_instance(FMOD_STUDIO_EVENTINSTANCE *p_instance) {
    return p_instance != nullptr && FMOD_Studio_EventInstance_IsValid(p_instance);
}

float FmodEventPlayer::set_parameter(String p_parameter_name, float p_value) {
    if (event_description){
        FMOD_STUDIO_PARAMETER_DESCRIPTION parameter_description;
        FMOD_ERR_COND_PRINT(FMOD_Studio_EventDescription_GetParameterDescriptionByName(event_description, p_parameter_name.ascii().get_data(), &parameter_description));
        p_value = CLAMP(p_value, parameter_description.minimum, parameter_description.maximum);
    }

    parameters[p_parameter_name] = p_value; 
    if (is_valid_instance(event_instance)){
        apply_parameters(event_instance);
    }

    return p_value;
}

void FmodEventPlayer::clear_parameter(String p_parameter_name) {
    parameters.erase(p_parameter_name);
    if (is_valid_instance(event_instance)){
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
    if (p_what == NOTIFICATION_PAUSED || p_what == NOTIFICATION_UNPAUSED) {
        if (can_process()) {
            set_pause_flag(PAUSE_TREE, false);
        } else {
            set_pause_flag(PAUSE_TREE, true);
        }
    }

    switch (p_what)
    {
    case NOTIFICATION_ENTER_TREE:{
    } break;

    case NOTIFICATION_PROCESS:{
        if (is_valid_instance(event_instance)) {

            apply_attributes(event_instance);

            // NOTE: Make sure you do this last, as we won't have an instance after this
            if (get_playback_state() == FMOD_STUDIO_PLAYBACK_STOPPED){
                FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Release(event_instance));
            }
        }
    } break;

    case NOTIFICATION_PREDELETE:{
        if (has_valid_instance()){
            if (is_playing()){
                stop_immediate();
            }
            FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Release(event_instance));
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

FMOD_STUDIO_EVENTINSTANCE *FmodEventPlayer::create_instance() {
    if (event_description == nullptr){
        load_event_description();
    }

    FMOD_STUDIO_EVENTINSTANCE *instance;
    FMOD_ERR_COND_PRINT(FMOD_Studio_EventDescription_CreateInstance(event_description, &instance));
    return instance;
}

void FmodEventPlayer::apply_parameters(FMOD_STUDIO_EVENTINSTANCE *p_instance) {
    int count;
    FMOD_ERR_COND_FAIL(FMOD_Studio_EventDescription_GetParameterDescriptionCount(event_description, &count));
    for(int i = 0; i < count; i++){
        FMOD_STUDIO_PARAMETER_DESCRIPTION description;
        FMOD_ERR_COND_FAIL(FMOD_Studio_EventDescription_GetParameterDescriptionByIndex(event_description, i, &description));

        String parameter_name = String(description.name);
        if (parameters.has(parameter_name)){
            float parameter_value = parameters[parameter_name];
            FMOD_Studio_EventInstance_SetParameterByID(p_instance, description.id, parameter_value, false);
            LOG_VERBOSE(vformat("Set parameter \"%s\" in event \"%s\" to \"%f\"", parameter_name, event_path, parameter_value))
        }

    }
}

void FmodEventPlayer::apply_attributes(FMOD_STUDIO_EVENTINSTANCE *p_instance) {
    DEV_ASSERT(is_valid_instance(p_instance));

    FMOD_3D_ATTRIBUTES attributes{};

    bool attributes_found = false;

    Node* parent = get_parent();
    if (!parent) {
        WARN_PRINT_ONCE("FmodEventPlayer must have a parent.");
        return;
    }

    Node2D* parent_2d = Object::cast_to<Node2D>(parent);
    if (parent_2d) {
        attributes = FmodInterface::get_attributes_2d(parent_2d);
        attributes_found = true;
    }

    Spatial* parent_spatial = Object::cast_to<Spatial>(parent);
    if (parent_spatial) {
        attributes = FmodInterface::get_attributes_3d(parent_spatial);
        attributes_found = true;
    }
    
    if (!attributes_found) {
        WARN_PRINT_ONCE("FmodEventPlayer parent must be Node2D or Spatial.");
        return;
    }


    FMOD_ERR_COND_FAIL(FMOD_Studio_EventInstance_Set3DAttributes(p_instance, &attributes));
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
    if (event_path.empty()) {
        LOG_VERBOSE("No event_path set, can't set pause. Ignoring...");
        return;
    }

    if (is_valid_instance(event_instance))
    {
        FMOD_ERR_COND_PRINT(FMOD_Studio_EventInstance_SetPaused(event_instance, p_paused));
    }
}
