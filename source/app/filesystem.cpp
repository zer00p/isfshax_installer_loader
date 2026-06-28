#include "filesystem.h"
#include "common_paths.h"
#include "gui.h"

#include <dirent.h>
#include <sys/unistd.h>
#include <sys/statvfs.h>

struct stat existStat;

bool fileExist(const std::string& path) {
    if (lstat(path.c_str(), &existStat) == 0 && S_ISREG(existStat.st_mode)) return true;
    return false;
}

bool dirExist(const std::string& path) {
    if (lstat(path.c_str(), &existStat) == 0 && S_ISDIR(existStat.st_mode)) return true;
    return false;
}

bool copyFile(const std::string& src, const std::string& dest) {
    try {
        return std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing);
    } catch (...) {
        return false;
    }
}

bool moveFile(const std::string& src, const std::string& dest) {
    if (rename(src.c_str(), dest.c_str()) == 0) return true;
    if (copyFile(src, dest)) {
        removeFile(src);
        return true;
    }
    return false;
}

bool removeFile(const std::string& path) {
    return unlink(path.c_str()) == 0;
}

bool removeDir(const std::string& path) {
    return rmdir(path.c_str()) == 0;
}

bool createDirectories(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (...) {
        return false;
    }
}

int fileOpen(const std::string& path, int flags, mode_t mode) {
    return open(path.c_str(), flags, mode);
}

FILE* fileFopen(const std::string& path, const char* mode) {
    return fopen(path.c_str(), mode);
}

DIR* dirOpen(const std::string& path) {
    return opendir(path.c_str());
}

bool isSlcPath(const std::string& path) {
    return false; // Not used
}

uint64_t getFreeSpace(const std::string& path) {
    struct statvfs s;
    if (statvfs(path.c_str(), &s) == 0) {
        return (uint64_t)s.f_bavail * s.f_frsize;
    }
    return 0;
}

uint64_t getFileSize(const std::string& path) {
    struct stat s;
    if (lstat(path.c_str(), &s) == 0) {
        return s.st_size;
    }
    return 0;
}

size_t countFiles(const std::string& path) {
    DIR* dir = dirOpen(path);
    if (!dir) return 0;
    size_t count = 0;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_type == DT_REG) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

bool deleteDirContent(const std::string& path) {
    DIR* dirHandle;
    if ((dirHandle = dirOpen(path.c_str())) == nullptr) return false;

    struct dirent *dirEntry;
    while((dirEntry = readdir(dirHandle)) != nullptr) {
        if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0) continue;

        std::string fullPath = path;
        if (fullPath.back() != '/') fullPath += "/";
        fullPath += dirEntry->d_name;

        if ((dirEntry->d_type & DT_DIR) == DT_DIR) {
            deleteDirContent(fullPath);
            removeDir(fullPath);
        } else {
            removeFile(fullPath);
        }
    }

    closedir(dirHandle);
    return true;
}

bool isDirEmpty(const std::string& path) {
    DIR* dirHandle;
    if ((dirHandle = dirOpen(path.c_str())) == nullptr) return true;

    struct dirent *dirEntry;
    while((dirEntry = readdir(dirHandle)) != nullptr) {
        if ((dirEntry->d_type & DT_DIR) == DT_DIR && (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)) continue;
        
        // An entry other than the root and parent directory was found
        closedir(dirHandle);
        return false;
    }
    
    closedir(dirHandle);
    return true;
}
