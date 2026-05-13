#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "parser.h"

using namespace std;

Instruction parse(string line)
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

void printCode(const vector<Instruction>& code)
{
    for (int i = 0; i < code.size(); i++)
    {
        cout << code[i].result << " = " << code[i].op1;

        if (code[i].op != "")
        {
            cout << " " << code[i].op << " " << code[i].op2;
        }

        cout << endl;
    }
}