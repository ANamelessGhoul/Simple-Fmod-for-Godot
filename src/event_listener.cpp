#include "event_listener.h"
#include "core/os/input.h"
#include "helpers.h"
#include "godot_fmod.h"
#include "logging.h"


FmodEventListener::FmodEventListener(): 
        listener_index(-1),
        weight(1.0f) {
	set_process(true);
}

void FmodEventListener::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_weight", "weight"), &FmodEventListener::set_weight);
    ClassDB::bind_method(D_METHOD("get_weight"), &FmodEventListener::get_weight);

    ADD_PROPERTY(PropertyInfo(Variant::REAL, "weight", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_weight", "get_weight");
}

void FmodEventListener::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE:{
            FmodInterface::get_singleton()->register_listener(this);
        } break;

        case NOTIFICATION_EXIT_TREE:{
            FmodInterface::get_singleton()->unregister_listener(this);
        } break;

        case NOTIFICATION_PROCESS:{
            apply_attributes();
        } break;
    }
}

void FmodEventListener::set_listener_index(int p_listener_index) {
    listener_index = p_listener_index;
    if (listener_index < 0) {
        return;
    }
    apply_attributes();
    apply_weight();
}


void FmodEventListener::set_weight(float p_weight) {
    ERR_FAIL_COND_MSG(p_weight < 0 || p_weight > 1.0f, "Weight must be in range [0, 1].");

    weight = p_weight;
    apply_weight();
}


void FmodEventListener::apply_weight() {
    if (listener_index < 0) {
        return;
    }

    FmodInterface::get_singleton()->set_listener_weight(listener_index, weight);
}


void FmodEventListener::apply_attributes() {
    FMOD_3D_ATTRIBUTES attributes{};

    bool attributes_found = false;

    Node* parent = get_parent();
    if (!parent) {
        WARN_PRINT_ONCE("FmodEventListener must have a parent.");
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
        WARN_PRINT_ONCE("FmodEventListener parent must be Node2D or Spatial.");
        return;
    }

    FmodInterface::get_singleton()->set_listener_attributes(listener_index, attributes);
}

