
// Encoding
// PI(A,B) = Pair(0,Pair(A,B))
// LAMBDA(A,B) = Pair(1,Pair(A,B))
// APPLY(A,B) = Pair(2,Pair(A,B))
// STAR = Pair(3,0) = 7
// BOX = Pair(3,1) = 14
// VAR(n) = Pair(4+2n,0) = 9 + 4n [n >= 0]
// The empty context is 0, and the context Gamma,A is Pair (A,Gamma).
// STAR and BOX are the only terms x with (x&2)!=0

// Increment the index of each variable in xx.  Uses Subst.
// Making this a macro means that we can absorb an "=" and a "(" into the macro.
#define Lift(xx) Subst (4, 13, -4, xx)

// Substitute yy for vv in term, and normalise.  Variables > yy get adjusted by
// -context.  [The precise normalisation is: if yy and term are normal, and the
// substitution has a normal form, then the normal form is returned.]
TREE Subst (INT vv, TREE yy, INT context, TREE term)
{
   Tree
      aux = Left (term),        // The operation of term.
      xx = lastRight;           // The body of term.

   {
      return
         aux - 2 ?
         aux > 2 ?
         // Variable or Star or Box.
         aux - vv ? term - (aux > vv) * context : yy :
         // aux = 0 or aux = 1: lambda or pi.  The stray 'term =' below is
         // harmless, but allows us to push the '=' into the Lift macro.
         Pair (aux, Pair (Subst (vv, yy, context, Left (xx)),
                          Subst (vv+2, term = Lift (yy), context, Right (xx))))
         :
         // Application.  Use Apply.
         Apply (Subst (vv, yy, context, Left (xx)),
                Subst (vv, yy, context, Right (xx)));
   }
}

// Apply yy to xx and normalise.  [Precisely, if yy and xx are normal, and
// yy(xx) is normalisable, Apply(yy,xx) returns the normal form of yy(xx).
TREE Apply (TREE yy, TREE xx)
{
   return Left (yy) - 1
      // 5 << x == Pair(2,x)
      ? 5 << Pair (yy, xx)
      : Subst (4, xx, 4, Right (lastRight));
}

// We use xx as a bit stream.  The MAYBE macro tests the next bit for us.
#define MAYBE (xx /= 2) % 2 &&

// Derive parses a bit stream into terms of CoC and normalises everything.  The
// outputs are accumulated into the variable yy.  We also recurse, so as to
// cover all the BitStreams which are < xx.
TREE Derive (BitStream xx)
{
   Tree
      aux,
      auxTerm,
      // The axiom.
      context = 0,
      term = 7,
      type = 14;

   // Inside the while condition is the main recursion that makes us monotone.
   // It doesn't need to be inside the while, but that allows us to compress the
   // "),".  It also means we get called more often, which makes "accumulate"
   // bigger...
   while (DESCEND && Derive (xx - 1), MAYBE (1))

      // Get another term.
      auxTerm = Left (Left (Derive (xx))),
         // And get its type.
         aux = Left (lastRight),

         // And get the left-over bit-stream.  This leaves the context from
         // the sub-derivation in lastRight.
         xx = Left (lastRight),

         // Rules that depend on two antecedents...  The two contexts (one is in
         // lastRight) must be the same.
         context - lastRight || (
            // APPLY.  type must be PI(aux,-).
            Left (type) || Left (lastRight) - aux ||
            MAYBE (type = Subst (4, auxTerm, 4, lastRight),
                   term = Apply (term, auxTerm)),

            // Weakening.  auxType must be STAR or BOX.  The / 2 & MAYBE
            // combines MAYBE with testing the correct bit of auxType.  It is
            // safe to do this immediately after an APPLY above, because APPLY
            // does not change contexts.
            aux / 2 & MAYBE ( context = Pair (auxTerm, context),
                              term = Lift (term),
                              type = Lift (type) )

            ),

         context && MAYBE (
            // If we get here, we are either going to do PI formation or LAMBDA
            // introduction.  PI formation requires type to be STAR or BOX.  We
            // allow LAMBDA introduction whenever the context is non-empty.
            // This extension is a conservative extension of CoC.
            term = Pair (
               // Because of the && in MAYBE, this subexpression returns a
               // boolean 1 if we're doing LAMBDA introduction, 0 if we're
               // doing PI formation.  The ~type&2| ensures that we do LAMBDA
               // introduction if type is not the Star or Box needed to do PI
               // formation.
               ~type & 2 | MAYBE (
                  // If we're doing lambda introduction on term, then we also
                  // need to do a PI formation on type.  This is always
                  // non-zero.  1 << x = Pair(0,x).
                  type = 1 << Pair (Left (context), type)),
               Pair (Left (context), term)),

            // Remove the context item we just used.
            context = lastRight ),

         // If type is STAR or BOX then we allow variable introduction.
         type / 2 & MAYBE (
            context = Pair (term, context),
            type = Lift (term),
            term = 9 );     // Pair (4, 0)
   {
      // Pair term, type, context, and xx together, and chuck it all onto
      // accumulate.
      return accumulate = Pair (Pair (term, Pair (type, Pair (xx, context))),
                                accumulate);
   }
}

TREE main ()
{
   return Derive (Derive (Derive (Derive (Derive (99)))));
}
