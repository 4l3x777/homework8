#include "bayan.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

bool bayan::Bayan::ParsingCLArgs(int argc, char* argv[]) 
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

        if (variables.count("help")) 
        {
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
    catch (boost::program_options::error const &e) 
    {
        std::cerr << "Options error: " << e.what() << std::endl;
        return false;
    }
};

void bayan::Bayan::getDublicates(int argc, char ** argv)
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
};


