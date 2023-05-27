#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <fstream>
#include <map>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/detail/sha1.hpp>

// boost required
namespace std 
{
  template <typename T>
  ostream& operator<<(ostream &os, const vector<T> &vec) {
    for (auto it { vec.cbegin() }; it != vec.cend(); ++it) {
      if (it != vec.cbegin()) os << ' ';
      os << *it;
    }
    return os;
  }
}

namespace bayan {

// hash types
enum HashAlgorithmType
{
	CRC32,
	MD5,
    SHA1
};

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

// hasher class interface
class IHasher 
{
public:
    virtual std::string getDigest(const char* buf, size_t buf_size) = 0;
};

// crc32 hasher
class Crc32 : public IHasher 
{
public:
    std::string getDigest(const char* buf, size_t buf_size) override {
        boost::crc_32_type crc_hash;
        crc_hash.process_bytes(buf, buf_size);
        return std::to_string(crc_hash.checksum());
    };

};

// md5 hasher
class Md5 : public IHasher 
{
public:
    std::string getDigest(const char* buf, size_t buf_size) override {
        boost::uuids::detail::md5 md5_hash;
        md5_hash.process_bytes(buf, buf_size);
        boost::uuids::detail::md5::digest_type digest;
        md5_hash.get_digest(digest);
        return std::string(reinterpret_cast<char*>(&digest));
    };

};

// sha1 hasher
class Sha1 : public IHasher 
{
public:
    std::string getDigest(const char* buf, size_t buf_size) override {
        boost::uuids::detail::sha1 sha1_hash;
        sha1_hash.process_bytes(buf, buf_size);
        boost::uuids::detail::sha1::digest_type digest;
        sha1_hash.get_digest(digest);
        return std::string(reinterpret_cast<char*>(&digest));
    };

};


class PerepareTasks
{
public:
    static std::map<size_t, std::vector<FileInfo>> sortFilesBySize(std::vector<FileInfo>& files)
    {
        std::map<size_t, std::vector<FileInfo>> sorted_files;
        for(auto file : files)
        {
            auto iterator_sorted_files = sorted_files.find(file.file_size);
            if (iterator_sorted_files == sorted_files.end())
            {
                sorted_files.insert(std::pair<size_t, std::vector<FileInfo>>(file.file_size, std::vector<FileInfo>{ file }));
            }
            else
            {
                iterator_sorted_files->second.push_back(file);
            }
        }

        return sorted_files;
    };
};


class DublicatesFinder
{
    static void printDublicates(const std::vector<std::string>& dublicates)
    {
        for (auto dublicat: dublicates)
        {
            std::cout << dublicat << std::endl;
        }
        std::cout << std::endl;
    }

    static bool compareFiles(const FileInfo& file1, const FileInfo& file2, std::shared_ptr<IHasher> Hasher, size_t block_size)
    {
        std::ifstream input1(file1.path, std::ios::binary);
        std::ifstream input2(file2.path, std::ios::binary); 

        // block check
        std::string chunk1, chunk2;
        while (!input1.eof() && !input2.eof())
        {
            uint8_t symbol;
                
            input1.read((char*)&symbol, 1);
            chunk1.push_back(symbol);

            input2.read((char*)&symbol, 1);
            chunk2.push_back(symbol);

            if (chunk1.size() == block_size && block_size == chunk2.size())
            {
                if (
                    Hasher->getDigest(chunk1.c_str(), block_size) != 
                    Hasher->getDigest(chunk2.c_str(), block_size)
                )
                {
                    input1.close();
                    input2.close();
                    return false;
                }
                else
                {
                    chunk1.clear();
                    chunk2.clear();
                }
            }
        }

        // last check
        if (!chunk1.empty() && !chunk2.empty())
        {
            if (
                Hasher->getDigest(chunk1.c_str(), chunk1.size()) != 
                Hasher->getDigest(chunk2.c_str(), chunk2.size())
            )
            {
                input1.close();
                input2.close();
                return false;
            }
        }

        // all passed return true
        input1.close();
        input2.close();
        return true;
    }

public:
    static void getDublicates(std::vector<FileInfo>& files, PBayanSettings settings)
    {
        std::shared_ptr<IHasher> Hasher;
        switch(settings->hash_type)
        {
            case HashAlgorithmType::CRC32: Hasher = std::make_shared<Crc32>(); break;
            case HashAlgorithmType::MD5: Hasher = std::make_shared<Md5>(); break;
            case HashAlgorithmType::SHA1: Hasher = std::make_shared<Sha1>(); break;
            default: break;
        }

        for (auto iter_file1 = files.begin(); iter_file1 != files.end(); ++iter_file1)
        {
            std::vector<std::string> dublicate_files_paths;
            for(auto iter_file2 = iter_file1 + 1; iter_file2 != files.end();)
            {
                if(compareFiles(*iter_file1, *iter_file2, Hasher, settings->block_size))
                {
                    dublicate_files_paths.push_back(iter_file2->path);
                    iter_file2 = files.erase(iter_file2);
                }
                else
                {
                    ++iter_file2;
                }
            }
            if(!dublicate_files_paths.empty())
            {
                dublicate_files_paths.push_back(iter_file1->path);
                printDublicates(dublicate_files_paths);
            }
        } 
    }
};

class DirectoryHandler 
{
    static bool matchPatterns(const std::string& filename, PBayanSettings settings) {
        if (settings->patterns.empty()) return true;
        for (auto const &pattern: settings->patterns)
        {
            if (std::regex_match(filename, std::regex(pattern))) return true;
        }
        return false;
    };


    static std::vector<FileInfo> getDirectoryFiles(const std::string& dir_path, PBayanSettings settings) 
    {
        std::vector<FileInfo> files;
        for (auto entry = std::filesystem::recursive_directory_iterator(std::filesystem::path(dir_path)); entry != std::filesystem::recursive_directory_iterator(); ++entry)
        {
            if (entry->is_regular_file() && !entry->is_symlink())
            {
                if (matchPatterns(entry->path().filename().string(), settings) && entry->file_size() >= settings->min_file_size) 
                {
                    files.push_back
                    (
                        {
                            entry->path().string(),
                            entry->file_size()
                        }
                    );
                }
            }
            else if (entry->is_directory())
            {
                if (settings->level == true)
                {
                    for (auto exclusion_path : settings->exclude_paths)
                    {
                        if(entry->path().string() == exclusion_path) 
                        {
                            entry.disable_recursion_pending();
                            break;
                        }
                    } 
                }
                else
                {
                    entry.disable_recursion_pending();
                }
            }
        }
        return files;
    };

public:
    static std::vector<FileInfo> getAllFiles(PBayanSettings settings)
    {
        std::vector<FileInfo> all_paths;
        for(auto directory_path: settings->directories_paths)
        {
            auto paths = getDirectoryFiles(directory_path, settings);
            all_paths.insert(all_paths.end(), paths.begin(), paths.end());
        }
        return all_paths;
    }
};

class Bayan 
{
    BayanSettings options; 

    bool ParsingCLArgs(int argc, char* argv[]) 
    {
        try {
            boost::program_options::positional_options_description positional_description { };
            positional_description.add("targets", -1);

            boost::program_options::options_description hidden_description { "Hidden options" };
            hidden_description.add_options()
            ("targets",
                boost::program_options::value<std::vector<std::string>>()->
                multitoken()->value_name("PATH")->default_value({ "./" }),
                "targets directories");

            boost::program_options::options_description named_description {
                "Allowed options"
            };

            named_description.add_options()
            ("help,h", "give this help list")
            ("exclude,e",
                boost::program_options::value<std::vector<std::string>>()->
                multitoken()->value_name("DIR")->default_value({ }),
                "list of directories to exclude, may be passed multiple times")
            ("level,l",
                boost::program_options::value<bool>()->value_name("BOOL")->default_value(false),
                "scan level, 1 - with all subdirectories, 0 - only target directories")
            ("pattern,p",
                boost::program_options::value<std::vector<std::string>>()->
                multitoken()->value_name("REGEX")->default_value(std::vector<std::string>{ }),
                "only matched files will be processed when given, may be passed multiple times")
            ("min-file,s",
                boost::program_options::value<size_t>()->value_name("BYTES")->default_value(1),
                "lower limit of files in bytes to compare")
            ("block-size,b",
                boost::program_options::value<size_t>()->value_name("BYTES")->default_value(5),
                "default block size in bytes to process")
            ("hash-func,f",
                boost::program_options::value<std::string>()->value_name("NAME")->default_value(std::string("crc32")),
                "hash function to use, crc32, md5, sha1 is valid");

            boost::program_options::options_description cmdline_description { };
            cmdline_description.add(hidden_description).add(named_description);

            boost::program_options::variables_map variables;
            boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv)
                .options(cmdline_description)
                .positional(positional_description)
                .run(),
            variables);
            boost::program_options::notify(variables);

            if (variables.count("help")) {
                std::cout << named_description << std::endl;
                return false;
            }
       
            options.directories_paths = variables["targets"].as<std::vector<std::string>>();         
            auto hash_func = variables["hash-func"].as<std::string>();
            boost::algorithm::to_lower(hash_func);
            if (hash_func == "crc32") options.hash_type = HashAlgorithmType::CRC32;
            else if (hash_func == "md5") options.hash_type = HashAlgorithmType::MD5;
            else if (hash_func == "sha1") options.hash_type = HashAlgorithmType::SHA1;
            options.exclude_paths = variables["exclude"].as<std::vector<std::string>>();           
            options.level = variables["level"].as<bool>();
            options.patterns = variables["pattern"].as<std::vector<std::string>>();
            options.min_file_size = variables["min-file"].as<size_t>();
            options.block_size = variables["block-size"].as<size_t>();

            variables.clear();
            return true;
        }
        catch (boost::program_options::error const &e) {
            std::cerr << "Options error: " << e.what() << std::endl;
            return false;
        }
    };

public:
    void getDublicates(int argc, char ** argv)
    {
        if (ParsingCLArgs(argc, argv))
        {
            auto files = DirectoryHandler::getAllFiles(&options);
            if (!files.empty())
            {
                auto tasks = PerepareTasks::sortFilesBySize(files);
                for (auto task: tasks)
                {
                    DublicatesFinder::getDublicates(task.second, &options);
                }
            }
            else
            {
                std::cout << "Files not found!" << std::endl;
            }
        }
    }
};

};