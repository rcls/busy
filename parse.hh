#ifndef PARSE_HH_
#define PARSE_HH_

// A recursive descent parser for terms.  We generate both bitstreams for input
// into Derive, and a sequent.

// Grammar:

// term := var | ( var : term ) term | [ var : term ] term
//       | term term | term > term | * | # | ( term )

// Variables are alpha-numeric. (x:A)B is a PI type, [x:A]B is a lambda.
// A>B is (x:A)B for x not free in B.  Scoping rules are like LEGO: [x]y and
// (x)y bind tightly to the right. x>y is right associative.
// Application is left associative.
// a b c = (a b) c
// a (x:b) c d = a ((x:b) (c d))
// a b>c d = a (b>(c d))

#include <string>
#include <utility>
#include <vector>

#include "tree.hh"

typedef std::string Variable;
typedef std::vector <Variable> VarList;

class CharNotFound : public std::exception
{
public:
   CharNotFound (char c, const char * input) :
      Char (c),
      Input (input)
      { }
   ~CharNotFound() throw() { }
   const char * what()
      {
         if (What.empty()) {
            ((((What = "Expected '") += Char) += "' got '") += *Input) += "'";
         }
         return What.c_str();
      }
   char Char;
   const char * Input;
   std::string What;
};

const char * ParseTerm (Tree & term,
                        const VarList & context,
                        const char * input);

const char * ParseNonArrowTerm (Tree & term,
                                const VarList & context,
                                const char * input);

const char * ParseUnappliedTerm (Tree & term,
                                 const VarList & context,
                                 const char * input);

const char * ParseVariable (std::string & var, const char * input);

const char * SkipWhite (const char * input);
const char * CheckChar (char c, const char * input);

Tree Subst (Tree main, int var, Tree replace);

Tree Lift (Tree t, int var);

Tree WeakHeadNormalise (Tree t);

Tree Normalise (Tree t);

bool NormalisedEquals (Tree a, Tree b);

#endif
