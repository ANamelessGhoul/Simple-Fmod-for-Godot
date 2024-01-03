#ifndef GODOTFMOD_FILE_OPERATIONS_H
#define GODOTFMOD_FILE_OPERATIONS_H

#include "core/os/file_access.h"
#include "fmod_common.h"
#include "fmod_studio_common.h"

namespace FmodInterfaceFileOperations {

    FMOD_RESULT F_CALLBACK file_open(
        const char* name,
        unsigned int* filesize,
        void** handle,
        void* userdata);

    FMOD_RESULT F_CALLBACK file_close(
        void* handle,
        void* userdata);

    FMOD_RESULT F_CALLBACK file_read(
        void *handle,
        void *buffer,
        unsigned int sizebytes,
        unsigned int *bytesread,
        void *userdata);

	FMOD_RESULT F_CALLBACK file_seek(
        void *handle,
        unsigned int pos,
        void *userdata);

}// namespace FmodFileOperations

#endif// GODOTFMOD_FILE_OPERATIONS_H
