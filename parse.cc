
// A recursive descent parser for terms.

// Grammar:

// term := var | { var : term } term | [ var : term ] term
//       | term term | term > term | * | # | ( term )

// Variables are alpha-numeric. {x:A}B is a PI type, [x:A]B is a lambda.
// A>B is (x:A)B for x not free in B.  Scoping rules are like LEGO: [x]y and
// (x)y bind tightly to the right. x>y is right associative.
// Application is left associative.
// a b c = (a b) c
// a (x:b) c d = a ((x:b) (c d))
// a b>c d = a (b>(c d))

#include "bitstream.hh"
#include "parse.hh"

const char * ParseTerm (Tree & term,
                        const VarList & context,
                        const char * input)
{
   input = ParseNonArrowTerm (term, context, input);

   if (*input != '>') {
      return input;
   }

   ++input;

   Tree second;

   input = ParseTerm (second, context, input);

   term = Pair (0, Pair (term, Lift (second, 0)));

   return input;
}

const char * ParseNonArrowTerm (Tree & term,
                                const VarList & context,
                                const char * input)
{
   input = SkipWhite (input);

   input = ParseUnappliedTerm (term, context, input);

   while (true) {

      input = SkipWhite (input);

      if (!*input ||
          *input == ']' ||
          *input == ')' ||
          *input == '}' ||
          *input == '>') {
         return input;
      }

      Tree arg;

      input = ParseUnappliedTerm (arg, context, input);

      term = Pair (2, Pair (term, arg));
   }
}

const char * ParseUnappliedTerm (Tree & term,
                                 const VarList & context,
                                 const char * input)
{
   if (*input == '(') {
      return CheckChar (')', ParseTerm (term, context, input + 1));
   }
   if (*input == '[' || *input == '{') {
      bool isLambda = *input == '[';
      // A lambda abstraction.
      input = SkipWhite (input + 1);
      std::string var;
      input = ParseVariable (var, input);
      input = CheckChar (':', input);
      Tree argType;
      input = ParseTerm (argType, context, input);
      input = CheckChar (isLambda ? ']' : '}', input);

      VarList newVarList = context;
      newVarList.push_back (var);
      Tree bodyTerm;
      input = ParseTerm (bodyTerm, newVarList, input);
      term = Pair (isLambda, Pair (argType, bodyTerm));
      return input;
   }

   if (*input == '*') {
      term = Pair (3, 0);
      return SkipWhite (input + 1);
   }

   std::string var;
   input = ParseVariable (var, input);

   for (VarList::const_reverse_iterator i = context.rbegin();
        i != context.rend(); ++i) {
      if (*i == var) {
         term = Pair (4 + 2 * (i - context.rbegin()), 0);
         return input;
      }
   }

   std::cerr << "Warning: Free variable \"" << var << "\"\n";
   term = Pair (4 + 2 * context.size(), 0);
   return input;      
}

const char * CheckChar (char c, const char * input)
{
   input = SkipWhite (input);
   if (*input != c) {
      throw CharNotFound (c, input);
   }
   return SkipWhite (input + 1);
}

Tree Subst (Tree main, int var, Tree replace)
{
   int opcode = main.Left().ToInt();
   if (opcode == 0 || opcode == 1) {
      // PI or LAMBDA
      return Pair (main.Left(),
                   Pair (Subst (main.Right().Left(), var, replace),
                         Subst (main.Right().Right(),
                                var + 1, Lift (replace, 0))));
   }
   if (opcode == 2) {
      // Apply.
      return Pair (main.Left(),
                   Pair (Subst (main.Right().Left(), var, replace),
                         Subst (main.Right().Right(), var, replace)));
   }
   if (opcode == 3 || opcode < 4 + 2 * var) {
      return main;
   }
   if (opcode == 4 + 2 * var) {
      return replace;
   }
   else {
      // Larger variable...
      return Pair (opcode - 2, 0);
   }
}

Tree Lift (Tree t, int var)
{
   int opcode = t.Left().ToInt();
   if (opcode == 0 || opcode == 1) {
      // PI or LAMBDA
      return Pair (t.Left(),
                   Pair (Lift (t.Right().Left(), var),
                         Lift (t.Right().Right(), var + 1)));
   }
   if (opcode == 2) {
      // Apply.
      return Pair (t.Left(),
                   Pair (Lift (t.Right().Left(), var),
                         Lift (t.Right().Right(), var)));
   }
   if (opcode == 3 || opcode < 4 + 2 * var) {
      return t;
   }
   else {
      return Pair (opcode + 2, 0);
   }
}

Tree WeakHeadNormalise (Tree t)
{
   while (t.Left() == 2) {
      Tree left = WeakHeadNormalise (t.Right().Left());
      if (left.Left() != 1) {
         // We didn't get a LAMBDA, so we're done.
         return Pair (2, Pair (left, t.Right().Right()));
      }
      t = Subst (left.Right().Right(), 0, t.Right().Right());
   }
   return t;
}

Tree Normalise (Tree t)
{
   while (t.Left() == 2) {
      Tree left = Normalise (t.Right().Left());
      if (left.Left() != 1) { // not a lambda.
         break;
      }
      t = Subst (left.Right().Right(), 0, Normalise (t.Right().Right()));
   }
   if (t.Left().ToInt() < 3) {
      return Pair (t.Left(), Pair (Normalise (t.Right().Left()),
                                   Normalise (t.Right().Right())));
   }
   else {
      return t;
   }
}

bool NormalisedEquals (Tree a, Tree b)
{
   if (a == b) {
      return true;
   }
   a = WeakHeadNormalise (a);
   b = WeakHeadNormalise (b);

   if (a.Left() != b.Left()) {
      return false;
   }

   int opcode = a.Left().ToInt();

   if (opcode < 3) {
      // PI, LAMBDA, APPLY.
      return NormalisedEquals (a.Right().Left(), b.Right().Left())
         &&  NormalisedEquals (a.Right().Right(), b.Right().Right());
   }
   // Variable or STAR or BOX.
   return opcode != 3 || a.Right() == b.Right();
}

const char * SkipWhite (const char * input)
{
   while (isspace (*input)) {
      ++input;
   }
   return input;
}

const char * ParseVariable (std::string & var, const char * input)
{
   if (!isalnum (*input)) {
      throw CharNotFound ('a', input);
   }
   while (isalnum (*input)) {
      var += *input++;
   }
   return input;
}

int main (int argc, const char *const * argv)
{
   Tree term;
   try {
      const char * input = ParseTerm (term, VarList(), argv[1]);
      if (*SkipWhite (input) != 0) {
         std::cerr << "Unexpected text after end...\n";
         return 1;
      }
   }
   catch (const CharNotFound & c) {
      std::cerr << "Expected '" << c.Char << "' at position "
                << c.Input - argv[1] << ".\n";
      return 1;
   }
   std::cout << term << std::endl;

   Bits bits;
   bits.push_back (false);
   Tree type;
   Generate (bits, Context(), term, type);
   std::cout << type << std::endl;

   // Now convert to a Tree...
   Tree bt = 0;
   for (Bits::reverse_iterator i = bits.rbegin(); i != bits.rend(); ++i) {
      std::cout << *i;
      bt = bt.Double();
      if (*i) {
         bt = bt.Increment();
      }
   }

   std::cout << std::endl;

   Tree output = Derive (bt);

   PrintDerived (std::cout, output);

   // Check the returned term.
   assert (Normalise (term) == output.Left().Left());
   // Check the returned type.
   assert (Normalise (type) == output.Left().Right().Left());
   // Check there are no remaining bits.
   assert (output.Left().Right().Right().Left().IsNull());
   // Check that the context is empty.
   assert (output.Left().Right().Right().Right().IsNull());

   return 0;
}
