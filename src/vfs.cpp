#include "vfs.hpp"
#include "server.hpp"
#include "inotify.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mutex>
#include <string.h>
#include <iostream>
#include "functions.hpp"

// TODO: improve
std::string winevfs_lower(std::string& in) {
  std::string out = in;
  std::transform(out.begin(), out.end(), out.begin(), ::tolower);
  return out;
}

static std::string lower(std::string& in) {
  return winevfs_lower(in);
}

static int real_stat(const char* str, struct stat64* buf) {
  return winevfs____xstat64(_STAT_VER, str, buf);
}

static std::unordered_map<std::string, struct stat64> stat_cache;
static std::mutex stat_cache_mutex;
int winevfs_stat(std::string path_str, struct stat64* buf, bool use_cache = false) {
  if (use_cache) {
    std::lock_guard<std::mutex> lock(stat_cache_mutex);
    auto it = stat_cache.find(path_str);
    if (it != stat_cache.end()) {
      //printf("Used cache: %s\n", path_str.c_str());fflush(stdout);
      *buf = it->second;
      return 0;
    }
  }

  int ret = real_stat(path_str.c_str(), buf);
  if (ret == 0) {
    std::lock_guard<std::mutex> lock(stat_cache_mutex);
    stat_cache[path_str] = *buf;
  }

  return ret;
}

int fs_stat(std::filesystem::path path, struct stat64* buf, bool use_cache = false) {
  return winevfs_stat(path, buf, use_cache);
}

static bool fs_exists(std::filesystem::path source, bool use_cache = false) {
  struct stat64 s;
  return fs_stat(source, &s, use_cache) == 0;
}

static bool fs_isdir(std::filesystem::path source, bool use_cache = false) {
  struct stat64 s;
  if (fs_stat(source, &s, use_cache))
    return false;

  return S_ISDIR(s.st_mode);
}

static DIR* fs_opendir(std::filesystem::path source) {
  std::string path_str = source;
  return (DIR*)winevfs__opendir(path_str.c_str());
}

static bool has_parent_path(std::filesystem::path source) {
  return source.has_parent_path() && source.parent_path() != source;
}

static std::filesystem::path parent_path(std::filesystem::path source) {
  if (true || source.has_filename())
    return source.parent_path();
  else
    return source.parent_path().parent_path();
}

static void fs_mkdir_p(std::filesystem::path source) {
  if (!source.has_parent_path()) {
    return;
  }

  if (!fs_isdir(source)) {
    std::filesystem::path parent = source.parent_path();
    fs_mkdir_p(parent);

    winevfs__mkdir(std::string(source).c_str(), 0777);
  }
}

static std::string fakecwd;
static std::mutex fakecwd_mutex;
void winevfs_setcwd(const char* cwd) {
  std::lock_guard<std::mutex> lock(fakecwd_mutex);
  fakecwd = cwd;
}

static std::filesystem::path fs_getcwd() {
  if (false) {
    std::lock_guard<std::mutex> lock(fakecwd_mutex);
    if (fakecwd.size() > 0) {
      //printf("FAKECWD: %s\n", fakecwd.c_str());fflush(stdout);
      return fakecwd;
    }
  }

  char path[PATH_MAX];
  getcwd(path, PATH_MAX - 1);

  //printf("CWD: %s\n", path);fflush(stdout);
  return std::filesystem::path(path);
}

static std::filesystem::path abspath_simple(std::filesystem::path source) {
  std::filesystem::path new_path;

  for (auto it = source.begin(); it != source.end(); it++) {
    if (*it == "." || *it == "")
      continue;
    if (*it == "..") {
      if (new_path.has_parent_path()) {
        new_path = new_path.parent_path();
        continue;
      }
    }

    new_path /= *it;
  }

  return new_path;
}

std::unordered_map<int, std::string> winevfs_fd_table;
std::mutex winevfs_fd_table_mutex;

std::string winevfs_get_fd_path(int atfd) {
  //printf("FD: %i\n", atfd);fflush(stdout);
  if (atfd == AT_FDCWD)
    // TODO: optimize, make a new function that returns std::string instead of std::filesystem::path
    return fs_getcwd();

  std::string retpath;

  if (false) {
    std::lock_guard<std::mutex> lock(winevfs_fd_table_mutex);
    auto it = winevfs_fd_table.find(atfd);
    if (it != winevfs_fd_table.end()) {
      //puts("Found in FD table");fflush(stdout);
      retpath = it->second;
    }
  }

  static int pid = getpid();

  char procfdpath[PATH_MAX];
  sprintf(procfdpath, "/proc/%i/fd/%i", pid, atfd);

  char readlinked[PATH_MAX];
  ssize_t bytes = winevfs__readlink(procfdpath, readlinked, PATH_MAX - 1);
  if (bytes <= 0) {
    return fs_getcwd();
  }
  readlinked[bytes] = 0;

  //printf("Found in proc: %s\n", readlinked);fflush(stdout);
  if (false && retpath.size() > 0)
    return retpath;

  return std::string(readlinked);
}

static std::filesystem::path get_fd_path(int atfd) {
  return winevfs_get_fd_path(atfd);
}

std::filesystem::path full_abspath(std::filesystem::path source, int atfd) {
  std::filesystem::path path = abspath_simple(source);

  if (path.is_absolute())
    return path;

  path = get_fd_path(atfd) / path;
  return abspath_simple(path);
}

/*std::string winevfs_readlink_wrapper(const char* path) {
  }*/

#if 0
static std::filesystem::path simple_readlink(std::filesystem::path source) {
  source = abspath_simple(source);

  char readlinked[PATH_MAX];
  if (winevfs__readlink(source.c_str(), readlinked, PATH_MAX - 1) < 0) {
    return source;
  }

  std::filesystem::path readlinked_path = readlinked;
  if (!readlinked_path.is_absolute())
    readlinked_path = abspath_simple(source / readlinked_path);

  return readlinked_path;
}
#endif

static std::unordered_map<std::string, std::string> readlink_cache;
static std::mutex readlink_cache_mutex;

static std::filesystem::path cached_readlink(std::filesystem::path source) {
  source = abspath_simple(source);
  std::string source_str = source;
  if (!strncmp(source_str.c_str(), "/proc", sizeof("/proc") - 1)) {
    // TODO: readlink this too
    return source;
  }

  //std::string lowersource = lower(source_str);
  {
    std::lock_guard<std::mutex> lock(readlink_cache_mutex);
    //auto it = readlink_cache.find(lowersource);
    auto it = readlink_cache.find(source_str);
    if (it != readlink_cache.end())
      return it->second;
  }

  //puts(source_str.c_str());

  if (has_parent_path(source)) {
    std::filesystem::path readlinked_parent = cached_readlink(source.parent_path());
    source = readlinked_parent / source.filename();
    //source_str = source;
    //lowersource = lower(source_str);
  }

  // TODO: if parent doesn't exist, don't check readlink
  char readlinked[PATH_MAX];
  ssize_t bytes = winevfs__readlink(source.c_str(), readlinked, PATH_MAX - 1);
  if (bytes <= 0) {
    if (errno != EINVAL && false) // EINVAL = exists, but not a link
      return source;
    else
      strcpy(readlinked, std::string(source).c_str());
  } else {
    readlinked[bytes] = 0;
  }

  std::filesystem::path readlinked_path = readlinked;
  if (!readlinked_path.is_absolute())
    readlinked_path = abspath_simple(source.parent_path() / readlinked_path);

  std::lock_guard<std::mutex> lock(readlink_cache_mutex);
  //readlink_cache[lowersource] = readlinked_path;
  //std::cout << source_str << std::endl;
  //std::cout << std::string(source) << std::endl;
  //std::cout << "---" << std::endl;
  readlink_cache[source_str] = readlinked_path;
  readlink_cache[source] = readlinked_path;

  return readlinked_path;
}

std::filesystem::path winevfs_abspath(std::filesystem::path source, int atfd) {
  std::filesystem::path path = abspath_simple(source);

  /*if (path.is_absolute())
    return path;*/

  if (!path.is_absolute())
    path = get_fd_path(atfd) / path;
  std::filesystem::path readlinked = cached_readlink(path);
  return readlinked;
}

std::string winevfs_abspath(std::string source, int atfd) {
  return std::string(winevfs_abspath(std::filesystem::path(source), atfd));
}

void unique_vector::insert(std::string string) {
  std::string lowerstring = lower(string);
  auto it = this->set.find(lowerstring);
  if (it == this->set.end()) {
    this->set.insert(lowerstring);
    this->vector.push_back(string);
  }
}

void unique_vector::erase(std::string string) {
  std::string lowerstring = lower(string);
  auto it = this->set.find(lowerstring);
  if (it != this->set.end()) {
    this->set.erase(lowerstring);

    for (auto vit = this->vector.begin(); vit != this->vector.end(); vit++) {
      std::string lowerit = lower(*vit);
      if (lowerit == lowerstring) {
        this->vector.erase(vit);
        break;
      }
    }
  }
}

bool unique_vector::empty() {
  return this->vector.empty();
}

std::string unique_vector::last() {
  return this->vector.back();
}

// TODO: somehow make this work better multi-process
static std::unordered_map<std::string, std::string> read_mappings;
static std::mutex read_mappings_mutex;
static std::unordered_map<std::string, std::string> write_mappings;
static std::mutex write_mappings_mutex;
std::unordered_map<std::string, folder_mapping> winevfs_folder_mappings;
std::unordered_map<std::string, std::string> winevfs_reverse_folder_mappings;
std::mutex winevfs_folder_mappings_mutex;
//std::unordered_map<std::string, std::string> unlink_mappings; // TODO
static bool inited = false;
static std::mutex inited_mutex;

static void add_folder(std::filesystem::path folder, std::filesystem::path dest) {
  std::string folder_str = folder;
  std::string lowerfolder = lower(folder_str);
  std::string folder_filename = folder.filename();

  if (has_parent_path(folder)) {
    std::filesystem::path parent_folder = parent_path(folder);
    std::string parent_filename = parent_folder.filename();
    parent_filename = lower(parent_filename);
    std::filesystem::path parent_dest = parent_path(dest);
    std::string parent_dest_filename = parent_dest.filename();
    parent_dest_filename = lower(parent_dest_filename);

    if (parent_filename != parent_dest_filename) {
      parent_dest = std::filesystem::path();
    }

    add_folder(parent_folder, parent_dest);

    std::string parent_str = parent_path(folder);
    std::string lowerparent = lower(parent_str);

    {
      std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
      auto it = winevfs_folder_mappings.find(lowerparent);
      if (it != winevfs_folder_mappings.end()) {
        it->second.children.insert(folder_filename);
      } else {
        //std::cout << "PARENT NOT FOUND: " << lowerparent << std::endl;
      }
    };
  }

  std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
  if (winevfs_folder_mappings.find(lowerfolder) == winevfs_folder_mappings.end()) {
    folder_mapping folder_set;
    if (!dest.empty())
      folder_set.folder_paths.insert(dest);
    winevfs_folder_mappings[lowerfolder] = folder_set;
  } else if (!dest.empty()) {
    winevfs_folder_mappings[lowerfolder].folder_paths.insert(dest);
  }

  //std::cout << "M: " << lowerfolder << " = " << dest << std::endl;

  if (!dest.empty()) {
    std::string lowerdest = dest;
    lowerdest = lower(lowerdest);
    if (lowerdest == lowerfolder)
      return;

    winevfs_reverse_folder_mappings[lowerdest] = lowerfolder;
    //std::cout << "F: " << lowerdest << " = " << lowerfolder << std::endl;

    std::filesystem::path lowerpath = lowerfolder;
    std::filesystem::path basepath = lowerpath.filename();
    while (has_parent_path(lowerpath)) {
      std::filesystem::path orig_lowerpath = lowerpath;
      lowerpath = parent_path(lowerpath);

      //std::cout << "NL: " << orig_lowerpath << std::endl;

      auto folder_it = winevfs_folder_mappings.find(lowerpath);
      if (folder_it != winevfs_folder_mappings.end()) {
        if (!folder_it->second.folder_paths.empty()) {
          std::string parent_altpath = folder_it->second.folder_paths.last();
          parent_altpath = lower(parent_altpath);

          //std::cout << "PAL: " << parent_altpath << std::endl;

          auto it = winevfs_reverse_folder_mappings.find(parent_altpath);
          if (it != winevfs_reverse_folder_mappings.end()) {
            //std::filesystem::path altpath = it->second;
            //altpath = altpath / basepath;
            //std::string altlower = altpath;
            //altlower = lower(altlower);

            std::filesystem::path altpath = parent_altpath;
            altpath = altpath / basepath;

            //std::cout << "LOWER: " << lowerdest << std::endl;
            //std::cout << "ALT: " << altpath << " -> " << orig_lowerpath << std::endl;
            winevfs_reverse_folder_mappings[altpath] = orig_lowerpath;
          }
        }
      }

      //lowerdest = lowerpath;
      //lowerdest = lower(lowerdest);



      std::string lowerfilename = lowerpath.filename();
      basepath = lower(lowerfilename);// / basepath;
    }
  }
}

static void _add_read_entry(std::string source, std::string destination, bool isfolder = false) {
  if (!isfolder) {
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    read_mappings[lower(source)] = destination;
  } else {
    add_folder(source, destination);
    winevfs_watch_directory(destination);
  }

  //std::cout << "RE: " << source << " -> " << destination << std::endl;

  std::filesystem::path source_path = source;
  add_folder(parent_path(source_path), parent_path(destination));

  {
    std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
    std::string parent_str = parent_path(source_path);
    std::string source_filename = source_path.filename();
    winevfs_folder_mappings[lower(parent_str)].children.insert(source_filename);
  };
  //std::cout << source << std::endl;
  //std::cout << destination << std::endl;
}

void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination,
                                std::string search, bool recursive) {
  source = winevfs_abspath(source);
  destination = winevfs_abspath(destination);

  std::string wanted_entry;
  if (!search.empty()) {
    std::string lower_src = source;
    lower_src = lower(lower_src);
    search = lower(search);

    //printf("LowerSrc: %s\n", lower_src.c_str());
    //printf("Search:   %s\n", search.c_str());
    if (!strncmp(lower_src.c_str(), search.c_str(), lower_src.size())) {
      const char* rest = search.c_str() + lower_src.size();
      if (rest[0] == '/')
        rest++;

      //printf("Rest: %s\n", rest);

      wanted_entry = rest;
      wanted_entry = wanted_entry.substr(0, wanted_entry.find("/", 0));

      //std::filesystem::path rest_path = rest;
      //wanted_entry = rest_path.root_path();
      //printf("Wanted: %s\n", wanted_entry.c_str());
    }
  }

  DIR* d = fs_opendir(destination);
  if (d) {
    _add_read_entry(source, destination, true);

    struct dirent* entry;
    while ((entry = (struct dirent*)winevfs__readdir(d)) != NULL) {
      if (entry->d_name[0] == '.') {
        if (entry->d_name[1] == 0)
          continue;

        if (entry->d_name[1] == '.' && entry->d_name[2] == 0)
          continue;
      }

      std::string out = destination / entry->d_name;
      std::string path = source / entry->d_name;

      if (fs_isdir(out)) {
        _add_read_entry(path, out, true);

        std::string entry_name = entry->d_name;
        entry_name = lower(entry_name);
        //std::cout << entry_name << " " << wanted_entry << std::endl;
        if (recursive || (!wanted_entry.empty() && entry_name == wanted_entry)) {
          winevfs_add_read_directory(path, out, search);
        }
      } else {
        _add_read_entry(path, out);
      }
    }

    winevfs__closedir(d);

    return;
  }

  _add_read_entry(source, destination);
}

void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination) {
  source = winevfs_abspath(source);
  destination = winevfs_abspath(destination);

  _add_read_entry(source, destination);
}

void winevfs_add_simple_read_folder(std::filesystem::path source, std::filesystem::path destination) {
  source = winevfs_abspath(source);
  destination = winevfs_abspath(destination);

  _add_read_entry(source, destination, true);
}

void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination) {
  source = winevfs_abspath(source);
  destination = winevfs_abspath(destination);
  std::string source_string = source;

  std::lock_guard<std::mutex> lock(write_mappings_mutex);
  write_mappings[lower(source_string)] = destination;
}

void winevfs_delete_file(std::filesystem::path source) {
  // TODO: Handle directories
  // TODO: Remove from winpath cache
  std::string source_str = source;

  {
    std::lock_guard<std::mutex> lock(stat_cache_mutex);
    stat_cache.erase(source_str);
  }

  {
    std::lock_guard<std::mutex> lock(readlink_cache_mutex);
    readlink_cache.erase(source_str);
  }

  //std::string source_lower = lower(source_str);

  /*{
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    read_mappings.erase(source_lower);
    }*/

  std::string parent = source.parent_path();
  std::string parent_lower = lower(parent);
  //printf("PL: %s\n", parent_lower.c_str());fflush(stdout);
  // TODO: also check in reverse_folder_mappings

  {
    std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
    auto reverse_it = winevfs_reverse_folder_mappings.find(parent_lower);
    if (reverse_it != winevfs_reverse_folder_mappings.end()) {
      std::string reverse_lower = lower(reverse_it->second);
      auto it = winevfs_folder_mappings.find(reverse_lower);
      if (it != winevfs_folder_mappings.end()) {
        //printf("ERASE: %s\n", source.filename().c_str());fflush(stdout);
        it->second.children.erase(source.filename());
      }
    }
  }
}

static std::unordered_map<std::string, std::filesystem::path> path_cache;
static std::mutex path_cache_mutex;
static std::filesystem::path winpath(std::filesystem::path source, int atfd) {
  // TODO: maybe rewrite so that it checks the cache first?
  if (atfd != AT_FDCWD)
    source = full_abspath(source, atfd);

  if (fs_exists(source)) {
    return source;
  }

  if (atfd == AT_FDCWD)
    source = full_abspath(source, atfd);

  std::string path_str = source;
  path_str = lower(path_str);

  {
    std::lock_guard<std::mutex> lock(path_cache_mutex);
    auto it = path_cache.find(path_str);
    if (it != path_cache.end())
      return it->second;
  }

  if (!source.has_parent_path())
    return source;

  std::filesystem::path parent = source.parent_path();
  std::filesystem::path basename = source.filename();
  std::string basename_lower = basename;
  basename_lower = lower(basename_lower);

  std::filesystem::path winparent = winpath(parent, atfd);
  std::string winparent_lower = winparent;
  winparent_lower = lower(winparent_lower);

  {
    std::lock_guard<std::mutex> lock(path_cache_mutex);
    auto it = path_cache.find(winparent_lower);
    if (it == path_cache.end())
      path_cache[winparent_lower] = winparent;
  }

  // No need to check if it's a directory, it will return NULL if it's not
  DIR* d = fs_opendir(winparent);
  if (d) {
    struct dirent* entry;
    while ((entry = (struct dirent*)winevfs__readdir(d)) != NULL) {
      std::string filename = entry->d_name;
      if (lower(filename) == basename_lower) {
        winevfs__closedir(d);
        return winparent / filename;
      }
    }

    winevfs__closedir(d);
  }

  return winparent / basename;
}

std::string winevfs_winpath(std::string source, int atfd) {
  return winpath(source, atfd);
}

void winevfs_read_vfsfile(char* envfile) {
  FILE* fp = (FILE*)winevfs__fopen(envfile, "r");
  if (!fp) {
    printf("Can't find vfs file at: %s\n", envfile);fflush(stdout);
    return;
  } else {
    printf("Reading vfs file: %s\n", envfile);fflush(stdout);
  }

  ssize_t read;
  size_t len = 0;
  char* line = NULL;

  bool quick = false;
  bool first = true;

  int phase = 0;
  bool is_read = false;
  bool is_delete = false;
  std::string input = "";
  std::string output = "";
  int entries = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    // Remove trailing \n
    line[strlen(line) - 1] = 0;

    if (first) {
      first = false;

      if (!strcmp(line, "quick")) {
        quick = true;
        continue;
      }
    }

    if (phase == 0) {
      is_read = false;
      is_delete = false;

      if (line[0] == 'R') {
        is_read = true;
      } else if (line[0] == 'W') {
        is_read = false;
      } else if (line[0] == 'D') {
        is_delete = true;
      }

      phase = 1;
      continue;
    }

    if (phase == 1) {
      input = line;

      phase = 2;
      continue;
    }

    if (phase == 2) {
      output = line;

      if (is_delete) {
        winevfs_delete_file(input);
      } else {
        if (is_read || true) {
          //printf("In: %s -> Out %s\n", input.c_str(), output.c_str());
          if (quick) {
            winevfs_add_read_file(input, output);
          } else {
            //winevfs_add_read_directory(input, output);
            if (fs_isdir(output)) {
              winevfs_add_simple_read_folder(input, output);
            } else {
              winevfs_add_read_file(input, output);
            }
            //_add_read_entry(input, output, true);
          }
        }

        if (!is_read) {
          winevfs_add_write_directory(input, output);
        }

        entries++;
      }

      phase = 0;
      continue;
    }
  }

  printf("Found %i entries\n", entries);fflush(stdout);

  free(line);

  fclose(fp);
}

void winevfs_write_vfsfile(char* envfile) {
  FILE* fp = (FILE*)winevfs__fopen(envfile, "w");

  fputs("quick\n", fp);

  for (auto it = read_mappings.begin(); it != read_mappings.end(); it++) {
    fputs("R\n", fp);

    fputs(it->first.c_str(), fp);
    fputc('\n', fp);

    fputs(it->second.c_str(), fp);
    fputc('\n', fp);
  }

  for (auto it = write_mappings.begin(); it != write_mappings.end(); it++) {
    fputs("W\n", fp);

    fputs(it->first.c_str(), fp);
    fputc('\n', fp);

    fputs(it->second.c_str(), fp);
    fputc('\n', fp);
  }

  fclose(fp);
}

#ifndef SERVER_BUILD
static void server_client_cb(char command, void* data, int size) {
  char* first = (char*)data;
  char* second = NULL;
  if (command == 'R') {
    second = first + strlen(first) + 1;
    puts("READ:");fflush(stdout);
    printf("dest: %s\nsrc: %s\n", first, second);fflush(stdout);
    _add_read_entry(first, second);
  } else if (command == 'D') {
    puts("DELETE:");
    printf("file: %s\n", first);fflush(stdout);
    winevfs_delete_file(first);
  }
}
#endif

std::mutex winevfs_client_processing_mutex;
static void listener_cb(fs_event event) {
  std::lock_guard<std::mutex> processing_lock(winevfs_client_processing_mutex);

  if (event.event == 'R') {
    //puts("READ:");fflush(stdout);
    std::filesystem::path path = event.file;
    std::filesystem::path newpath;
    std::string lower_path = parent_path(path);
    lower_path = lower(lower_path);
    bool found = false;

    //printf("EF: %s\n", event.file.c_str());fflush(stdout);
    //printf("LP: %s\n", lower_path.c_str());fflush(stdout);

    {
      std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
      auto it = winevfs_reverse_folder_mappings.find(lower_path);
      if (it != winevfs_reverse_folder_mappings.end()) {
        found = true;
        newpath = it->second;
        //printf("NP: %s\n", newpath.c_str());fflush(stdout);
      }
    }

    if (!found) {
      //puts("NOT FOUND");fflush(stdout);
      return;
    }

    newpath = newpath / path.filename();
    lower_path = newpath;
    lower_path = lower(lower_path);

    //printf("FP: %s\n", lower_path.c_str());fflush(stdout);

    if (event.isdir) {
      winevfs_add_read_directory(lower_path, path, "", true);
    } else {
      winevfs_add_read_file(lower_path, path);
    }
  } else {
    //puts("DELETE:");fflush(stdout);
    //printf("file: %s\n", event.file.c_str());fflush(stdout);
    winevfs_delete_file(event.file);
  }
}

void winevfs_init(bool client, bool force) {
  std::lock_guard<std::mutex> lock(inited_mutex);
  if (inited && !force)
    return;

  // TODO: empty this directory
  fs_mkdir_p("/tmp/.winevfs/fakedir/");

  char* envfile = getenv("WINEVFS_VFSFILE");
  if (!envfile || !envfile[0]) {
    inited = true;
    return;
  }

  if (client) {
    //winevfs_init_client(server_client_cb);
    winevfs_init_listen(listener_cb);
  }

  winevfs_read_vfsfile(envfile);

  if (false) {
    std::string patchfile = envfile;
    patchfile += "_patch";
    winevfs_read_vfsfile((char*)patchfile.c_str());
  }

  inited = true;
}

static std::string find_read_mapping(std::string path, bool simple = false) {
  std::string path_lower = lower(path);

  {
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    auto it = read_mappings.find(path_lower);
    if (it != read_mappings.end()) {
      return it->second;
    }
  }

  if (simple)
    return "";

  // TODO: Have top-level output directory mapping to see if path_lower
  //   is in the mappings. Would speed up performance

  std::filesystem::path path_fs = path;

  while (has_parent_path(path_fs)) {
    std::string path_str = path_fs;
    std::string path_str_lower = lower(path_str);

    std::vector<std::string> foldervec;
    {
      std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
      auto it = winevfs_folder_mappings.find(path_str_lower);
      if (it != winevfs_folder_mappings.end() && it->second.children.empty()) {
        //printf("Found empty path for: %s\n", path_str.c_str());fflush(stdout);
        foldervec = it->second.folder_paths.vector;
      } else {
        // TODO: add cache
        if (it == winevfs_folder_mappings.end()) {
          //printf("Not found path for: %s\n", path_str.c_str());fflush(stdout);
        } else {
          if (false) {
            printf("Not found path for: %s (but has children):\n", path_str.c_str());
            if (!it->second.folder_paths.empty()) {
              printf("  %s\n", it->second.folder_paths.last().c_str());
            }
          }
          return "";
          if (false) {
            for (auto vit = it->second.children.vector.begin();
                 vit != it->second.children.vector.end();
                 vit++) {
              printf("    %s\n", vit->c_str());fflush(stdout);
            }
          }
          fflush(stdout);
        }
      }
    }

    if (!foldervec.empty()) {
      for (auto fp_it = foldervec.begin();
           fp_it != foldervec.end();
           fp_it++) {
        //printf("Adding %s for %s\n", (*fp_it).c_str(), path_str_lower.c_str());fflush(stdout);
        winevfs_add_read_directory(path_str_lower, *fp_it, path_lower);
      }

      return find_read_mapping(path, true);
    }

    path_fs = path_fs.parent_path();
  }

  return "";
}

//extern
std::string winevfs_get_path_inner(std::filesystem::path in, Intent intent, int atfd) {
  //printf("%i\n", getpid());

#ifndef SERVER_BUILD
  {
    std::lock_guard<std::mutex> client_lock(winevfs_client_processing_mutex);
  }
#endif

  std::filesystem::path path;

  // TODO: CWD for folders that still need to be found through find_read_mappings
  if (false && in.is_relative() && !has_parent_path(in)) {
    std::filesystem::path cwd = fs_getcwd();
    std::string cwd_lower = cwd;
    cwd_lower = lower(cwd_lower);

    std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
    auto reverse_it = winevfs_reverse_folder_mappings.find(cwd_lower);
    if (reverse_it != winevfs_reverse_folder_mappings.end()) {
      in = reverse_it->second / in;
      std::cout << "CWD: " << cwd << std::endl;fflush(stdout);
      std::cout << "RIS: " << reverse_it->second << std::endl;fflush(stdout);
      std::cout << "NEWIN: " << in << std::endl;fflush(stdout);
    }
  }

  //std::cout << in << std::endl;
  path = winevfs_abspath(in, atfd);

  if (path.filename() == ".ciopfs") {
    return "/dev/null";
  }

  std::string path_str = path;
  //std::cout << path << std::endl;
  std::string path_lower = lower(path_str);

  //std::cout << "LOWER: " << path_lower << std::endl;fflush(stdout);

  {
    std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
    std::string path_add;
    std::string new_path_lower = path_lower;
    std::filesystem::path path_lower_fs = new_path_lower;

    while (has_parent_path(path_lower_fs)) {
      //printf("Trying %s\n", (new_path_lower).c_str());
      auto reverse_it = winevfs_reverse_folder_mappings.find(new_path_lower);
      if (reverse_it != winevfs_reverse_folder_mappings.end()) {
        //printf("Found reverse: %s -> %s\n", path_lower.c_str(), (reverse_it->second + path_add).c_str());fflush(stdout);
        path_lower = reverse_it->second + path_add;
        path = path_lower;
        path_str = path_lower;
        break;
      }

      path_add = std::string("/") + std::string(path_lower_fs.filename()) + path_add;
      path_lower_fs = parent_path(path_lower_fs);
      new_path_lower = path_lower_fs;
    }
  }

  // TODO: Handle Intent_Delete -- done(ish)
  // TODO: Pretend it's case insensitive:
  //   https://github.com/wine-mirror/wine/blob/ba9f3dc198dfc81bb40159077b73b797006bb73c/dlls/ntdll/directory.c#L1167
  //   Return 0x65735546 for statfs.f_type and pretend a file named .ciopfs exists
  // TODO: If we're being called by lstat(), send cached value if possible instead of
  //   falling through to another stat after this.
  // TODO: readlink can be used to determine if a file exists, maybe make file_exists_cache
  // TODO: Possibly redirect /Data/Data to /Data
  // TODO: Don't load the entire read mappings at once. Load them as they're needed
  // FIXME: inotify R events will overwrite the current file, even if they're
  //   from a folder that's much lower in the load order

  if (false) {
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    auto it = read_mappings.find(path_lower);
    if (it != read_mappings.end()) {
      std::cout << "READM: " << it->second << std::endl;fflush(stdout);

      {
        std::lock_guard<std::mutex> lock1(stat_cache_mutex);
        stat_cache.erase(it->second);
      }
      return it->second;
    }
  };

  std::string readmapping = find_read_mapping(path_lower);
  if (!readmapping.empty()) {
    //std::cout << "READM: " << readmapping << std::endl;fflush(stdout);
    {
      std::lock_guard<std::mutex> lock1(stat_cache_mutex);
      stat_cache.erase(readmapping);
    }
    return readmapping;
  }

  {
    // Not needed anymore?
    if (false) {
      std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
      auto reverse_it = winevfs_reverse_folder_mappings.find(path_lower);
      if (reverse_it != winevfs_reverse_folder_mappings.end()) {
        //printf("Found reverse: %s -> %s\n", path_lower.c_str(), reverse_it->second.c_str());fflush(stdout);
        path_lower = reverse_it->second;
      }
    }

    std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
    auto it = winevfs_folder_mappings.find(path_lower);
    if (it != winevfs_folder_mappings.end()) {
      std::string win_path = winpath(path, atfd);
      //std::cout << "FOLDER WINP: " << win_path << std::endl;fflush(stdout);

      if (!fs_isdir(win_path, true))
        return it->second.folder_paths.last();//"/tmp/.winevfs/fakedir/";
      else
        return win_path;
    }
  }

  if (intent == Intent_Modify || intent == Intent_Create) {
    {
      std::lock_guard<std::mutex> lock(write_mappings_mutex);

      for (auto it = write_mappings.begin(); it != write_mappings.end(); it++) {
        if (!strncmp(path_lower.c_str(), it->first.c_str(), it->first.size())) {
          const char* rest = path.c_str() + it->first.size();
          if (rest[0] == '/')
            rest++;

          //std::cout << "WRITE: " << path << std::endl;fflush(stdout);
          std::filesystem::path newpath = std::filesystem::path(it->second) / std::filesystem::path(rest);
          //std::cout << "WRITE (new): " << newpath << std::endl;fflush(stdout);
          newpath = winpath(newpath, AT_FDCWD);
          //std::cout << "WRITE (new_win): " << newpath << std::endl;fflush(stdout);

          fs_mkdir_p(newpath.parent_path());

#ifndef SERVER_BUILD
          if (false) {
            std::string newpath_str = newpath;
            int data_size = path_str.size() + newpath_str.size() + 2;
            char* data = (char*)malloc(data_size);
            memcpy(data, path_str.c_str(), path_str.size() + 1);
            memcpy(data + path_str.size() + 1, newpath_str.c_str(), newpath_str.size() + 1);
            winevfs_send_command('R', (void*)data, data_size);
            free(data);
          }
#endif

          _add_read_entry(path, newpath);
          //std::lock_guard<std::mutex> read_lock(read_mappings_mutex);
          //read_mappings[path_lower] = newpath;
          //std::cout << it->second << std::endl;
          //std::cout << path_lower << std::endl;
          //std::cout << newpath << std::endl;
          return newpath;
        }
      }
    };
  }

  return winpath(in, atfd);
}

std::string winevfs_get_path(std::filesystem::path in, Intent intent, int atfd) {
  winevfs_init(true);
  std::string path = winevfs_get_path_inner(in, intent, atfd);

  if (intent == Intent_Delete) {
    winevfs_delete_file(path);
#ifndef SERVER_BUILD
    //winevfs_send_command('D', (void*)path.c_str(), path.size() + 1);
#endif
  }

  return path;
}

const char* winevfs_get_path(const char* in, Intent intent, int atfd) {
  std::string retval = winevfs_get_path(std::filesystem::path(in), intent, atfd);
  char* ret;
  if (true) {
    ret = (char*)malloc(retval.size() + 1);
    strcpy(ret, retval.c_str());
  } else {
    ret = (char*)malloc(strlen(in) + 1);
    strcpy(ret, in);
  }
  return ret;
}
