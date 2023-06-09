#include "utils.h"
#include <filesystem>
#include <regex>
#include <fstream>
#include <iostream>

std::map<size_t, std::vector<bayan::FileInfo>> bayan::PerepareTasks::sortFilesBySize(std::vector<FileInfo>& files)
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

void bayan::DublicatesFinder::printDublicates(const std::vector<std::string>& dublicates)
{
        for (auto dublicat: dublicates)
        {
            std::cout << dublicat << std::endl;
        }
        std::cout << std::endl;
};

bool bayan::DublicatesFinder::compareFiles(const FileInfo& file1, const FileInfo& file2, std::shared_ptr<IHasher> Hasher, size_t block_size)
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
};

void bayan::DublicatesFinder::getDublicates(std::vector<FileInfo>& files, PBayanSettings settings)
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
};

bool bayan::DirectoryHandler::matchPatterns(const std::string& filename, PBayanSettings settings) 
{
    if (settings->patterns.empty()) return true;
    for (auto const &pattern: settings->patterns)
    {
        if (std::regex_match(filename, std::regex(pattern))) return true;
    }
    return false;
};

std::vector<bayan::FileInfo> bayan::DirectoryHandler::getDirectoryFiles(const std::string& dir_path, PBayanSettings settings) 
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

std::vector<bayan::FileInfo> bayan::DirectoryHandler::getAllFiles(PBayanSettings settings)
{
    std::vector<FileInfo> all_paths;
    for(auto directory_path: settings->directories_paths)
    {
        auto paths = getDirectoryFiles(directory_path, settings);
        all_paths.insert(all_paths.end(), paths.begin(), paths.end());
    }
    return all_paths;
};