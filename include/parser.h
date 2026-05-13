#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <ostream>
using namespace std;

struct Instruction {
    string result, op1, op, op2;
};

Instruction parse(const string& line);

// Print to stdout
void printCode(const vector<Instruction>& code);

// Print to any output stream (file, stringstream, cout, etc.)
void printCode(const vector<Instruction>& code, ostream& out);

#endif