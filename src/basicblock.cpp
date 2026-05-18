// ============================ basicblock.cpp ============================

#include "basicblock.h"

#include <set>
#include <vector>
#include <string>

using namespace std;

vector<BasicBlock> formBasicBlocks(
    vector<Instruction>& code)
{
    set<int> leaders;

    // first instruction
    leaders.insert(0);

    // find leaders
    for (int i = 0; i < (int)code.size(); i++)
    {
        // label
        if (!code[i].result.empty() &&
            code[i].result.back() == ':')
        {
            leaders.insert(i);
        }

        // jumps
        if (code[i].op == "goto" ||
            code[i].op == "ifgoto")
        {
            // next instruction is leader
            if (i + 1 < (int)code.size())
            {
                leaders.insert(i + 1);
            }
        }
    }

    vector<int> leaderList(
        leaders.begin(),
        leaders.end()
    );

    vector<BasicBlock> blocks;

    // create blocks
    for (int i = 0;
         i < (int)leaderList.size();
         i++)
    {
        BasicBlock block;

        block.id = i;

        int start = leaderList[i];

        int end;

        if (i + 1 < (int)leaderList.size())
        {
            end = leaderList[i + 1];
        }
        else
        {
            end = code.size();
        }

        for (int j = start; j < end; j++)
        {
            block.instructions.push_back(
                code[j]
            );
        }

        blocks.push_back(block);
    }

    return blocks;
}