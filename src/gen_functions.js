var fs = require("fs");

var _functions = [
  {
    ret: "int",
    name: "open",
    args: [
      ["const char*", "pathname", "open_intent"],
      ["int", "flags"],
      ["..."]
    ],
    has_64: true,
    variadic: [
      ["mode_t", "mode"]
    ],
    wrap: {
      func: "winevfs_wrap_open",
      args: ["pathname"]
    }
  },
  {
    ret: "int",
    name: "openat",
    args: [
      ["int", "dirfd"],
      ["const char*", "pathname", "open_intent"],
      ["int", "flags"],
      ["..."]
    ],
    has_64: true,
    variadic: [
      ["mode_t", "mode"]
    ],
    wrap: {
      func: "winevfs_wrap_open",
      args: ["pathname", "dirfd"]
    },
    at: "dirfd"
  },
  {
    ret: "int",
    name: "creat",
    args: [
      ["const char*", "file", "w"],
      ["int", "mode"]
    ],
    wrap: {
      func: "winevfs_wrap_open",
      args: ["file"]
    },
    has_64: true
  },
  {
    ret: "void*",
    name: "fopen",
    args: [
      ["const char*", "pathname", "fopen_intent"],
      ["const char*", "mode"]
    ],
    has_64: true
  },
  {
    ret: "void*",
    name: "freopen",
    args: [
      ["const char*", "pathname", "fopen_intent"],
      ["const char*", "mode"],
      ["void*", "stream"]
    ],
    has_64: true
  },
  {
    ret: "int",
    name: ["stat", "__xstat"],
    args: [
      ["int", "ver"],
      ["const char*", "path", "r"],
      ["struct stat*", "buf"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["stat64", "__xstat64"],
    args: [
      ["int", "ver"],
      ["const char*", "path", "r"],
      ["struct stat64*", "buf"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["lstat", "__lxstat", "__lstat"],
    args: [
      ["int", "ver"],
      ["const char*", "path", "r"],
      ["struct stat*", "buf"]
    ],
    has_64: true
  },
  {
    ret: "int",
    name: ["fstatat", "__fxstatat"],
    args: [
      ["int", "ver"],
      ["int", "dirfd"],
      ["const char*", "path", "r"],
      ["struct stat*", "statbuf"],
      ["int", "flags"]
    ],
    at: "dirfd",
    has_64: true
  },
  {
    ret: "int",
    name: ["faccessat", "__faccessat"],
    args: [
      ["int", "dirfd"],
      // FIXME: should it be R for W_OK?
      ["const char*", "file", "r"],
      ["int", "mode"],
      ["int", "flags"]
    ],
    at: "dirfd"
  },
  {
    ret: "int",
    name: ["access", "__access"],
    args: [
      ["const char*", "file", "r"],
      ["int", "type"]
    ]
  },
  {
    ret: "int",
    name: "unlink",
    args: [
      ["const char*", "path", "d"]
    ]
  },
  {
    ret: "int",
    name: "rename",
    args: [
      ["const char*", "old", "d"],
      ["const char*", "newpath", "w"]
    ]
  },
  {
    ret: "int",
    name: "renameat",
    args: [
      ["int", "oldfd"],
      ["const char*", "old", "d"],
      ["int", "newfd"],
      ["const char*", "newpath", "w"]
    ],
    at: {
      "old": "oldfd",
      "newpath": "newfd"
    }
  },
  {
    ret: "int",
    name: ["renameat2", "__renameat2"],
    args: [
      ["int", "oldfd"],
      ["const char*", "old", "d"],
      ["int", "newfd"],
      ["const char*", "newpath", "w"],
      ["unsigned int", "flags"]
    ],
    at: {
      "old": "oldfd",
      "newpath": "newfd"
    }
  },
  {
    ret: "int",
    name: "renameatu",
    args: [
      ["int", "fd1"],
      ["const char*", "src", "r"],
      ["int", "fd2"],
      ["const char*", "dst", "r"],
      ["unsigned int", "flags"]
    ],
    at: {
      "dst": "fd2",
      "src": "fd1"
    }
  },
  {
    ret: "int",
    name: "linkat",
    args: [
      ["int", "fromfd"],
      ["const char*", "from", "r"],
      ["int", "tofd"],
      ["const char*", "to", "w"],
      ["int", "flags"]
    ],
    at: {
      "to": "tofd",
      "from": "fromfd"
    }
  },
  {
    ret: "int",
    name: "symlinkat",
    args: [
      ["const char*", "from", "r"],
      ["int", "tofd"],
      ["const char*", "to", "w"],
    ],
    at: {
      "to": "tofd"
    }
  },
  {
    ret: "int",
    name: ["mkdir", "__mkdir"],
    args: [
      ["const char*", "path", "w"],
      //["mode_t", "mode"]
      ["unsigned int", "mode"]
    ]
  },
  {
    ret: "void*",
    name: "dlopen",
    disabled: true, // TODO: fix relative paths
    args: [
      ["const char*", "filename", "r"],
      ["int", "flags"]
    ]
  },
  {
    ret: "void*",
    name: ["opendir", "__opendir"],
    args: [
      ["const char*", "name", "r"]
    ],
    is_opendir: true,
    has_64: true
  },
  {
    ret: "void*",
    name: ["opendirat", "__opendirat"],
    args: [
      ["int", "dirfd"],
      ["const char*", "name", "r"]
    ],
    is_opendir: true,
    has_64: true,
    at: "dirfd"
  },
  {
    ret: "int",
    name: "utimensat",
    args: [
      ["int", "dirfd"],
      ["const char*", "pathname", "r"],
      ["const struct timespec*", "times"],
      ["int", "flags"]
    ],
    at: "dirfd"
  },
  {
    ret: "ssize_t",
    name: "readlink",
    args: [
      ["const char*", "path", "r"],
      ["char*", "buf"],
      ["size_t", "bufsiz"]
    ]
  },
  {
    ret: "int",
    name: ["chdir", "__chdir"],
    args: [
      ["const char*", "path", "r"]
    ],
    //origonly: true,
    //disabled: true // doesn't help
  },
  {
    ret: "int",
    name: ["chmod", "__chmod"],
    args: [
      ["const char*", "file", "r"],
      ["int", "mode"]
    ]
  },
  {
    ret: "int",
    name: "lchmod",
    args: [
      ["const char*", "file", "r"],
      ["int", "mode"]
    ]
  },
  {
    ret: "int",
    name: "fchmodat",
    args: [
      ["int", "fd"],
      ["const char*", "file", "r"],
      ["int", "mode"],
      ["int", "flag"]
    ],
    at: "fd"
  },
  {
    ret: "ssize_t",
    name: ["getxattr", "lgetxattr"],
    args: [
      ["const char*", "path", "r"],
      ["const char*", "name"],
      ["void*", "value"],
      ["size_t", "size"]
    ]
  },
  {
    ret: "ssize_t",
    name: ["listxattr", "llistxattr"],
    args: [
      ["const char*", "path", "r"],
      ["char*", "list"],
      ["size_t", "size"]
    ]
  },
  {
    ret: "int",
    name: ["setxattr", "lsetxattr"],
    args: [
      ["const char*", "path", "r"],
      ["const char*", "name"],
      ["const void*", "value"],
      ["size_t", "size"],
      ["int", "flags"]
    ]
  },
  {
    ret: "int",
    name: "removexattr",
    args: [
      ["const char*", "path", "r"],
      ["const char*", "name"]
    ]
  },
  // TODO: rmdir?

  {
    ret: "int",
    name: "closedir",
    args: [
      ["void*", "dir"]
    ],
    origonly: true
  },
  {
    ret: "void*",
    name: "readdir",
    args: [
      ["void*", "dir"]
    ],
    origonly: true,
    has_64: true
  },
  {
    ret: "int",
    name: ["fchdir", "__fchdir"],
    args: [
      ["int", "fd"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["close"],
    args: [
      ["int", "fd"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["dup", "__dup"],
    args: [
      ["int", "fd"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["dup2", "__dup2"],
    args: [
      ["int", "fd1"],
      ["int", "fd2"]
    ],
    origonly: true
  },
  {
    ret: "ssize_t",
    name: ["sendmsg"],
    args: [
      ["int", "socket"],
      ["void*", "message"],
      ["int", "flags"]
    ],
    origonly: true
  },
  {
    ret: "ssize_t",
    name: ["recvmsg"],
    args: [
      ["int", "socket"],
      ["void*", "message"],
      ["int", "flags"]
    ],
    origonly: true
  },
  {
    ret: "int",
    name: ["statfs", "__statfs"],
    args: [
      ["const char*", "file"],
      ["struct statfs*", "buf"]
    ],
    origonly: true,
    has_64: true
  },
  {
    ret: "int",
    name: ["fstatfs", "__fstatfs"],
    args: [
      ["int", "fd"],
      ["struct statfs*", "buf"]
    ],
    origonly: true
  },
  {
    ret: "pid_t",
    name: "setsid",
    args: [],
    origonly: true
  }
];

var functions = [];
_functions.forEach(fn => {
  if (fn.disabled)
    return;

  if (fn.name instanceof Array) {
    fn.name.forEach(name => {
      var newfn = JSON.parse(JSON.stringify(fn));
      newfn.name = name;
      functions.push(newfn);

      if (newfn.has_64) {
        newfn = JSON.parse(JSON.stringify(newfn));
        newfn.name += "64";
        functions.push(newfn);
      }
    });
  } else {
    functions.push(fn);

    if (fn.has_64) {
      fn = JSON.parse(JSON.stringify(fn));
      fn.name += "64";
      functions.push(fn);
    }
  }
});

function fnheader(fn, fnname, variadic) {
  var ret = fn.ret;
  ret += " " + (fnname || fn.name);
  ret += "(";

  var args = [];
  fn.args.forEach(arg => {
    if (arg.length === 1) {
      if (variadic)
        args.push(arg[0]);
      else {
        fn.variadic.forEach(varg => {
          args.push(varg[0] + " " + varg[1]);
        });
      }
    } else {
      args.push(arg[0] + " " + arg[1]);
    }
  });

  ret += args.join(", ");
  ret += ")";

  return ret;
}

function vartype(fn, varname) {
  var ret = fn.ret + " (*" + (varname || "") + ")(";

  var args = [];
  fn.args.forEach(arg => {args.push(arg[0]);});
  ret += args.join(", ");
  ret += ")";

  return ret;
}

function varnames(fn, variadic) {
  var args = [];
  fn.args.forEach(arg => {
    if (arg.length > 1)
      args.push(arg[1]);
    else if (variadic)
      args.push(arg[0]);
    else {
      fn.variadic.forEach(varg => {
        args.push(varg[1]);
      });
    }
  });

  return args.join(", ");
}

var retstr = "";

retstr += '#include "vfs_minimal.hpp"\n';
retstr += '#include "log.h"\n';
retstr += "#include <sys/types.h>\n";
retstr += "#define RTLD_NEXT ((void*) -1l)\n";
retstr += "#define O_CREAT 0100\n";

retstr += 'extern "C" {\n';
retstr += "extern void* dlsym (void* handle, const char* name);\n\n";
retstr += "extern void free(void *ptr);";
//retstr += "extern int puts(const char *s);";
retstr += "extern void winevfs_add_opendir(void* dir, const char* path, int atfd);";
retstr += "extern void winevfs_add_opendir64(void* dir, const char* path, int atfd);";
retstr += "extern void winevfs_wrap_open(int fd, const char* path, int atfd=AT_FDCWD);";
//retstr += "extern void fflush(void* stream);";
//retstr += "extern void* stdout;";


functions.forEach(fn => {
  retstr += fnheader(fn, "winevfs__" + fn.name) + " {\n";
  retstr += "    static " + vartype(fn, "original") + " = (" + vartype(fn) + ")dlsym(RTLD_NEXT, \"" + fn.name + "\");\n";
  retstr += "    return original(" + varnames(fn) + ");\n";
  retstr += "}\n\n";
});

retstr += "\n\n";

functions.forEach(fn => {
  if (fn.origonly)
    return;

  var fnname = null;
  if (fn.variadic)
    fnname = "winevfs_variadic__" + fn.name;
  retstr += fnheader(fn, fnname) + " {\n";

  if (fn.is_opendir) {
    retstr += "    const char* orig_name = name;\n"
  }

  if (fn.wrap) {
    fn.wrap.args.forEach(arg => {
      var oarg_obj;
      fn.args.forEach(oarg => {
        if (oarg[1] === arg) {
          oarg_obj = oarg;
        }
      });

      retstr += "    " + oarg_obj[0] + " orig_" + arg + " = " + arg + ";\n";
    });
  }

  function do_trace(inout) {
    retstr += "    trace(\"" + inout + ": " + fn.name + "(";
    var fmtargs = [];
    var outargs = [];
    fn.args.forEach(arg => {
      if (arg[0] === "...")
        return;

      var fmtarg = arg[1] + "=";

      if (arg[0] === "char*" ||
          arg[0] === "const char*") {
        fmtarg += "%s";
        outargs.push(arg[1]);
      } else if (arg[arg.length - 1] === "*") {
        fmtarg += "%p";
        outargs.push(arg[1]);
      } else {
        fmtarg += "%i";
        outargs.push(arg[1]);
      }

      fmtargs.push(fmtarg);
    });
    retstr += fmtargs.join(", ");
    retstr += ")\", ";
    retstr += outargs.join(", ");
    retstr += ");\n";
  }

  do_trace(" in");

  //retstr += "    puts(\"" + fn.name + "\");fflush(stdout);\n";
  fn.args.forEach(arg => {
    if (arg.length === 3) {
      var iname = arg[1] + "_intent";
      retstr += "    Intent " + iname + " = Intent_Read;\n";
      if (arg[2] === "open_intent") {
        retstr += "    if (flags & O_CREAT) {\n";
        retstr += "        " + iname + " = Intent_Create;\n";
        retstr += "    }\n";
      } else if (arg[2] === "fopen_intent") {
        retstr += "    if (mode && mode[0] == 'w') {\n";
        retstr += "        " + iname + " = Intent_Create;\n";
        retstr += "    }\n";
      } else if (arg[2] === "w") {
        retstr += "    " + iname + " = Intent_Create;\n";
      } else if (arg[2] === "d") {
        retstr += "    " + iname + " = Intent_Delete;\n";
      }

      //retstr += "    puts(" + arg[1] + ");fflush(stdout);\n";
      var at = "AT_FDCWD";
      if (fn.at) {
        if (typeof fn.at === "string") {
          at = fn.at;
        } else if (arg[1] in fn.at) {
          at = fn.at[arg[1]];
        }
      }
      retstr += "    " + arg[1] + " = winevfs_get_path(" + arg[1] + ", " + iname + ", " + at + ");\n";

      //retstr += "    puts(" + arg[1] + ");fflush(stdout);\n";
    }
  });

  do_trace("out");

  retstr += "    " + fn.ret + " ret = winevfs__" + fn.name + "(" + varnames(fn) + ");\n";

  fn.args.forEach(arg => {
    if (arg.length === 3) {
      retstr += "    free((void*)" + arg[1] + ");\n";
    }
  });

  if (fn.wrap) {
    retstr += "    " + fn.wrap.func + "(ret, ";
    var newargs = [];
    fn.wrap.args.forEach(arg => {
      newargs.push("orig_" + arg);
    });
    retstr += newargs.join(", ") + ");\n";
  }

  if (fn.is_opendir) {
    var at = "AT_FDCWD";
    if (fn.at) {
      at = fn.at;
    }

    retstr += "    winevfs_add_" + fn.name.replace(/_/g, "").replace(/at/, "") + "(ret, orig_name, " + at + ");\n";
  }

  retstr += "    return ret;\n";

  retstr += "}\n\n";
});

functions.forEach(fn => {
  if (!fn.variadic)
    return;

  retstr += fnheader(fn, false, true) + " {\n";
  retstr += "    void* args = __builtin_apply_args();\n";
  retstr += "    void* ret = __builtin_apply((void(*)(...))winevfs_variadic__" + fn.name + ", args, 100);\n";
  retstr += "    __builtin_return(ret);\n";
  retstr += "}\n\n";
});

retstr += "}\n";

fs.writeFileSync(__dirname + "/functions.cpp", retstr);

retstr = "#include <sys/types.h>\n";
retstr += 'extern "C" {\n';

functions.forEach(fn => {
  retstr += fnheader(fn, "winevfs__" + fn.name) + ";\n";
});

retstr += "}\n";
fs.writeFileSync(__dirname + "/functions.hpp", retstr);

//console.log(retstr);
