#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "parser.h"

using namespace std;

// ─────────────────────────────────────────────
// Parse TAC line
// ─────────────────────────────────────────────
Instruction parse(const string& line)
{
    Instruction inst;

    string tempLine = line;

    // remove leading spaces
    while (!tempLine.empty() && tempLine[0] == ' ')
    {
        tempLine.erase(0, 1);
    }

    // LABEL
    // Example:
    // L1:
    if (!tempLine.empty() && tempLine.back() == ':')
    {
        inst.result = tempLine;
        inst.op1 = "";
        inst.op = "";
        inst.op2 = "";

        return inst;
    }

    stringstream ss(tempLine);

    string first;
    ss >> first;

    // ─────────────────────────────────────────
    // GOTO
    // goto L1
    // ─────────────────────────────────────────
    if (first == "goto")
    {
        inst.result = "";

        inst.op = "goto";

        ss >> inst.op1;

        inst.op2 = "";

        return inst;
    }

    // ─────────────────────────────────────────
    // CONDITIONAL GOTO
    // if cond goto L1
    // ─────────────────────────────────────────
    if (first == "if")
    {
        inst.result = "";

        ss >> inst.op1;

        string gotoWord;
        ss >> gotoWord;

        ss >> inst.op2;

        inst.op = "ifgoto";

        return inst;
    }

    // ─────────────────────────────────────────
    // NORMAL TAC
    // t1 = a + b
    // ─────────────────────────────────────────

    inst.result = first;

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

// ─────────────────────────────────────────────
// Print to cout
// ─────────────────────────────────────────────
void printCode(const vector<Instruction>& code)
{
    printCode(code, cout);
}

// ─────────────────────────────────────────────
// Print to any output stream
// ─────────────────────────────────────────────
void printCode(
    const vector<Instruction>& code,
    ostream& out
)
{
    for (int i = 0; i < (int)code.size(); i++)
    {
        const Instruction& in = code[i];

        // LABEL
        if (
            !in.result.empty() &&
            in.result.back() == ':'
        )
        {
            out << in.result << endl;
        }

        // GOTO
        else if (in.op == "goto")
        {
            out << "goto "
                << in.op1
                << endl;
        }

        // CONDITIONAL GOTO
        else if (in.op == "ifgoto")
        {
            out << "if "
                << in.op1
                << " goto "
                << in.op2
                << endl;
        }

        // NORMAL TAC
        else
        {
            out << in.result
                << " = "
                << in.op1;

            if (in.op != "")
            {
                out << " "
                    << in.op
                    << " "
                    << in.op2;
            }

            out << endl;
        }
    }
}