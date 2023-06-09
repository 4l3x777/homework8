#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "hasher.h"

// boost required
namespace std 
{
  template <typename T>
  std::ostream& operator<<(std::ostream &os, const std::vector<T> &vec) {
    for (auto it { vec.cbegin() }; it != vec.cend(); ++it) {
      if (it != vec.cbegin()) os << ' ';
      os << *it;
    }
    return os;
  }
}

namespace bayan {

// file struct
typedef struct 
{
    std::string path;
    size_t file_size;
} FileInfo, *PFileInfo;

// byan options struct
typedef struct 
{
    // work with directory params
    std::vector<std::string> directories_paths;
    std::vector<std::string> exclude_paths;
    std::vector<std::string> patterns;
    bool level;

    // work with file params
    HashAlgorithmType hash_type;
    size_t min_file_size;
    size_t block_size;
} BayanSettings, *PBayanSettings;

class PerepareTasks
{
public:
    static std::map<size_t, std::vector<FileInfo>> sortFilesBySize(std::vector<FileInfo>& files);
};

class DublicatesFinder
{
    static void printDublicates(const std::vector<std::string>& dublicates);
    static bool compareFiles(const FileInfo& file1, const FileInfo& file2, std::shared_ptr<IHasher> Hasher, size_t block_size);
public:
    static void getDublicates(std::vector<FileInfo>& files, PBayanSettings settings);
};

class DirectoryHandler 
{
    static bool matchPatterns(const std::string& filename, PBayanSettings settings);
    static std::vector<FileInfo> getDirectoryFiles(const std::string& dir_path, PBayanSettings settings);
public:
    static std::vector<FileInfo> getAllFiles(PBayanSettings settings);
};

};
