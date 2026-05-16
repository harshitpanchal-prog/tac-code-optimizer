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

            if      (i.op == "+") res = a + b;
            else if (i.op == "-") res = a - b;
            else if (i.op == "*") res = a * b;
            else if (i.op == "/") {
                // Division by zero is undefined behaviour — leave instruction as-is
                if (b == 0) continue;
                res = a / b;
            }
            else continue; // unknown operator — don't fold

            i.op1 = to_string(res);
            i.op  = "";
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
//
// Key uses '|' delimiters so that expressions with different operand/operator
// splits can never produce the same key string.
// e.g.  op1="ab", op="+", op2="c"  →  "ab|+|c"
//       op1="a",  op="b+", op2="c" →  "a|b+|c"   (different — no false match)
//
void CSE(vector<Instruction>& code) {
    map<string, string> expr;

    for (auto &i : code) {
        if (i.op != "") {
            string key = i.op1 + "|" + i.op + "|" + i.op2;

            if (expr.count(key)) {
                i.op1 = expr[key];
                i.op  = "";
                i.op2 = "";
            } else {
                expr[key] = i.result;
            }
        }
    }
}

// ---------------- DEAD CODE ELIMINATION ----------------
//
// Algorithm: Iterative Backward Liveness Analysis
//
//   Liveness rule:
//     A variable v is LIVE before instruction I if:
//       - v is used by I, OR
//       - v is live after I and I does not define v.
//
//   Conservative live-out seed:
//     Non-temporary variables (user-defined names like x, y, a, b) are assumed
//     live at the exit of every block — we have no inter-block info here.
//     Temporaries (t1, t2, …) are assumed dead at block exit; they must
//     be proven live by being used in a subsequent instruction.
//
//   The outer while-loop repeats until no instruction is removed (fixed point).
//   This is required because removing one dead instruction may expose another.
//
void deadCodeElimination(vector<Instruction>& code) {
    bool changed = true;

    while (changed) {
        changed = false;

        // ── Seed: non-temporary results are always live-out ──────────────
        set<string> live;
        for (const auto& instr : code) {
            // Labels and jump instructions are control-flow: always keep
            bool isLabel = (!instr.result.empty() && instr.result.back() == ':');
            if (!isLabel && !isTemporary(instr.result) && !instr.result.empty())
                live.insert(instr.result);
        }

        // ── Backward pass ─────────────────────────────────────────────────
        vector<bool> keep(code.size(), false);

        for (int i = (int)code.size() - 1; i >= 0; i--) {
            const auto& instr = code[i];

            bool isLabel = (!instr.result.empty() && instr.result.back() == ':');
            bool isJump  = (instr.op == "goto");

            // Keep if: label, jump, or result is live
            if (isLabel || isJump || live.count(instr.result)) {
                keep[i] = true;

                // This instruction "satisfies" the use of result —
                // remove it from live (it's now defined here, not needed above)
                live.erase(instr.result);

                // Its operands are now required — mark them live
                if (!instr.op1.empty() && !isNumber(instr.op1) && instr.op1 != "_")
                    live.insert(instr.op1);
                if (!instr.op2.empty() && !isNumber(instr.op2) && instr.op2 != "_")
                    live.insert(instr.op2);
            }
            // else: result is dead — instruction is silently dropped
        }

        // ── Rebuild code, flag convergence ───────────────────────────────
        vector<Instruction> newCode;
        for (int i = 0; i < (int)code.size(); i++) {
            if (keep[i])
                newCode.push_back(code[i]);
            else
                changed = true;  // at least one instruction removed → re-run
        }

        code = newCode;
    }
}