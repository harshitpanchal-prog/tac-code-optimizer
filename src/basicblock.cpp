#include "basicblock.h"
#include <set>
#include <map>

using namespace std;

vector<BasicBlock> formBasicBlocks(vector<Instruction>& code) {

    set<int> leaders;
    leaders.insert(0); // first instruction

    // Step 1: find leaders
    for (int i = 0; i < (int)code.size(); i++) {

        // if label exists → leader
        // Guard: result must be non-empty before calling .back()
        if (!code[i].result.empty() && code[i].result.back() == ':') {
            leaders.insert(i);
        }

        // if jump → next is leader
        if (code[i].op == "goto") {
            if (i + 1 < (int)code.size())
                leaders.insert(i + 1);
        }
    }

    vector<int> leaderList(leaders.begin(), leaders.end());

    vector<BasicBlock> blocks;

    // Step 2: create blocks
    for (int i = 0; i < (int)leaderList.size(); i++) {
        BasicBlock block;
        block.id = i;

        int start = leaderList[i];
        int end = (i + 1 < (int)leaderList.size()) ? leaderList[i + 1] : (int)code.size();

        for (int j = start; j < end; j++) {
            block.instructions.push_back(code[j]);
        }

        blocks.push_back(block);
    }

    return blocks;
}