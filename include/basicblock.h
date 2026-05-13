#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include <vector>
#include <string>
#include "parser.h"

using namespace std;

struct BasicBlock {
    int id;
    vector<Instruction> instructions;
};

vector<BasicBlock> formBasicBlocks(vector<Instruction>& code);

#endif