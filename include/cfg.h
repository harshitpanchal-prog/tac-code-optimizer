#ifndef CFG_H
#define CFG_H

#include <vector>
#include <map>
#include "basicblock.h"

using namespace std;

std::map<int, std::vector<int>> buildCFG(std::vector<BasicBlock>& blocks);

#endif