
// Convert a tree back into a bitstream --- i.e., invert derive...

// We model the behaviour of Derive.  State describes what goes on inside that
// function.

#include "bitstream.hh"

static bool NormalisedEquals (const Context & a, const Context & b);

struct State
{
   enum Code {
      WHILE,
      AUXILARY,
      BINARY,
      APPLY,
      WEAK,
      CONTEXT,
      INTRO
   };

   State (Bits & b) :
      code (WHILE),
      bits (b),
      term (7),                 // STAR.
      type (14),                // BOX.
      auxTerm (0),
      auxContext (0)
      { }


   void Generate (const Context & c, Tree term);

   void Advance();
   void AdvanceTo (Code code);
   void AdvanceTo (Code code, Tree term);

   void DoApply (Tree auxTerm);
   void DoWeak (Tree auxTerm);

   void DoLambda();
   void DoPi();
   void DoIntro();

   Code code;

   Bits & bits;

   Tree term;
   Tree type;
   Context context;

   Tree auxTerm;
   Tree auxType;
   Context auxContext;
};

void Generate (Bits & bits, const Context & context,
               Tree term, Tree & type)
{
   State state (bits);

   state.Generate (context, term);
   state.AdvanceTo (State::WHILE);
   bits.push_back (false);      // Return...
   type = state.type;
}

void State::Generate (const Context & c, Tree t)
{
   int opcode = t.Left().ToInt();
   switch (opcode) {
   case 0:
   case 1: {
      // PI or LAMBDA.
      Context newContext = c;
      newContext.push_back (t.Right().Left());
      Generate (newContext, t.Right().Right());
      if (opcode == 0) {
         DoPi();
      }
      else {
         DoLambda();
      }
      break;
   }
   case 2:
      // APPLY.
      Generate (c, t.Right().Left());
      DoApply (t.Right().Right());
      break;

   case 3: {
      // Should be *.
      assert (t.Right() == 0);
      if (c.empty()) {
         break;
      }
      Context newContext = c;
      newContext.pop_back();
      Generate (newContext, t);
      DoWeak (c.back());
      break;
   }
   default: {                   // Variable.
      assert ((opcode & 1) == 0);
      size_t var = (opcode >> 1) - 2;
      assert (var < c.size());
      Context newContext = c;
      newContext.pop_back();
      if (var == 0) {
         // Intro...
         Generate (newContext, c.back());
         DoIntro();
      }
      else {
         // Weak...
         Generate (newContext, Pair (4 + 2 * (var - 1), 0));
         DoWeak (c.back());
      }
      break;
   }
   }
}

bool NormalisedEquals (const Context & a, const Context & b)
{
   if (a.size() != b.size()) {
      return false;
   }

   for (size_t i = 0; i != a.size(); ++i) {
      if (!::NormalisedEquals (a[i], b[i])) {
         return false;
      }
   }

   return true;
}

// Advance state while doing nothing.
void State::Advance()
{
   switch (code) {
   case WHILE:

      // Loop!
      bits.push_back (true);
      code = AUXILARY;
      break;

   case AUXILARY:

      // Trivial auxilary.
      bits.push_back (false);
      auxTerm = Pair (3,0);
      auxType = Pair (3,1);
      auxContext.clear();

      code = BINARY;
      break;

   case BINARY:

      if (NormalisedEquals (context, auxContext)) {
         code = APPLY;
      }
      else {
         code = CONTEXT;
      }
      break;

   case APPLY: {

      Tree whType = WeakHeadNormalise (type);
      if (whType.Left() == 0 &&
          NormalisedEquals (whType.Right().Left(), auxType)) {
         bits.push_back (false);
      }
      code = WEAK;
      break;
   }

   case WEAK:

      bits.push_back (false);
      code = CONTEXT;
      break;

   case CONTEXT:

      if (!context.empty()) {
         bits.push_back (false);
      }
      code = INTRO;
      break;

   case INTRO:
      bits.push_back (false);
      code = WHILE;
      break;
   }
}

void State::AdvanceTo (State::Code newCode)
{
   while (code != newCode) {
      Advance();
   }
}

void State::AdvanceTo (Code newCode, Tree newAuxTerm)
{
   while (code != newCode ||
          context != auxContext ||
          auxTerm != newAuxTerm) {
      if (code == AUXILARY) {
         // Generate the auxilary we want...
         ::Generate (bits, context, newAuxTerm, auxType);
         auxContext = context;
         auxTerm = newAuxTerm;
         code = BINARY;
      }
      else {
         Advance();
      }
   }
}

void State::DoApply (Tree newAuxTerm)
{
   AdvanceTo (BINARY, newAuxTerm);

   Tree whType = WeakHeadNormalise (type);
   assert (whType.Left() == 0);
   assert (::NormalisedEquals (whType.Right().Left(), auxType));

   bits.push_back (true);
   term = Pair (2, Pair (term, auxTerm));
   type = Subst (whType.Right().Right(), 0, auxTerm);

   code = WEAK;
}

void State::DoWeak (Tree newAuxTerm)
{
   AdvanceTo (WEAK, newAuxTerm);

   assert (auxType.Left() == 3);
   bits.push_back (true);
   context.push_back (auxTerm);
   term = Lift (term, 0);
   type = Lift (type, 0);
   code = CONTEXT;
}

void State::DoLambda()
{
   AdvanceTo (CONTEXT);

   assert (!context.empty());

   bits.push_back (true);
   bits.push_back (true);

   term = Pair (1, Pair (context.back(), term));
   type = Pair (0, Pair (context.back(), type));
   context.pop_back();

   code = INTRO;
}

void State::DoPi()
{
   AdvanceTo (CONTEXT);

   assert (!context.empty());
   assert (type.Left() == 3);
   bits.push_back (true);
   bits.push_back (false);

   term = Pair (0, Pair (context.back(), term));
   context.pop_back();

   code = INTRO;
}

void State::DoIntro()
{
   AdvanceTo (INTRO);

   assert (type.Left() == 3);

   bits.push_back (true);
   context.push_back (term);
   type = Lift (term, 0);
   term = Pair (4, 0);

   code = WHILE;
}
