#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
using namespace std;

struct Instruction {
    string result, op1, op, op2;
};

Instruction parse(string line);
void printCode(const vector<Instruction>& code);

#endif