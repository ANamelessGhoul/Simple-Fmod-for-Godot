#include "helpers.h"

namespace FmodHelpers {
	
	FMOD_VECTOR to_fmod_vector(Vector3 vec) {
		return FMOD_VECTOR{vec.x, vec.y, vec.z};
	}
	
	FMOD_VECTOR to_fmod_vector(Vector2 vec) {
		return FMOD_VECTOR{vec.x, vec.y, 0};
	}
	
}