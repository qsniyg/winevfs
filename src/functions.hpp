extern "C" {
int winevfs__open(const char* pathname, int flags, ...);
int winevfs__openat(int dirfd, const char* pathname, int flags, ...);
void* winevfs__fopen(const char* pathname, const char* mode);
int winevfs____xstat(int ver, const char* path, struct stat* buf);
int winevfs____lxstat(int ver, const char* path, struct stat* buf);
int winevfs____fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags);
int winevfs__faccessat(int dirfd, const char* file, int mode, int flags);
int winevfs__unlink(const char* path);
int winevfs__rename(const char* old, const char* newpath);
int winevfs__renameat(int oldfd, const char* old, int newfd, const char* newpath);
int winevfs__renameatu(int fd1, const char* src, int fd2, const char* dst, unsigned int flags);
int winevfs__linkat(int fromfd, const char* from, int tofd, const char* to, int flags);
int winevfs__symlinkat(const char* from, int fd, const char* to);
int winevfs__mkdir(const char* path, unsigned int mode);
void* winevfs__opendir(const char* name);
}
