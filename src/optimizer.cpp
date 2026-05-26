// ============================ optimizer.cpp ============================

#include <unordered_map>
#include <map>
#include <set>

#include "optimizer.h"
#include "utils.h"

using namespace std;

// ======================================================
// Check relational operators
// ======================================================
bool isRelOp(const string& op)
{
    return (
        op == "<"  ||
        op == ">"  ||
        op == "<=" ||
        op == ">=" ||
        op == "==" ||
        op == "!="
    );
}

// ======================================================
// CONSTANT FOLDING
// ======================================================
void constantFolding(vector<Instruction>& code)
{
    for (auto &i : code)
    {
        if (
            i.op != "" &&
            isNumber(i.op1) &&
            isNumber(i.op2)
        )
        {
            int a = stoi(i.op1);
            int b = stoi(i.op2);

            int res = 0;

            // arithmetic
            if (i.op == "+")
                res = a + b;

            else if (i.op == "-")
                res = a - b;

            else if (i.op == "*")
                res = a * b;

            else if (i.op == "/")
            {
                if (b == 0)
                    continue;

                res = a / b;
            }

            // relational
            else if (i.op == "<")
                res = (a < b);

            else if (i.op == ">")
                res = (a > b);

            else if (i.op == "<=")
                res = (a <= b);

            else if (i.op == ">=")
                res = (a >= b);

            else if (i.op == "==")
                res = (a == b);

            else if (i.op == "!=")
                res = (a != b);

            else
            {
                continue;
            }

            i.op1 = to_string(res);

            i.op = "";
            i.op2 = "";
        }
    }
}

// ======================================================
// CONSTANT PROPAGATION
// ======================================================
void constantPropagation(vector<Instruction>& code)
{
    unordered_map<string, string> table;

    for (auto &i : code)
    {
        // NEVER propagate into jumps
        if (
            i.op == "goto" ||
            i.op == "ifgoto"
        )
        {
            continue;
        }

        // replace operands safely
        if (
            table.count(i.op1) &&
            !isRelOp(i.op)
        )
        {
            i.op1 = table[i.op1];
        }

        if (
            table.count(i.op2) &&
            !isRelOp(i.op)
        )
        {
            i.op2 = table[i.op2];
        }

        // store constants
        if (
            i.op == "" &&
            isNumber(i.op1)
        )
        {
            table[i.result] = i.op1;
        }
        else
        {
            table.erase(i.result);
        }
    }
}

// ======================================================
// ALGEBRAIC SIMPLIFICATION
// ======================================================
void algebraicSimplification(
    vector<Instruction>& code)
{
    for (auto &i : code)
    {
        // x + 0 = x
        if (i.op == "+" &&
            i.op2 == "0")
        {
            i.op = "";
            i.op2 = "";
        }

        // 0 + x = x
        else if (i.op == "+" &&
                 i.op1 == "0")
        {
            i.op1 = i.op2;
            i.op = "";
            i.op2 = "";
        }

        // x * 1 = x
        else if (i.op == "*" &&
                 i.op2 == "1")
        {
            i.op = "";
            i.op2 = "";
        }

        // 1 * x = x
        else if (i.op == "*" &&
                 i.op1 == "1")
        {
            i.op1 = i.op2;
            i.op = "";
            i.op2 = "";
        }

        // x * 0 = 0
        else if (
            i.op == "*" &&
            (i.op1 == "0" ||
             i.op2 == "0")
        )
        {
            i.op1 = "0";
            i.op = "";
            i.op2 = "";
        }
    }
}

// ======================================================
// COMMON SUBEXPRESSION ELIMINATION
// ======================================================
void CSE(vector<Instruction>& code)
{
    map<string, string> expr;

    for (auto &i : code)
    {
        if (
            i.op != "" &&
            i.op != "goto" &&
            i.op != "ifgoto"
        )
        {
            string key =
                i.op1 + "|" +
                i.op + "|" +
                i.op2;

            if (expr.count(key))
            {
                i.op1 = expr[key];
                i.op = "";
                i.op2 = "";
            }
            else
            {
                expr[key] = i.result;
            }
        }
    }
}

// ======================================================
// DEAD CODE ELIMINATION
// ======================================================
void deadCodeElimination(vector<Instruction>& code)
{
    set<string> live;

    // final output variable
    live.insert("result");

    vector<bool> keep(code.size(), false);

    // backward traversal
    for (int i = (int)code.size() - 1; i >= 0; i--)
    {
        Instruction& instr = code[i];

        bool isLabel =
        (
            !instr.result.empty() &&
            instr.result.back() == ':'
        );

        bool isJump =
        (
            instr.op == "goto" ||
            instr.op == "ifgoto"
        );

        // always keep labels/jumps
        if (isLabel || isJump)
        {
            keep[i] = true;

            if (
                !instr.op1.empty() &&
                !isNumber(instr.op1)
            )
            {
                live.insert(instr.op1);
            }

            if (
                !instr.op2.empty() &&
                !isNumber(instr.op2)
            )
            {
                live.insert(instr.op2);
            }

            continue;
        }

        // keep if live
        if (
            live.count(instr.result) ||
            !isTemporary(instr.result)
        )
        {
            keep[i] = true;

            live.erase(instr.result);

            if (
                !instr.op1.empty() &&
                !isNumber(instr.op1)
            )
            {
                live.insert(instr.op1);
            }

            if (
                !instr.op2.empty() &&
                !isNumber(instr.op2)
            )
            {
                live.insert(instr.op2);
            }
        }
    }

    vector<Instruction> newCode;

    for (int i = 0; i < (int)code.size(); i++)
    {
        if (keep[i])
        {
            newCode.push_back(code[i]);
        }
    }

    code = newCode;
}