#include "godot_fmod.h"
#include "logging.h"
#include "helpers.h"

#include "core/vector.h"
#include "core/engine.h"
#include "core/project_settings.h"
#include "core/os/os.h"
#include "main/main.h"


#define GLOBAL_DEF_INFO(m_name, m_default_value, m_type, m_hint, m_hint_string) \
	GLOBAL_DEF(m_name, m_default_value);									\
	ProjectSettings::get_singleton()->set_custom_property_info(m_name, PropertyInfo(m_type, m_name, m_hint, m_hint_string, PROPERTY_USAGE_DEFAULT));

#define GLOBAL_DEF_INFO_RST(m_name, m_default_value, m_type, m_hint, m_hint_string) \
	GLOBAL_DEF_RST(m_name, m_default_value);									\
	ProjectSettings::get_singleton()->set_custom_property_info(m_name, PropertyInfo(m_type, m_name, m_hint, m_hint_string, PROPERTY_USAGE_DEFAULT));



FmodInterface* FmodInterface::singleton = nullptr;

FmodInterface::FmodInterface() {
    if (singleton == nullptr){
        singleton = this;
    }
    else{
        ERR_FAIL_MSG("FmodInterface singleton already exists.");
    }

	GLOBAL_DEF_RST("fmod/initialization/auto_initialize", true);
	GLOBAL_DEF_INFO("fmod/initialization/number_of_channels", 1024, Variant::INT, PROPERTY_HINT_RANGE, "0,4095,1");
	GLOBAL_DEF("fmod/initialization/live_update", false);
	GLOBAL_DEF_INFO("fmod/initialization/sample_rate", 48000, Variant::INT, PROPERTY_HINT_RANGE, "8000,192000,1000");
	GLOBAL_DEF_INFO("fmod/initialization/speaker_mode", FMOD_SPEAKERMODE::FMOD_SPEAKERMODE_DEFAULT, Variant::INT, PROPERTY_HINT_ENUM, "Default,Raw,Mono,Stereo,Quad,Surround,5.1,7.1,7.1.4");
	GLOBAL_DEF_INFO("fmod/initialization/num_of_raw_speakers", 32, Variant::INT, PROPERTY_HINT_RANGE, "0,32,1");
	GLOBAL_DEF("fmod/initialization/dsp_buffer_length", 1024);
	GLOBAL_DEF("fmod/initialization/dsp_buffer_count", 4);

	GLOBAL_DEF_RST("fmod/banks/auto_load_banks", true);
	GLOBAL_DEF_INFO_RST("fmod/banks/bank_paths", Vector<String>(), Variant::ARRAY, PROPERTY_HINT_TYPE_STRING, "4/15:");


	if (!Main::is_project_manager() && GLOBAL_GET("fmod/initialization/auto_initialize")){
		call_deferred("_bind_to_game_loop");
		init_from_settings(FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL);
	}

	if (GLOBAL_GET("fmod/banks/auto_load_banks")){
		load_banks_from_settings();
	}
}

FmodInterface::~FmodInterface() {
    if (is_initialized()){
        shutdown();
    }

	if (singleton == this){
	    singleton = nullptr;
	}
}

void FmodInterface::_bind_methods() {
    // Status
	ClassDB::bind_method(D_METHOD("init", "num_of_channels", "studio_flags", "flags"), &FmodInterface::init);
	ClassDB::bind_method(D_METHOD("is_initialized"), &FmodInterface::is_initialized);
	ClassDB::bind_method(D_METHOD("update"), &FmodInterface::update);
	ClassDB::bind_method(D_METHOD("shutdown"), &FmodInterface::shutdown);

    // Banks
	ClassDB::bind_method(D_METHOD("load_bank", "path", "flags"), &FmodInterface::load_bank);
	ClassDB::bind_method(D_METHOD("has_bank", "path"), &FmodInterface::has_bank);

    // Bus
	ClassDB::bind_method(D_METHOD("set_bus_volume", "path", "volume"), &FmodInterface::set_bus_volume);
	ClassDB::bind_method(D_METHOD("get_bus_volume", "path"), &FmodInterface::get_bus_volume);

    // Bank data
    ClassDB::bind_method(D_METHOD("get_loaded_event_paths"), &FmodInterface::get_loaded_event_paths);
    ClassDB::bind_method(D_METHOD("get_loaded_bus_paths"), &FmodInterface::get_loaded_bus_paths);

    ClassDB::bind_method(D_METHOD("_bind_to_game_loop"), &FmodInterface::bind_to_game_loop);

	/* FMOD_INITFLAGS */
	BIND_CONSTANT(FMOD_INIT_NORMAL);
	BIND_CONSTANT(FMOD_INIT_STREAM_FROM_UPDATE);
	BIND_CONSTANT(FMOD_INIT_MIX_FROM_UPDATE);
	BIND_CONSTANT(FMOD_INIT_3D_RIGHTHANDED);
	BIND_CONSTANT(FMOD_INIT_CHANNEL_LOWPASS);
	BIND_CONSTANT(FMOD_INIT_CHANNEL_DISTANCEFILTER);
	BIND_CONSTANT(FMOD_INIT_PROFILE_ENABLE);
	BIND_CONSTANT(FMOD_INIT_VOL0_BECOMES_VIRTUAL);
	BIND_CONSTANT(FMOD_INIT_GEOMETRY_USECLOSEST);
	BIND_CONSTANT(FMOD_INIT_PREFER_DOLBY_DOWNMIX);
	BIND_CONSTANT(FMOD_INIT_THREAD_UNSAFE);
	BIND_CONSTANT(FMOD_INIT_PROFILE_METER_ALL);

	/* FMOD_STUDIO_INITFLAGS */
	BIND_CONSTANT(FMOD_STUDIO_INIT_NORMAL);
	BIND_CONSTANT(FMOD_STUDIO_INIT_LIVEUPDATE);
	BIND_CONSTANT(FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS);
	BIND_CONSTANT(FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE);
	BIND_CONSTANT(FMOD_STUDIO_INIT_DEFERRED_CALLBACKS);
	BIND_CONSTANT(FMOD_STUDIO_INIT_LOAD_FROM_UPDATE);

	/* FMOD_STUDIO_LOAD_BANK_FLAGS */
	BIND_CONSTANT(FMOD_STUDIO_LOAD_BANK_NORMAL);
	BIND_CONSTANT(FMOD_STUDIO_LOAD_BANK_NONBLOCKING);
	BIND_CONSTANT(FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES);

	/* FMOD_STUDIO_LOADING_STATE */
	BIND_CONSTANT(FMOD_STUDIO_LOADING_STATE_UNLOADING);
	BIND_CONSTANT(FMOD_STUDIO_LOADING_STATE_LOADING);
	BIND_CONSTANT(FMOD_STUDIO_LOADING_STATE_LOADED);
	BIND_CONSTANT(FMOD_STUDIO_LOADING_STATE_ERROR);
	BIND_CONSTANT(FMOD_STUDIO_LOADING_STATE_UNLOADED);

	/* FMOD_STUDIO_PLAYBACK_STATE */
	BIND_CONSTANT(FMOD_STUDIO_PLAYBACK_PLAYING);
	BIND_CONSTANT(FMOD_STUDIO_PLAYBACK_SUSTAINING);
	BIND_CONSTANT(FMOD_STUDIO_PLAYBACK_STOPPED);
	BIND_CONSTANT(FMOD_STUDIO_PLAYBACK_STARTING);
	BIND_CONSTANT(FMOD_STUDIO_PLAYBACK_STOPPING);

	/* FMOD_STUDIO_STOP_MODE */
	BIND_CONSTANT(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	BIND_CONSTANT(FMOD_STUDIO_STOP_IMMEDIATE);

	/* FMOD_STUDIO_EVENT_CALLBACK_TYPE */
	BIND_CONSTANT(FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER);
	BIND_CONSTANT(FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT);
	BIND_CONSTANT(FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED);
	BIND_CONSTANT(FMOD_STUDIO_EVENT_CALLBACK_SOUND_STOPPED);

	/* FMOD_SPEAKERMODE */
	BIND_CONSTANT(FMOD_SPEAKERMODE_DEFAULT);
	BIND_CONSTANT(FMOD_SPEAKERMODE_RAW);
	BIND_CONSTANT(FMOD_SPEAKERMODE_MONO);
	BIND_CONSTANT(FMOD_SPEAKERMODE_STEREO);
	BIND_CONSTANT(FMOD_SPEAKERMODE_QUAD);
	BIND_CONSTANT(FMOD_SPEAKERMODE_SURROUND);
	BIND_CONSTANT(FMOD_SPEAKERMODE_5POINT1);
	BIND_CONSTANT(FMOD_SPEAKERMODE_7POINT1);
	BIND_CONSTANT(FMOD_SPEAKERMODE_7POINT1POINT4);
	BIND_CONSTANT(FMOD_SPEAKERMODE_MAX);

	/* FMOD_MODE */
	BIND_CONSTANT(FMOD_DEFAULT);
	BIND_CONSTANT(FMOD_LOOP_OFF);
	BIND_CONSTANT(FMOD_LOOP_NORMAL);
	BIND_CONSTANT(FMOD_LOOP_BIDI);
	BIND_CONSTANT(FMOD_2D);
	BIND_CONSTANT(FMOD_3D);
	BIND_CONSTANT(FMOD_CREATESTREAM);
	BIND_CONSTANT(FMOD_CREATESAMPLE);
	BIND_CONSTANT(FMOD_CREATECOMPRESSEDSAMPLE);
	BIND_CONSTANT(FMOD_OPENUSER);
	BIND_CONSTANT(FMOD_OPENMEMORY);
	BIND_CONSTANT(FMOD_OPENMEMORY_POINT);
	BIND_CONSTANT(FMOD_OPENRAW);
	BIND_CONSTANT(FMOD_OPENONLY);
	BIND_CONSTANT(FMOD_ACCURATETIME);
	BIND_CONSTANT(FMOD_MPEGSEARCH);
	BIND_CONSTANT(FMOD_NONBLOCKING);
	BIND_CONSTANT(FMOD_UNIQUE);
	BIND_CONSTANT(FMOD_3D_HEADRELATIVE);
	BIND_CONSTANT(FMOD_3D_WORLDRELATIVE);
	BIND_CONSTANT(FMOD_3D_INVERSEROLLOFF);
	BIND_CONSTANT(FMOD_3D_LINEARROLLOFF);
	BIND_CONSTANT(FMOD_3D_LINEARSQUAREROLLOFF);
	BIND_CONSTANT(FMOD_3D_INVERSETAPEREDROLLOFF);
	BIND_CONSTANT(FMOD_3D_CUSTOMROLLOFF);
	BIND_CONSTANT(FMOD_3D_IGNOREGEOMETRY);
	BIND_CONSTANT(FMOD_IGNORETAGS);
	BIND_CONSTANT(FMOD_LOWMEM);
	BIND_CONSTANT(FMOD_VIRTUAL_PLAYFROMSTART);

}

void FmodInterface::init(int p_num_of_channels, FMOD_STUDIO_INITFLAGS p_studio_flags, FMOD_INITFLAGS p_core_flags) {
	// initialize FMOD Studio and FMOD Core System with provided flags
    FMOD_ERR_COND_FAIL(FMOD::Studio::System::create(&studio_system));
	FMOD_ERR_COND_FAIL(studio_system->getCoreSystem(&core_system));
    FMOD_ERR_COND_FAIL(studio_system->initialize(p_num_of_channels, p_studio_flags, p_core_flags, nullptr));


    LOG_VERBOSE("Successfully initialized.");
    if (p_studio_flags & FMOD_STUDIO_INIT_LIVEUPDATE){
        LOG_VERBOSE("Live update enabled!");
    }

    FMOD_ERR_COND_FAIL(core_system->setFileSystem(
			&FmodInterfaceFileOperations::file_open,
			&FmodInterfaceFileOperations::file_close,
			&FmodInterfaceFileOperations::file_read,
			&FmodInterfaceFileOperations::file_seek,
			nullptr,
			nullptr,
			-1));
	LOG_VERBOSE("Custom File System enabled.");
}

void FmodInterface::init_from_settings(FMOD_STUDIO_INITFLAGS p_studio_flags, FMOD_INITFLAGS p_core_flags) {
	// initialize FMOD Studio and FMOD Core System with provided flags
    FMOD_ERR_COND_FAIL(FMOD::Studio::System::create(&studio_system));
	FMOD_ERR_COND_FAIL(studio_system->getCoreSystem(&core_system));

	int dsp_buffer_length = GLOBAL_GET("fmod/initialization/dsp_buffer_length");
	int dsp_buffer_count = GLOBAL_GET("fmod/initialization/dsp_buffer_count");
	core_system->setDSPBufferSize(dsp_buffer_length, dsp_buffer_count);

	int sample_rate = GLOBAL_GET("fmod/initialization/sample_rate");
	int speaker_mode = GLOBAL_GET("fmod/initialization/speaker_mode");
	int num_of_raw_speakers = GLOBAL_GET("fmod/initialization/num_of_raw_speakers");
	core_system->setSoftwareFormat(sample_rate, static_cast<FMOD_SPEAKERMODE>(speaker_mode), num_of_raw_speakers);

	int num_of_channels = GLOBAL_GET("fmod/initialization/number_of_channels");
	if (GLOBAL_GET("fmod/initialization/live_update")){
		p_studio_flags |= FMOD_STUDIO_INIT_LIVEUPDATE;
	}
    FMOD_ERR_COND_FAIL(studio_system->initialize(num_of_channels, p_studio_flags, p_core_flags, nullptr));


    LOG_VERBOSE("Successfully initialized.");
    if (p_studio_flags & FMOD_STUDIO_INIT_LIVEUPDATE){
        LOG_VERBOSE("Live update enabled!");
    }

    FMOD_ERR_COND_FAIL(core_system->setFileSystem(
			&FmodInterfaceFileOperations::file_open,
			&FmodInterfaceFileOperations::file_close,
			&FmodInterfaceFileOperations::file_read,
			&FmodInterfaceFileOperations::file_seek,
			nullptr,
			nullptr,
			-1));
	LOG_VERBOSE("Custom File System enabled.");
}

bool FmodInterface::is_initialized() {
	return studio_system != nullptr && studio_system->isValid();
}

void FmodInterface::update() {
	FMOD_INIT_CHECK_ONCE();

    studio_system->update();
}

void FmodInterface::shutdown() {
	FMOD_INIT_CHECK();

    FMOD_ERR_COND_PRINT(studio_system->unloadAll());
	FMOD_ERR_COND_FAIL(studio_system->release());
    
    LOG_VERBOSE("Successfully released.");
}

void FmodInterface::load_banks_from_settings(){
	FMOD_INIT_CHECK();
	Vector<String> bank_paths = GLOBAL_GET("fmod/banks/bank_paths");
	for (int i = 0; i < bank_paths.size(); i++){
		load_bank(bank_paths[i], FMOD_STUDIO_LOAD_BANK_NORMAL);
	}
}

void FmodInterface::load_bank(const String& p_filepath, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
	FMOD_INIT_CHECK();

    FMOD::Studio::Bank *bank;
    FMOD_ERR_COND_FAIL_MSG(studio_system->loadBankFile(p_filepath.ascii().get_data(), flags, &bank)\
    , vformat("Could not load bank: %s", p_filepath));

    filepath_bank_map.insert(p_filepath, bank);
    add_event_paths(bank);
    add_bus_paths(bank);

    LOG_VERBOSE(vformat("Loaded Bank: %s", p_filepath));
}

bool FmodInterface::has_bank(const String& p_filepath) {
	FMOD_INIT_CHECK_V(false);

    return filepath_bank_map.has(p_filepath);
}

void FmodInterface::set_bus_volume(const String & p_bus_path, float p_volume){
	FMOD_INIT_CHECK();

    FMOD::Studio::Bus* bus;
    FMOD_ERR_COND_FAIL(studio_system->getBus(p_bus_path.ascii().get_data(), &bus));
    FMOD_ERR_COND_FAIL(bus->setVolume(p_volume));
    LOG_VERBOSE(vformat("Set volume of bus \"%s\" to \"%.2f\"", p_bus_path, p_volume));
}

float FmodInterface::get_bus_volume(const String &p_bus_path) {
	FMOD_INIT_CHECK_V(-1.f);

	FMOD::Studio::Bus* bus;
    FMOD_ERR_COND_FAIL_V(studio_system->getBus(p_bus_path.ascii().get_data(), &bus), 0.0f);
    float volume = 0.0f;
    FMOD_ERR_COND_FAIL_V(bus->getVolume(&volume), 0.0f);
    return volume;
}

FMOD::Studio::EventDescription *FmodInterface::get_event_description(String p_event_path) {
	FMOD_INIT_CHECK_V(nullptr);
	FMOD::Studio::EventDescription *event_description = nullptr;
    FMOD_ERR_COND_FAIL_V(studio_system->getEvent(p_event_path.ascii().get_data(), &event_description), nullptr);
    LOG_VERBOSE(vformat("Got event description for path: %s", p_event_path))
    return event_description;
}

void FmodInterface::reload_bank_metadata() {
    event_paths.clear();

    int capacity;
    studio_system->getBankCount(&capacity);
    FMOD::Studio::Bank** banks = new FMOD::Studio::Bank*[capacity];
    int count;
    studio_system->getBankList(banks, capacity, &count);

    for (int i = 0; i < count; i++)
    {
        FMOD::Studio::Bank* bank = banks[i];
        add_event_paths(bank);
        add_bus_paths(bank);
    }

    delete[] banks;
}

void FmodInterface::add_event_paths(FMOD::Studio::Bank *p_bank) {
	int event_capacity;
    FMOD_ERR_COND_PRINT(p_bank->getEventCount(&event_capacity));
    FMOD::Studio::EventDescription** events = new FMOD::Studio::EventDescription*[event_capacity];
    int event_count;
    FMOD_ERR_COND_PRINT(p_bank->getEventList(events, event_capacity, &event_count));

    for (int j = 0; j < event_count; j++)
    {
        char path[256];
        int retrieved;
        events[j]->getPath(path, 256, &retrieved);
        event_property_hint = event_property_hint + "," + path;
        event_paths.push_back(path);
    }

    delete[] events;
}

void FmodInterface::add_bus_paths(FMOD::Studio::Bank *p_bank) {
	int bus_capacity;
    FMOD_ERR_COND_PRINT(p_bank->getBusCount(&bus_capacity));
    FMOD::Studio::Bus** busses = new FMOD::Studio::Bus*[bus_capacity];
    int bus_count;
    FMOD_ERR_COND_PRINT(p_bank->getBusList(busses, bus_capacity, &bus_count));

    for (int j = 0; j < bus_count; j++)
    {
        char path[256];
        int retrieved;
        busses[j]->getPath(path, 256, &retrieved);
        bus_paths.push_back(path);
    }

    delete[] busses;
}

void FmodInterface::bind_to_game_loop() {
	SceneTree *tree = SceneTree::get_singleton();
	if (!tree){
		ERR_FAIL_MSG("Main Loop is not a scene tree!");
	}

	if (tree->connect("idle_frame", this, "update") != OK){
		ERR_FAIL_MSG("Could not connect to scene tree, idle_frame signal!");
	}

	LOG_VERBOSE("Connected to Scene Tree idle_frame.")
}
