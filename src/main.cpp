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
    // ── INPUT SOURCE ─────────────────────────────────────
    istream* input = &cin;

    ifstream fileStream;

    // Read from file if provided
    if (argc > 1)
    {
        fileStream.open(argv[1]);

        if (!fileStream.is_open())
        {
            cerr << "Error opening file.\n";
            return 1;
        }

        input = &fileStream;
    }

    // ── READ TAC CODE ───────────────────────────────────
    vector<Instruction> code;

    string line;

    while (getline(*input, line))
    {
        if (!line.empty())
        {
            code.push_back(parse(line));
        }
    }

    // ── ORIGINAL CODE ───────────────────────────────────
    cout << "----- ORIGINAL CODE -----\n";

    printCode(code);

    // ── BASIC BLOCKS ────────────────────────────────────
    vector<BasicBlock> blocks =
        formBasicBlocks(code);

    cout << "\n----- BASIC BLOCKS -----\n";

    for (int i = 0; i < (int)blocks.size(); i++)
    {
        cout << "Block "
             << blocks[i].id
             << ":\n";

        printCode(blocks[i].instructions);

        cout << endl;
    }

    // ── CFG ─────────────────────────────────────────────
    map<int, vector<int>> cfg =
        buildCFG(blocks);

    cout << "----- CFG -----\n";

    for (auto &p : cfg)
    {
        int src = p.first;

        if (blocks[src].instructions.empty())
            continue;

        const Instruction& lastInstr =
            blocks[src].instructions.back();

        bool hasGoto =
        (
            lastInstr.op == "goto" ||
            lastInstr.op == "ifgoto"
        );

        for (int j = 0;
             j < (int)p.second.size();
             j++)
        {
            int dst = p.second[j];

            string edgeType;

            if (hasGoto && j == 0)
            {
                edgeType = "[jump]";
            }
            else
            {
                edgeType = "[fall]";
            }

            cout << "Block "
                 << src
                 << " --"
                 << edgeType
                 << "--> Block "
                 << dst
                 << "\n";
        }

        // exit block
        if (p.second.empty())
        {
            cout << "Block "
                 << src
                 << " --> (exit)\n";
        }
    }

    // ── OPTIMIZATION ────────────────────────────────────
    cout << "\n----- OPTIMIZED BLOCKS -----\n";

    for (int i = 0; i < (int)blocks.size(); i++)
    {
        constantFolding(
            blocks[i].instructions
        );

        algebraicSimplification(
            blocks[i].instructions
        );

        constantPropagation(
            blocks[i].instructions
        );

        CSE(
            blocks[i].instructions
        );

        deadCodeElimination(
            blocks[i].instructions
        );
    }

    // ── PRINT OPTIMIZED BLOCKS ──────────────────────────
    for (int i = 0; i < (int)blocks.size(); i++)
    {
        cout << "Block "
             << blocks[i].id
             << ":\n";

        printCode(blocks[i].instructions);

        cout << "\n";
    }

    // ── SAVE OUTPUT FILE ────────────────────────────────
    ofstream outFile("output.txt");

    if (outFile.is_open())
    {
        outFile
            << "----- OPTIMIZED CODE -----\n";

        for (int i = 0;
             i < (int)blocks.size();
             i++)
        {
            outFile
                << "Block "
                << blocks[i].id
                << ":\n";

            printCode(
                blocks[i].instructions,
                outFile
            );

            outFile << "\n";
        }

        outFile.close();
    }
    else
    {
        cerr << "Could not write output.txt\n";
    }

    return 0;
}