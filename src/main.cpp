#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "parser.h"
#include "optimizer.h"
#include "basicblock.h"
#include "cfg.h"

using namespace std;

int main(int argc, char* argv[])
{
    // ── Input source: file argument OR stdin ──────────────────────────────
    // CLI usage:  ./optimizer input.txt    (reads from file)
    // Web usage:  ./optimizer              (reads from stdin — piped by server)
    //             echo "t1 = 5 + 3" | ./optimizer
    istream* input = &cin;
    ifstream fileStream;

    if (argc > 1)
    {
        fileStream.open(argv[1]);

        if (!fileStream.is_open())
        {
            cerr << "Error: could not open \"" << argv[1] << "\" — file missing or unreadable.\n";
            return 1;
        }

        input = &fileStream;
    }

    vector<Instruction> code;

    string line;

    while (getline(*input, line))
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

    for (int i = 0; i < (int)blocks.size(); i++)
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
        int src = p.first;
        const Instruction& lastInstr = blocks[src].instructions.back();
        bool hasGoto = (lastInstr.op == "goto");

        for (int j = 0; j < (int)p.second.size(); j++)
        {
            int dst = p.second[j];

            // First successor of a goto block is the jump target;
            // any additional successor is the fallthrough.
            string edgeType = (hasGoto && j == 0) ? "[jump]" : "[fall]";

            cout << "Block " << src << " --" << edgeType << "--> Block " << dst << "\n";
        }

        // Sink block (no successors)
        if (p.second.empty())
            cout << "Block " << src << " --> (exit)\n";
    }

    // STEP 3: Optimization

    cout << "\n----- OPTIMIZED BLOCKS -----\n";
    for (int i = 0; i < (int)blocks.size(); i++)
    {
        constantFolding(blocks[i].instructions);

        algebraicSimplification(blocks[i].instructions);

        constantPropagation(blocks[i].instructions);

        CSE(blocks[i].instructions);

        deadCodeElimination(blocks[i].instructions);
    }

    // ── Write optimized output to stdout (captured by server.js) ─────────
    for (int i = 0; i < (int)blocks.size(); i++)
    {
        cout << "Block " << blocks[i].id << ":\n";

        printCode(blocks[i].instructions);

        cout << "\n";
    }

    // ── Also persist to output.txt ────────────────────────────────────────
    ofstream outFile("output.txt");

    if (outFile.is_open())
    {
        outFile << "----- OPTIMIZED CODE -----\n";

        for (int i = 0; i < (int)blocks.size(); i++)
        {
            outFile << "Block " << blocks[i].id << ":\n";

            printCode(blocks[i].instructions, outFile);

            outFile << "\n";
        }

        outFile.close();
    }
    else
    {
        cerr << "Warning: could not open output.txt for writing.\n";
    }

    return 0;
}