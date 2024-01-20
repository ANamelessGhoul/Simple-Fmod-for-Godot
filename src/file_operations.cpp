#include "file_operations.h"

namespace FmodInterfaceFileOperations {

    FMOD_RESULT F_CALLBACK file_open(
            const char* name,
            unsigned int* filesize,
            void** handle,
            void* userdata) {
        Error error;
        FileAccess* file = FileAccess::open(name, FileAccess::READ, &error);

        if (error != Error::OK){
            return FMOD_RESULT::FMOD_ERR_FILE_BAD;
        }

        *filesize = file->get_len();
        *handle = reinterpret_cast<void*>(file);

        return FMOD_RESULT::FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK file_close(
            void* handle,
            void* userdata) {
        FileAccess* file {reinterpret_cast<FileAccess*>(handle)};
        file->close();
        memdelete(file);
        return FMOD_RESULT::FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK file_read(
            void *handle,
            void *buffer,
            unsigned int sizebytes,
            unsigned int *bytesread,
            void *userdata){
        

        // We get the Godot File object from the handle
        FileAccess* file {reinterpret_cast<FileAccess*>(handle)};

        // We read and store the requested data into the buffer.
        size_t bytes_read = file->get_buffer(reinterpret_cast<uint8_t*>(buffer), sizebytes);
        *bytesread = bytes_read;

        // Don't forget the return an error if end of the file is reached
        FMOD_RESULT result;
        if (file->eof_reached()) {
            result = FMOD_RESULT::FMOD_ERR_FILE_EOF;
        } else {
            result = FMOD_RESULT::FMOD_OK;
        }
        return result;
    }

    FMOD_RESULT F_CALLBACK file_seek(
            void *handle,
            unsigned int pos,
            void *userdata){
        FileAccess* file {reinterpret_cast<FileAccess*>(handle)};
        // update the position of the cursor
        file->seek(pos);
        return FMOD_RESULT::FMOD_OK;
    }

}// namespace Callbacks