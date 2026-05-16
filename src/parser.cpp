#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "parser.h"

using namespace std;

Instruction parse(const string& line)
{
    Instruction inst;

    stringstream ss(line);

    ss >> inst.result;

    string eq;
    ss >> eq;

    ss >> inst.op1;

    if (ss >> inst.op)
    {
        ss >> inst.op2;
    }
    else
    {
        inst.op = "";
        inst.op2 = "";
    }

    return inst;
}

// Core: write instructions to any output stream
void printCode(const vector<Instruction>& code, ostream& out)
{
    for (int i = 0; i < (int)code.size(); i++)
    {
        out << code[i].result << " = " << code[i].op1;

        if (code[i].op != "")
        {
            out << " " << code[i].op << " " << code[i].op2;
        }

        out << "\n";
    }
}

// Convenience: print to stdout
void printCode(const vector<Instruction>& code)
{
    printCode(code, cout);
}