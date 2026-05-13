#include <unordered_map>
#include <map>
#include <set>
#include "optimizer.h"
#include "utils.h"

using namespace std;

// ---------------- CONSTANT FOLDING ----------------
void constantFolding(vector<Instruction>& code) {
    for (auto &i : code) {
        if (i.op != "" && isNumber(i.op1) && isNumber(i.op2)) {
            int a = stoi(i.op1);
            int b = stoi(i.op2);
            int res = 0;

            if (i.op == "+") res = a + b;
            else if (i.op == "-") res = a - b;
            else if (i.op == "*") res = a * b;
            else if (i.op == "/") res = a / b;

            i.op1 = to_string(res);
            i.op = "";
            i.op2 = "";
        }
    }
}

// ---------------- CONSTANT PROPAGATION ----------------
void constantPropagation(vector<Instruction>& code) {
    unordered_map<string, string> table;

    for (auto &i : code) {
        if (table.count(i.op1))
            i.op1 = table[i.op1];

        if (table.count(i.op2))
            i.op2 = table[i.op2];

        if (i.op == "" && isNumber(i.op1)) {
            table[i.result] = i.op1;
        } else {
            table.erase(i.result);
        }
    }
}

// ---------------- ALGEBRAIC SIMPLIFICATION ----------------
void algebraicSimplification(vector<Instruction>& code)
{
    for (auto &i : code)
    {
        // x + 0 = x
        if (i.op == "+" && i.op2 == "0")
        {
            i.op = "";
            i.op2 = "";
        }

        // 0 + x = x
        else if (i.op == "+" && i.op1 == "0")
        {
            i.op1 = i.op2;
            i.op = "";
            i.op2 = "";
        }

        // x * 1 = x
        else if (i.op == "*" && i.op2 == "1")
        {
            i.op = "";
            i.op2 = "";
        }

        // 1 * x = x
        else if (i.op == "*" && i.op1 == "1")
        {
            i.op1 = i.op2;
            i.op = "";
            i.op2 = "";
        }

        // x * 0 = 0
        else if (i.op == "*" &&
                (i.op1 == "0" || i.op2 == "0"))
        {
            i.op1 = "0";
            i.op = "";
            i.op2 = "";
        }
    }
}
// ---------------- COMMON SUBEXPRESSION ELIMINATION ----------------
void CSE(vector<Instruction>& code) {
    map<string, string> expr;

    for (auto &i : code) {
        if (i.op != "") {
            string key = i.op1 + i.op + i.op2;

            if (expr.count(key)) {
                i.op1 = expr[key];
                i.op = "";
                i.op2 = "";
            } else {
                expr[key] = i.result;
            }
        }
    }
}

// ---------------- DEAD CODE ELIMINATION ----------------
void deadCodeElimination(vector<Instruction>& code) {
    set<string> used;

    for (auto &i : code) {
        if (i.op1 != "") used.insert(i.op1);
        if (i.op2 != "") used.insert(i.op2);
    }

    vector<Instruction> newCode;

    for (auto &i : code) {
        if (used.count(i.result) || i.result == "x" || i.result == "y") {
            newCode.push_back(i);
        }
    }

    code = newCode;
}