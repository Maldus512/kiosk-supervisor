#ifndef IOUTILS_H_INCLUDED
#define IOUTILS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool copy_file(const char *source_path, const char *dest_path);

bool file_exe(const char *path);

bool check_mount(const char *path);

bool check_device();

bool mount_device(const char *path);

bool umount_device(const char *path);

#endif
