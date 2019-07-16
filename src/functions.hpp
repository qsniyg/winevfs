#include <sys/types.h>
extern "C" {
int winevfs__open(const char* pathname, int flags, mode_t mode);
int winevfs__open64(const char* pathname, int flags, mode_t mode);
int winevfs__openat(int dirfd, const char* pathname, int flags, mode_t mode);
int winevfs__openat64(int dirfd, const char* pathname, int flags, mode_t mode);
int winevfs__creat(const char* file, int mode);
int winevfs__creat64(const char* file, int mode);
void* winevfs__fopen(const char* pathname, const char* mode);
void* winevfs__fopen64(const char* pathname, const char* mode);
void* winevfs__freopen(const char* pathname, const char* mode, void* stream);
void* winevfs__freopen64(const char* pathname, const char* mode, void* stream);
int winevfs__stat(int ver, const char* path, struct stat* buf);
int winevfs____xstat(int ver, const char* path, struct stat* buf);
int winevfs__stat64(int ver, const char* path, struct stat64* buf);
int winevfs____xstat64(int ver, const char* path, struct stat64* buf);
int winevfs__lstat(int ver, const char* path, struct stat* buf);
int winevfs__lstat64(int ver, const char* path, struct stat* buf);
int winevfs____lxstat(int ver, const char* path, struct stat* buf);
int winevfs____lxstat64(int ver, const char* path, struct stat* buf);
int winevfs____lstat(int ver, const char* path, struct stat* buf);
int winevfs____lstat64(int ver, const char* path, struct stat* buf);
int winevfs__fstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags);
int winevfs__fstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags);
int winevfs____fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags);
int winevfs____fxstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags);
int winevfs__faccessat(int dirfd, const char* file, int mode, int flags);
int winevfs____faccessat(int dirfd, const char* file, int mode, int flags);
int winevfs__access(const char* file, int type);
int winevfs____access(const char* file, int type);
int winevfs__unlink(const char* path);
int winevfs__rename(const char* old, const char* newpath);
int winevfs__renameat(int oldfd, const char* old, int newfd, const char* newpath);
int winevfs__renameat2(int oldfd, const char* old, int newfd, const char* newpath, unsigned int flags);
int winevfs____renameat2(int oldfd, const char* old, int newfd, const char* newpath, unsigned int flags);
int winevfs__renameatu(int fd1, const char* src, int fd2, const char* dst, unsigned int flags);
int winevfs__linkat(int fromfd, const char* from, int tofd, const char* to, int flags);
int winevfs__symlinkat(const char* from, int tofd, const char* to);
int winevfs__mkdir(const char* path, unsigned int mode);
int winevfs____mkdir(const char* path, unsigned int mode);
void* winevfs__opendir(const char* name);
void* winevfs__opendir64(const char* name);
void* winevfs____opendir(const char* name);
void* winevfs____opendir64(const char* name);
void* winevfs__opendirat(int dirfd, const char* name);
void* winevfs__opendirat64(int dirfd, const char* name);
void* winevfs____opendirat(int dirfd, const char* name);
void* winevfs____opendirat64(int dirfd, const char* name);
int winevfs__utimensat(int dirfd, const char* pathname, const struct timespec* times, int flags);
ssize_t winevfs__readlink(const char* path, char* buf, size_t bufsiz);
int winevfs__chdir(const char* path);
int winevfs____chdir(const char* path);
int winevfs__chmod(const char* file, int mode);
int winevfs____chmod(const char* file, int mode);
int winevfs__lchmod(const char* file, int mode);
int winevfs__fchmodat(int fd, const char* file, int mode, int flag);
ssize_t winevfs__getxattr(const char* path, const char* name, void* value, size_t size);
ssize_t winevfs__lgetxattr(const char* path, const char* name, void* value, size_t size);
ssize_t winevfs__listxattr(const char* path, char* list, size_t size);
ssize_t winevfs__llistxattr(const char* path, char* list, size_t size);
int winevfs__setxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int winevfs__lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int winevfs__removexattr(const char* path, const char* name);
int winevfs__closedir(void* dir);
void* winevfs__readdir(void* dir);
void* winevfs__readdir64(void* dir);
int winevfs__fchdir(int fd);
int winevfs____fchdir(int fd);
int winevfs__close(int fd);
int winevfs__dup(int fd);
int winevfs____dup(int fd);
ssize_t winevfs__sendmsg(int socket, void* message, int flags);
ssize_t winevfs__recvmsg(int socket, void* message, int flags);
int winevfs__statfs(const char* file, struct statfs* buf);
int winevfs__statfs64(const char* file, struct statfs* buf);
int winevfs____statfs(const char* file, struct statfs* buf);
int winevfs____statfs64(const char* file, struct statfs* buf);
int winevfs__fstatfs(int fd, struct statfs* buf);
int winevfs____fstatfs(int fd, struct statfs* buf);
}
