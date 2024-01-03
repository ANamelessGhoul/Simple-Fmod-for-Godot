#include "helpers.h"

FMOD_VECTOR to_fmod_vector(Vector3 vec) {
	return FMOD_VECTOR{vec.x, vec.y, vec.z};
}