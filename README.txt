Introduction
============

This implements a parser, type-checker, interpreter and proof-search for the
Huet-Coquand "Calculus of Constructions" (CoC).  See Barendregts "Lambda
Calculi with Types" (Handbook of Logic in Computer Science, Volumes 1) for an
intro to this system and further references.

Recall Berry's paradox of "one plus the largest number that can be
stated in less than one hundred words".  If we rephrase "stated" to
restrict ourselves to some formal language L, then we don't get a
paradox, instead we get an efficient mechanism for generating large
numbers:

   The largest number that can be stated in the language L in less
   than N symbols.

If L is a reasonably expressive language, and N is moderately large, then the
number above is clearly going to be enormous.  If "stated in the language L" is
appropriately computable, then we can write a program by (a) writing an
interpreter for L, and (b) applying this to all statements of length <= N
symbols.

A Turing-complete programing language is not suitable for L - because we will
attempt to interpret a non-terminating program, and never return.  The Calculus
of Constructions is an excellent choice for L.  It is very expressive, but it is
not Turing-complete, and our interpreter will always return.

Structure of this Directory.
============================

The actual BIGNUM BAKEOFF entry is the file reduced.c.

The human readable source code is in two files, pair.c and pure.c.  The
Makefile concatenates these to form full.c, and then uses the perl script
pure.pl to convert full.c into reduced.c.  The C++ source files are test code.

Test Rig
========

pair.c implements a bijective pairing (Pair, Left, Right) on the natural
numbers (using the C type int).  This is not efficient for real usage.  For
testing purpose, we use C++ and operator overloading to give a practical
implementation of syntax trees (type Tree) and the pairing functions.  This is
driven by the source file tree.cc, which #includes pure.c.

The Program
===========

Consists of several functions.  We use the long names from the original source
(full.c), not the one-letter names from reduced.c.

int Pair (int, int) - a pairing function with
int Left (int)      - left project and
int Right (int)     - right projection
      These are used to encode syntax trees of pre-terms - see the comments at
      the start of pure.c for details.

int Subst (int, int, int, int) - Performs simultaneous syntactical substitution
      and normalisation on syntax trees.

int Lift (int) - actually a macro.  We using de Bruijn indices for variables.
      Lift increments all the indices in a syntax tree.

int Apply (int, int) - Form an application pre-term "f(a)" from "f" and "a" and
      normalise.

      Note that Subst, Lift and Apply do not terminate on all inputs, as they
      carry out normalisation of pre-terms.  We rely on Derive only giving them
      inputs on which they do terminate.

int Derive (int) - The main function.  The parameter is 'xx' which is used as
      a bit-stream we parse this into a syntax tree of CoC, being careful only
      to perform valid inferences of CoC (i.e., type-checking), while at the
      same time normalising (i.e., intepreting).

      Because Derive only constructs valid terms of CoC, the strong
      normalisation theorem for CoC guarantees that all our calls to Subst,
      Lift and Apply do in fact terminate.

      Additionally, we carry out a proof-search - if xx is non-zero, then
      Derive(xx) calls Derive(xx-1).  All the results are accumulated in the
      global variable "acculumate", as well as being returned.

int main() - returns Derive^5 (99).  The inner Derive (99) is boot
      strapping; it returns a moderate sized number (a stack of exponentials
      30419 high).  This guarentees that the subsequent calls to Derive cover a
      sufficiently interesting range of terms.

How Big is our number?
======================

To analyse this requires some knowledge of mathematical logic and proof theory.

The size of our output is driven by the proof-theoretic strength of CoC.
Higher Order Logic can be translated into this system (and vice versa), and
thus CoC suffices to formalise all of "normal" mathematics (basic analysis,
combinatorics, etc.).  [This glosses over the issue of intuitionistic
v. classical logic - the distinction is not relevant for us since they
coincide on Pi^0_2-arithmetic statements (as proved by Kriesel and Friedman);
we're actually only interested in Sigma^0_1 arithmetic statements.]

Our program searchs a range of terms, and carries out normalisation for them.
If a term in the range we cover proves a Sigma^0_1-arithmetic statement, then
the normalisation normalisation actually computes a witness (suitably encoded)
for that statement.

The following now states approximately how big the return value of Derive (and
hence) main, is:

   Let N be the base 2 log of the argument of Derive.

   Suppose that Higher Order Arithmetic can prove a closed Sigma^0_1-arithemetic
   statment "Exists(x) phi(x)", with a proof not longer than N.

   Then "Derive(N)" returns a number larger than the smallest x such that
   phi(x).

Exactly what I mean by the qualification "with a proof not longer than N" needs
explication: the measure of the length of the proof is taken after translating
into a CoC derivation and encoding as a bit stream in the format used by Derive.

This encoding is (I think) double exponential worst-case; however considering
the size of N, the size limitation still allows all proofs that a human is
actually ever going to write out in practice.  The limitation that is relevant
in practice is that the proof can be carried out in HOL.

