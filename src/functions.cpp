#include "vfs_minimal.hpp"
#define RTLD_NEXT ((void*) -1l)
#define O_CREAT 0x0100
extern "C" {
extern void* dlsym (void* handle, const char* name);

extern void free(void *ptr);extern int puts(const char *s);int winevfs__open(const char* pathname, int flags, ...) {
    static int (*original)(const char*, int, ...) = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
    return original(pathname, flags);
}

int winevfs__fopen(const char* pathname, const char* mode) {
    static int (*original)(const char*, const char*) = (int (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    return original(pathname, mode);
}

int winevfs____xstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__xstat");
    return original(ver, path, buf);
}

int winevfs____lxstat(int ver, const char* path, struct stat* buf) {
    static int (*original)(int, const char*, struct stat*) = (int (*)(int, const char*, struct stat*))dlsym(RTLD_NEXT, "__lxstat");
    return original(ver, path, buf);
}

int winevfs____fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    static int (*original)(int, int, const char*, struct stat*, int) = (int (*)(int, int, const char*, struct stat*, int))dlsym(RTLD_NEXT, "__fxstatat");
    return original(ver, dirfd, path, statbuf, flags);
}

int winevfs__faccessat(int dirfd, const char* file, int mode, int flags) {
    static int (*original)(int, const char*, int, int) = (int (*)(int, const char*, int, int))dlsym(RTLD_NEXT, "faccessat");
    return original(dirfd, file, mode, flags);
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

int fopen(const char* pathname, const char* mode) {
    Intent pathname_intent = Intent_Read;
    if (mode && mode[0] == 'w') {
        pathname_intent = Intent_Create;
    }
    pathname = winevfs_get_path(pathname, pathname_intent);
    int ret = winevfs__fopen(pathname, mode);
    free((void*)pathname);
    return ret;
}

int __xstat(int ver, const char* path, struct stat* buf) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____xstat(ver, path, buf);
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

int __fxstatat(int ver, int dirfd, const char* path, struct stat* statbuf, int flags) {
    Intent path_intent = Intent_Read;
    path = winevfs_get_path(path, path_intent);
    int ret = winevfs____fxstatat(ver, dirfd, path, statbuf, flags);
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
    Intent name_intent = Intent_Read;
    name = winevfs_get_path(name, name_intent);
    void* ret = winevfs__opendir(name);
    free((void*)name);
    return ret;
}

}
