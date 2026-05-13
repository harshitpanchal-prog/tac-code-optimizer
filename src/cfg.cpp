#include "cfg.h"
#include<vector>
#include<map>
#include "basicblock.h"
using namespace std;

map<int, vector<int>> buildCFG(vector<BasicBlock>& blocks)
{
    map<int, vector<int>> cfg;

    for (int i = 0; i < blocks.size(); i++)
    {
        // Sequential connection
        if (i + 1 < blocks.size())
        {
            cfg[i].push_back(i + 1);
        }
    }

    return cfg;
}