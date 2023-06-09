#pragma once
#include <string>
#include <memory>

namespace bayan {
// hash types
enum HashAlgorithmType
{
	CRC32,
	MD5,
    SHA1
};

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
    std::string getDigest(const char* buf, size_t buf_size) override;
};

// md5 hasher
class Md5 : public IHasher 
{
public:
    std::string getDigest(const char* buf, size_t buf_size) override;
};

// sha1 hasher
class Sha1 : public IHasher 
{
public:
    std::string getDigest(const char* buf, size_t buf_size) override;
};

};

