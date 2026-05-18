// ============================ cfg.cpp ============================

#include "cfg.h"
#include "basicblock.h"

#include <vector>
#include <map>
#include <string>
#include <cctype>

using namespace std;

static bool isNumber(const string& s)
{
    if (s.empty()) return false;

    for (char c : s)
    {
        if (!isdigit(c))
            return false;
    }

    return true;
}

static string stripColon(const string& s)
{
    if (!s.empty() && s.back() == ':')
        return s.substr(0, s.size() - 1);

    return s;
}

map<int, vector<int>> buildCFG(
    vector<BasicBlock>& blocks)
{
    map<int, vector<int>> cfg;

    // label -> block map
    map<string, int> labelMap;

    for (int i = 0; i < (int)blocks.size(); i++)
    {
        if (blocks[i].instructions.empty())
            continue;

        Instruction first =
            blocks[i].instructions[0];

        if (!first.result.empty() &&
            first.result.back() == ':')
        {
            string label =
                stripColon(first.result);

            labelMap[label] = i;
        }
    }

    // build edges
    for (int i = 0; i < (int)blocks.size(); i++)
    {
        if (blocks[i].instructions.empty())
            continue;

        Instruction last =
            blocks[i].instructions.back();

        // goto / ifgoto
        if (last.op == "goto" ||
            last.op == "ifgoto")
        {
            string target;

            // goto L1
            if (last.op == "goto")
            {
                target = last.op1;
            }

            // if cond goto L1
            else
            {
                target = last.op2;
            }

            // numeric target
            if (isNumber(target))
            {
                int blockTarget = stoi(target);

                if (blockTarget >= 0 &&
                    blockTarget < (int)blocks.size())
                {
                    cfg[i].push_back(blockTarget);
                }
            }

            // label target
            else if (labelMap.count(target))
            {
                cfg[i].push_back(
                    labelMap[target]
                );
            }

            // conditional goto -> fallthrough
            if (last.op == "ifgoto")
            {
                if (i + 1 < (int)blocks.size())
                {
                    cfg[i].push_back(i + 1);
                }
            }
        }

        // normal fallthrough
        else
        {
            if (i + 1 < (int)blocks.size())
            {
                cfg[i].push_back(i + 1);
            }
        }

        if (!cfg.count(i))
        {
            cfg[i] = {};
        }
    }

    return cfg;
}