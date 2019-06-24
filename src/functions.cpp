#include "vfs_minimal.hpp"
#include <sys/types.h>
#define RTLD_NEXT ((void*) -1l)
#define O_CREAT 0x0100
extern "C" {
extern void* dlsym (void* handle, const char* name);

extern void free(void *ptr);extern int puts(const char *s);extern void winevfs_add_opendir(void* dir, const char* path);extern void winevfs_add_opendir64(void* dir, const char* path);int winevfs__open(const char* pathname, int flags, ...) {
    static int (*original)(const char*, int, ...) = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
    return original(pathname, flags);
}

int winevfs__open64(const char* pathname, int flags, ...) {
    static int (*original)(const char*, int, ...) = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open64");
    return original(pathname, flags);
}

int winevfs__openat(int dirfd, const char* pathname, int flags, ...) {
    static int (*original)(int, const char*, int, ...) = (int (*)(int, const char*, int, ...))dlsym(RTLD_NEXT, "openat");
    return original(dirfd, pathname, flags);
}

int winevfs__openat64(int dirfd, const char* pathname, int flags, ...) {
    static int (*original)(int, const char*, int, ...) = (int (*)(int, const char*, int, ...))dlsym(RTLD_NEXT, "openat64");
    return original(dirfd, pathname, flags);
}

int winevfs__creat(const char* file, int mode) {
    static int (*original)(const char*, int) = (int (*)(const char*, int))dlsym(RTLD_NEXT, "creat");
    return original(file, mode);
}

int winevfs__creat64(const char* file, int mode) {
    static int (*original)(const char*, int) = (int (*)(const char*, int))dlsym(RTLD_NEXT, "creat64");
    return original(file, mode);
}

void* winevfs__fopen(const char* pathname, const char* mode) {
    static void* (*original)(const char*, const char*) = (void* (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    return original(pathname, mode);
}

void* winevfs__fopen64(const char* pathname, const char* mode) {
    static void* (*original)(const char*, const char*) = (void* (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen64");
    return original(pathname, mode);
}

void* winevfs__freopen(const char* pathname, const char* mode, void* stream) {
    static void* (*original)(const char*, const char*, void*) = (void* (*)(const char*, const char*, void*))dlsym(RTLD_NEXT, "freopen");
    return original(pathname, mode, stream);
}

void* winevfs__freopen64(const char* pathname, const char* mode, void* stream) {
    static void* (*original)(const char*, const char*, void*) = (void* (*)(const char*, const char*, void*))dlsym(RTLD_NEXT, "freopen64");
    return original(pathname, mode, stream);
}

int winevfs__stat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "stat");
    return original(ver, path, buf);
}

int winevfs__stat64(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "stat64");
    return original(ver, path, buf);
}

int winevfs____xstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__xstat");
    return original(ver, path, buf);
}

int winevfs____xstat64(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__xstat64");
    return original(ver, path, buf);
}

int winevfs__lstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "lstat");
    return original(ver, path, buf);
}

int winevfs__lstat64(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "lstat64");
    return original(ver, path, buf);
}

int winevfs____lxstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__lxstat");
    return original(ver, path, buf);
}

int winevfs____lxstat64(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__lxstat64");
    return original(ver, path, buf);
}

int winevfs____lstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__lstat");
    return original(ver, path, buf);
}

int winevfs____lstat64(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__lstat64");
    return original(ver, path, buf);
}

int winevfs__fstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    static int (*original)(int, int, const char*, struct stat*, int) = (int (*)(int, int, const char*, struct stat*, int))dlsym(RTLD_NEXT, "fstatat");
    return original(ver, dirfd, path, statbuf, flags);
}

int winevfs__fstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    static int (*original)(int, int, const char*, struct stat*, int) = (int (*)(int, int, const char*, struct stat*, int))dlsym(RTLD_NEXT, "fstatat64");
    return original(ver, dirfd, path, statbuf, flags);
}

int winevfs____fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    static int (*original)(int, int, const char*, struct stat*, int) = (int (*)(int, int, const char*, struct stat*, int))dlsym(RTLD_NEXT, "__fxstatat");
    return original(ver, dirfd, path, statbuf, flags);
}

int winevfs____fxstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    static int (*original)(int, int, const char*, struct stat*, int) = (int (*)(int, int, const char*, struct stat*, int))dlsym(RTLD_NEXT, "__fxstatat64");
    return original(ver, dirfd, path, statbuf, flags);
}

int winevfs__faccessat(int dirfd, const char* file, int mode, int flags) {
    static int (*original)(int, const char*, int, int) = (int (*)(int, const char*, int, int))dlsym(RTLD_NEXT, "faccessat");
    return original(dirfd, file, mode, flags);
}

int winevfs____faccessat(int dirfd, const char* file, int mode, int flags) {
    static int (*original)(int, const char*, int, int) = (int (*)(int, const char*, int, int))dlsym(RTLD_NEXT, "__faccessat");
    return original(dirfd, file, mode, flags);
}

int winevfs__access(const char* file, int type) {
    static int (*original)(const char*, int) = (int (*)(const char*, int))dlsym(RTLD_NEXT, "access");
    return original(file, type);
}

int winevfs____access(const char* file, int type) {
    static int (*original)(const char*, int) = (int (*)(const char*, int))dlsym(RTLD_NEXT, "__access");
    return original(file, type);
}

int winevfs__unlink(const char* path) {
    static int (*original)(const char*) = (int (*)(const char*))dlsym(RTLD_NEXT, "unlink");
    return original(path);
}

int winevfs__rename(const char* old, const char* newpath) {
    static int (*original)(const char*, const char*) = (int (*)(const char*, const char*))dlsym(RTLD_NEXT, "rename");
    return original(old, newpath);
}

int winevfs__renameat(int oldfd, const char* old, int newfd, const char* newpath) {
    static int (*original)(int, const char*, int, const char*) = (int (*)(int, const char*, int, const char*))dlsym(RTLD_NEXT, "renameat");
    return original(oldfd, old, newfd, newpath);
}

int winevfs__renameatu(int fd1, const char* src, int fd2, const char* dst, unsigned int flags) {
    static int (*original)(int, const char*, int, const char*, unsigned int) = (int (*)(int, const char*, int, const char*, unsigned int))dlsym(RTLD_NEXT, "renameatu");
    return original(fd1, src, fd2, dst, flags);
}

int winevfs__linkat(int fromfd, const char* from, int tofd, const char* to, int flags) {
    static int (*original)(int, const char*, int, const char*, int) = (int (*)(int, const char*, int, const char*, int))dlsym(RTLD_NEXT, "linkat");
    return original(fromfd, from, tofd, to, flags);
}

int winevfs__symlinkat(const char* from, int fd, const char* to) {
    static int (*original)(const char*, int, const char*) = (int (*)(const char*, int, const char*))dlsym(RTLD_NEXT, "symlinkat");
    return original(from, fd, to);
}

int winevfs__mkdir(const char* path, unsigned int mode) {
    static int (*original)(const char*, unsigned int) = (int (*)(const char*, unsigned int))dlsym(RTLD_NEXT, "mkdir");
    return original(path, mode);
}

void* winevfs__opendir(const char* name) {
    static void* (*original)(const char*) = (void* (*)(const char*))dlsym(RTLD_NEXT, "opendir");
    return original(name);
}

void* winevfs__opendir64(const char* name) {
    static void* (*original)(const char*) = (void* (*)(const char*))dlsym(RTLD_NEXT, "opendir64");
    return original(name);
}

int winevfs__utimensat(int dirfd, const char* pathname, const struct timespec* times, int flags) {
    static int (*original)(int, const char*, const struct timespec*, int) = (int (*)(int, const char*, const struct timespec*, int))dlsym(RTLD_NEXT, "utimensat");
    return original(dirfd, pathname, times, flags);
}

ssize_t winevfs__readlink(const char* path, char* buf, size_t bufsiz) {
    static ssize_t (*original)(const char*, char*, size_t) = (ssize_t (*)(const char*, char*, size_t))dlsym(RTLD_NEXT, "readlink");
    return original(path, buf, bufsiz);
}

int winevfs__closedir(void* dir) {
    static int (*original)(void*) = (int (*)(void*))dlsym(RTLD_NEXT, "closedir");
    return original(dir);
}

void* winevfs__readdir(void* dir) {
    static void* (*original)(void*) = (void* (*)(void*))dlsym(RTLD_NEXT, "readdir");
    return original(dir);
}

void* winevfs__readdir64(void* dir) {
    static void* (*original)(void*) = (void* (*)(void*))dlsym(RTLD_NEXT, "readdir64");
    return original(dir);
}



int open(const char* pathname, int flags, ...) {
    Intent pathname_intent = Intent_Read;
    if (flags & O_CREAT) {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__open(pathname, flags);
    free((void*)pathname);
    return ret;
}

int open64(const char* pathname, int flags, ...) {
    Intent pathname_intent = Intent_Read;
    if (flags & O_CREAT) {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__open64(pathname, flags);
    free((void*)pathname);
    return ret;
}

int openat(int dirfd, const char* pathname, int flags, ...) {
    Intent pathname_intent = Intent_Read;
    if (flags & O_CREAT) {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__openat(dirfd, pathname, flags);
    free((void*)pathname);
    return ret;
}

int openat64(int dirfd, const char* pathname, int flags, ...) {
    Intent pathname_intent = Intent_Read;
    if (flags & O_CREAT) {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__openat64(dirfd, pathname, flags);
    free((void*)pathname);
    return ret;
}

int creat(const char* file, int mode) {
    Intent file_intent = Intent_Read;
    file_intent = Intent_Create;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs__creat(file, mode);
    free((void*)file);
    return ret;
}

int creat64(const char* file, int mode) {
    Intent file_intent = Intent_Read;
    file_intent = Intent_Create;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs__creat64(file, mode);
    free((void*)file);
    return ret;
}

void* fopen(const char* pathname, const char* mode) {
    Intent pathname_intent = Intent_Read;
    if (mode && mode[0] == 'w') {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    void* ret = winevfs__fopen(pathname, mode);
    free((void*)pathname);
    return ret;
}

void* fopen64(const char* pathname, const char* mode) {
    Intent pathname_intent = Intent_Read;
    if (mode && mode[0] == 'w') {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    void* ret = winevfs__fopen64(pathname, mode);
    free((void*)pathname);
    return ret;
}

void* freopen(const char* pathname, const char* mode, void* stream) {
    Intent pathname_intent = Intent_Read;
    if (mode && mode[0] == 'w') {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    void* ret = winevfs__freopen(pathname, mode, stream);
    free((void*)pathname);
    return ret;
}

void* freopen64(const char* pathname, const char* mode, void* stream) {
    Intent pathname_intent = Intent_Read;
    if (mode && mode[0] == 'w') {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    void* ret = winevfs__freopen64(pathname, mode, stream);
    free((void*)pathname);
    return ret;
}

int stat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__stat(ver, path, buf);
    free((void*)path);
    return ret;
}

int stat64(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__stat64(ver, path, buf);
    free((void*)path);
    return ret;
}

int __xstat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____xstat(ver, path, buf);
    free((void*)path);
    return ret;
}

int __xstat64(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____xstat64(ver, path, buf);
    free((void*)path);
    return ret;
}

int lstat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__lstat(ver, path, buf);
    free((void*)path);
    return ret;
}

int lstat64(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__lstat64(ver, path, buf);
    free((void*)path);
    return ret;
}

int __lxstat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____lxstat(ver, path, buf);
    free((void*)path);
    return ret;
}

int __lxstat64(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____lxstat64(ver, path, buf);
    free((void*)path);
    return ret;
}

int __lstat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____lstat(ver, path, buf);
    free((void*)path);
    return ret;
}

int __lstat64(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____lstat64(ver, path, buf);
    free((void*)path);
    return ret;
}

int fstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__fstatat(ver, dirfd, path, statbuf, flags);
    free((void*)path);
    return ret;
}

int fstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__fstatat64(ver, dirfd, path, statbuf, flags);
    free((void*)path);
    return ret;
}

int __fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____fxstatat(ver, dirfd, path, statbuf, flags);
    free((void*)path);
    return ret;
}

int __fxstatat64(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____fxstatat64(ver, dirfd, path, statbuf, flags);
    free((void*)path);
    return ret;
}

int faccessat(int dirfd, const char* file, int mode, int flags) {
    Intent file_intent = Intent_Read;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs__faccessat(dirfd, file, mode, flags);
    free((void*)file);
    return ret;
}

int __faccessat(int dirfd, const char* file, int mode, int flags) {
    Intent file_intent = Intent_Read;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs____faccessat(dirfd, file, mode, flags);
    free((void*)file);
    return ret;
}

int access(const char* file, int type) {
    Intent file_intent = Intent_Read;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs__access(file, type);
    free((void*)file);
    return ret;
}

int __access(const char* file, int type) {
    Intent file_intent = Intent_Read;
    file = winevfs_get_path(file, file_intent);
    int ret = winevfs____access(file, type);
    free((void*)file);
    return ret;
}

int unlink(const char* path) {
    Intent path_intent = Intent_Read;
    path_intent = Intent_Delete;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__unlink(path);
    free((void*)path);
    return ret;
}

int rename(const char* old, const char* newpath) {
    Intent old_intent = Intent_Read;
    old_intent = Intent_Delete;
    old = winevfs_get_path(old, old_intent);
    Intent newpath_intent = Intent_Read;
    newpath_intent = Intent_Create;
    newpath = winevfs_get_path(newpath, newpath_intent);
    int ret = winevfs__rename(old, newpath);
    free((void*)old);
    free((void*)newpath);
    return ret;
}

int renameat(int oldfd, const char* old, int newfd, const char* newpath) {
    Intent old_intent = Intent_Read;
    old_intent = Intent_Delete;
    old = winevfs_get_path(old, old_intent);
    Intent newpath_intent = Intent_Read;
    newpath_intent = Intent_Create;
    newpath = winevfs_get_path(newpath, newpath_intent);
    int ret = winevfs__renameat(oldfd, old, newfd, newpath);
    free((void*)old);
    free((void*)newpath);
    return ret;
}

int renameatu(int fd1, const char* src, int fd2, const char* dst, unsigned int flags) {
    Intent src_intent = Intent_Read;
    src = winevfs_get_path(src, src_intent);
    Intent dst_intent = Intent_Read;
    dst = winevfs_get_path(dst, dst_intent);
    int ret = winevfs__renameatu(fd1, src, fd2, dst, flags);
    free((void*)src);
    free((void*)dst);
    return ret;
}

int linkat(int fromfd, const char* from, int tofd, const char* to, int flags) {
    Intent from_intent = Intent_Read;
    from = winevfs_get_path(from, from_intent);
    Intent to_intent = Intent_Read;
    to_intent = Intent_Create;
    to = winevfs_get_path(to, to_intent);
    int ret = winevfs__linkat(fromfd, from, tofd, to, flags);
    free((void*)from);
    free((void*)to);
    return ret;
}

int symlinkat(const char* from, int fd, const char* to) {
    Intent from_intent = Intent_Read;
    from = winevfs_get_path(from, from_intent);
    Intent to_intent = Intent_Read;
    to_intent = Intent_Create;
    to = winevfs_get_path(to, to_intent);
    int ret = winevfs__symlinkat(from, fd, to);
    free((void*)from);
    free((void*)to);
    return ret;
}

int mkdir(const char* path, unsigned int mode) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs__mkdir(path, mode);
    free((void*)path);
    return ret;
}

void* opendir(const char* name) {
    const char* orig_name = name;
    Intent name_intent = Intent_Read;
    name = winevfs_get_path(name, name_intent);
    void* ret = winevfs__opendir(name);
    free((void*)name);
    winevfs_add_opendir(ret, orig_name);
    return ret;
}

void* opendir64(const char* name) {
    const char* orig_name = name;
    Intent name_intent = Intent_Read;
    name = winevfs_get_path(name, name_intent);
    void* ret = winevfs__opendir64(name);
    free((void*)name);
    winevfs_add_opendir64(ret, orig_name);
    return ret;
}

int utimensat(int dirfd, const char* pathname, const struct timespec* times, int flags) {
    Intent pathname_intent = Intent_Read;
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__utimensat(dirfd, pathname, times, flags);
    free((void*)pathname);
    return ret;
}

ssize_t readlink(const char* path, char* buf, size_t bufsiz) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    ssize_t ret = winevfs__readlink(path, buf, bufsiz);
    free((void*)path);
    return ret;
}

}
