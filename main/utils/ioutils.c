#include "ioutils.h"
#include "log.h"
#include <signal.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

bool copy_file(const char *source_path, const char *dest_path) {

  FILE *read_f, *write_f;
  int read_fd, write_fd;
  struct stat stat_buf;
  off_t offset = 0;

  /* Open the input file. */
  read_f = fopen(source_path, "r");
  read_fd = fileno(read_f);
  if (read_fd == -1) {
    log_info("non riesco a leggere dal file di log %s", source_path);
    return false;
  }
  /* Stat the input file to obtain its size. */
  fstat(read_fd, &stat_buf);
  /* Open the output file for writing, with the same permissions as the
    source file. */
  write_f = fopen(dest_path, "w");
  write_fd = fileno(write_f);
  if (write_fd == -1) {
    log_info("non riesco a scrivere nel file di log %s", dest_path);
    return false;
  }
  /* Blast the bytes from one file to the other. */
  sendfile(write_fd, read_fd, &offset, stat_buf.st_size);
  /* Close up. */

  fclose(read_f);
  fclose(write_f);

  return true;
}

bool file_exe(const char *path) {
  struct stat s;
  return (stat(path, &s) == 0 && s.st_mode & S_IXUSR);
}

bool check_device() {
  struct stat s;
  /* device with no partition in sda so we had to check /dev/sda1 */
  lstat("/dev/sda1", &s);
  return S_IFBLK == (s.st_mode & S_IFMT);
}

bool check_mount(const char *path) {

  struct stat mountpoint;
  struct stat parent;
  /* Get the stat structure of the directory...*/
  if (stat(path, &mountpoint) == -1) {
    return false;
  }

  /* ... and its parent. */
  char tmp[strlen(path) + 3];
  sprintf(tmp, "%s/..", path);
  if (stat(tmp, &parent) == -1) {
    return false;
  }

  /* Compare the st_dev fields in the results: if they are
     equal, then both the directory and its parent belong
     to the same filesystem, and so the directory is not
     currently a mount point.
  */
  if (mountpoint.st_dev == parent.st_dev) {
    return false;
  } else {
    return true;
  }
}

bool mount_device(const char *path) {
  int errno;
  if ((errno = mount("/dev/sda1", path, "vfat", MS_NOATIME, NULL)) == -1) {
    return false;
  } else {
    log_info("Mount successful");
    return true;
  }
}
bool umount_device(const char *path) {
  int errno;
  if ((errno = umount(path)) == -1) {
    return false;
  } else {
    log_info("Umount successful");
    return true;
  }
}
