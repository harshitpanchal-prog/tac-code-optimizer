#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "parser.h"

using namespace std;

void constantFolding(vector<Instruction>& code);
void constantPropagation(vector<Instruction>& code);
void algebraicSimplification(vector<Instruction>& code);
void CSE(vector<Instruction>& code);
void deadCodeElimination(vector<Instruction>& code);

#endif