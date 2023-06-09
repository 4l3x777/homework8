#pragma once
#include "utils.h"

namespace bayan {

class Bayan 
{
    BayanSettings options; 
    bool ParsingCLArgs(int argc, char* argv[]);
public:
    void getDublicates(int argc, char ** argv);
};

};