
#ifndef BITSTREAM_HH_
#define BITSTREAM_HH_

// Convert a tree back into a bitstream --- i.e., invert derive...

// We model the behaviour of Derive.  State describes what goes on inside that
// function.

#include "parse.hh"

#include <vector>

typedef std::vector <Tree> Context;
typedef std::vector<bool> Bits;

void Generate (Bits & bits,
               const Context & context,
               Tree term,
               Tree & type);

#endif
