#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "parser.h"
#include "optimizer.h"
#include "basicblock.h"
#include "cfg.h"

using namespace std;

int main()
{
    ifstream file("input.txt");

    vector<Instruction> code;

    string line;

    while (getline(file, line))
    {
        if (!line.empty())
        {
            code.push_back(parse(line));
        }
    }

    cout << "----- ORIGINAL CODE -----\n";

    printCode(code);

    // STEP 1: Basic Blocks
    vector<BasicBlock> blocks = formBasicBlocks(code);

    cout << "\n----- BASIC BLOCKS -----\n";

    for (int i = 0; i < blocks.size(); i++)
    {
        cout << "Block " << blocks[i].id << ":\n";

        printCode(blocks[i].instructions);

        cout << endl;
    }

    // STEP 2: CFG
    std::map<int, std::vector<int>> cfg = buildCFG(blocks);
    cout << "----- CFG -----\n";

    for (auto &p : cfg)
    {
        cout << "Block " << p.first << " -> ";

        for (auto x : p.second)
        {
            cout << x << " ";
        }

        cout << endl;
    }

    // STEP 3: Optimization
    // FIX THIS AFTER SENDING optimizer.h

    cout << "\n----- OPTIMIZED BLOCKS -----\n";
    for (int i = 0; i < blocks.size(); i++)
    {
        constantFolding(blocks[i].instructions);

        algebraicSimplification(blocks[i].instructions);

        constantPropagation(blocks[i].instructions);

        CSE(blocks[i].instructions);

        deadCodeElimination(blocks[i].instructions);
    }
    
    for (int i = 0; i < blocks.size(); i++)
    {
        cout << "Block " << blocks[i].id << ":\n";
    
        printCode(blocks[i].instructions);
    
        cout << endl;
    }

    return 0;
}