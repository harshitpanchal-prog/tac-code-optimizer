#include "cfg.h"
#include "basicblock.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

// ── Helpers ──────────────────────────────────────────────────────────────────

// Strip trailing ':' from a label token (e.g. "L1:" → "L1")
static string stripColon(const string& s) {
    if (!s.empty() && s.back() == ':')
        return s.substr(0, s.size() - 1);
    return s;
}

// A goto is unconditional when there is no condition operand,
// i.e. op1 is empty or the placeholder "_".
static bool isUnconditionalGoto(const Instruction& instr) {
    return instr.op == "goto" &&
           (instr.op1.empty() || instr.op1 == "_");
}

// ── Build CFG ────────────────────────────────────────────────────────────────
//
// Supported TAC goto forms (op field == "goto"):
//
//   Conditional:    result = cond goto L1
//                   → adds edge to L1's block AND fallthrough to next block
//
//   Unconditional:  _ = _ goto L1   (op1 empty or "_")
//                   → adds edge to L1's block ONLY (no fallthrough)
//
// All other blocks get a single sequential fallthrough edge.
//
map<int, vector<int>> buildCFG(vector<BasicBlock>& blocks)
{
    map<int, vector<int>> cfg;

    // ── Step 1: Build label → block ID map ───────────────────────────────────
    // A block that starts with a label has its first instruction's `result`
    // field ending in ':' (set by the leader detection in basicblock.cpp).
    map<string, int> labelToBlock;

    for (int i = 0; i < (int)blocks.size(); i++) {
        if (blocks[i].instructions.empty()) continue;

        const string& first = blocks[i].instructions[0].result;
        if (!first.empty() && first.back() == ':') {
            labelToBlock[stripColon(first)] = i;
        }
    }

    // ── Step 2: Build edges ───────────────────────────────────────────────────
    for (int i = 0; i < (int)blocks.size(); i++) {
        if (blocks[i].instructions.empty()) continue;

        const Instruction& last = blocks[i].instructions.back();

        if (last.op == "goto") {
            // Resolve the jump target label → block ID
            string target = last.op2;
            if (labelToBlock.count(target)) {
                cfg[i].push_back(labelToBlock[target]);
            }

            // Conditional goto → also add fallthrough edge to the next block
            if (!isUnconditionalGoto(last) && i + 1 < (int)blocks.size()) {
                cfg[i].push_back(i + 1);
            }
            // Unconditional goto → no fallthrough; control never reaches i+1

        } else {
            // Normal (non-jump) block → sequential fallthrough only
            if (i + 1 < (int)blocks.size()) {
                cfg[i].push_back(i + 1);
            }
        }
    }

    return cfg;
}