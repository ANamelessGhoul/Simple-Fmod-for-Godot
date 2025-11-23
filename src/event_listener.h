#ifndef FMOD_EVENT_LISTENER_H
#define FMOD_EVENT_LISTENER_H

#include "godot_fmod.h"
#include "scene/main/node.h"



#include "fmod.hpp"
#include "fmod_errors.h"
#include "fmod_studio.hpp"


class FmodEventListener : public Node
{
    GDCLASS(FmodEventListener, Node);
    
public:
    FmodEventListener();

protected:

    static void _bind_methods();
	void _notification(int p_what);

public:

    void set_listener_index(int p_listener_index);
    int get_listener_index() { return listener_index; }

    void set_weight(float p_weight);
    float get_weight() { return weight; }

private:

    void apply_weight();
    void apply_attributes();

    int listener_index;
    float weight;
};


#endif // FMOD_EVENT_LISTENER_H

