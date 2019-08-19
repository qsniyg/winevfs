## winevfs

This is a virtual filesystem for wine, written primarily as a native linux alternative
to Mod Organizer's USVFS (which is currently broken under wine).

It works by replacing various standard system library functions using LD_PRELOAD, similar
to how USVFS works under Windows.

Although this might also work for some Linux programs, it might not work with all (notably
`find` from findutils), as it only replaces functions, rather than the actual syscalls.
It also contains a few wine-specific hacks to either ensure it works, or try to improve
performance.

Among the hacks are:

 * Everything is treated case-insensitive (using a cached algorithm)
 * fork() is currently intentionally not properly handled, as overriding setsid() works just as well,
   while keeping performance by not duplicating reading the VFS
 * It pretends to be CIOPFS, which will sometimes prevent wine from running its own case-insensitive
   algorithm.

There are also a number of limitations that may or may not be addressed in future versions:

 * Deleting files only somewhat works. Deleting a directory, or files that lay outside of the VFS,
   may not be properly registered
 * Performance is currently not as good as it could be. There are some ways it can be improved,
   including better caching and a few executable-specific hacks. These are planned for a future version.
   * This has been partly fixed through the `?full=` option (allowing certain whitelisted executables to
     read the entire VFS before loading)
 * This has only been tested with Bethesda games, and a few tools related to them.
 * wineserver must not be already running prior to this, as it needs to hook LD_PRELOAD into it
   in order to function.
   * **Make sure the hooked wineserver is killed before running a program without winevfs!**
     If you don't, the program _will_ fail, some in more disasterous ways than others.
     * Mod Organizer will clear your entire profile.
 * The case-insensitive string comparison only supports ASCII, it needs to support the full
   Unicode specification to be correct.
 * It works using a path cache instead of a node tree. What originally started out as an optimization
   ended up making the codebase significantly more complex and slower in some cases. Fixing this
   would require rewriting a significant amount of the code.

Programs that currently work (tested):

 * Skyrim (LE)
 * Fallout 4
 * Fallout NV
 * FNIS (requires dotnet40 to work)
 * Bodyslide

Programs that do not work (yet):

 * Nemesis Unlimited Behavior Engine (requires wine-mono to work, it fails under dotnet40)
   * It's a heavily multithreaded application, which currently fails due to a few thread races
   * I'm currently trying to fix it

### Usage

Winevfs is primarily intended to be used with [movfs4l.py](https://github.com/ajventer/ksp_stuff/blob/master/movfs4l.py),
but it can also be run manually if needed.

It reads a VFS information file that contains a list of every file or directory that will be redirected,
in this format:

```
R
/path/to/virtual/file1
/path/to/real/file1
R
/path/to/virtual/folder1
/path/to/real/folder1
W
/path/to/virtual/folder1
/path/to/real/folder2
```

When reading `/path/to/virtual/file`, winevfs will silently redirect the call to `/path/to/real/file1` instead.
`/path/to/virtual/folder1` will contain every file/folder (recursively) from `/path/to/real/folder1` and
`/path/to/real/folder2`, in that order. When writing to `/path/to/virtual/folder1`, if the file/folder
does not already exist, it will create a new entry in `/path/to/real/folder2`.

Running winevfs requires you to specify the VFS information file in the `WINEVFS_VFSFILE` environment variable:

`WINEVFS_VFSFILE=/path/to/vfsinfo ./bin/winevfs wine SomeApp.exe`

Depending on your system, you may need to rename `lib` to `lib64`, and `lib32` to `lib`. Refer to your
/usr folder to see how it should be organized.
