#ifndef TREE_HH_
#define TREE_HH_

#include <assert.h>
#include <iostream>
#include <stddef.h>

// This is the raw data storage for our syntax trees.
struct Node
{
   Node (const Node * l = NULL,
         const Node * r = NULL) :
      left (l),
      right (r)
      { }
   bool operator== (const Node & other) const
      { return left == other.left && right == other.right; }

   const Node * left;
   const Node * right;
};

// The pairing function on the raw data.
const Node * Pair (const Node * l,
                   const Node * r);

// This wrapper class is what everything uses.
struct Tree {

   Tree() { }
   Tree (int n);
   Tree (const Node * node) :
      it (node) { }
   Tree (const Tree & other) :
      it (other.it) { }
   Tree (const Tree & l, const Tree & r) :
      it (Pair (l.it, r.it)) { }

   Tree Left() const { return it->left; }
   Tree Right() const { return it->right; }

   bool IsNull() const { return it == NULL; }

   // This is really a conversion to bool, except by converting to void* we
   // avoid accidentally carrying out arithmetic on the result.
   operator const void *() const { return it; }

   bool operator== (const Tree & other) const
      { return it == other.it; }
   bool operator== (int other) const;
   bool operator!= (int other) const { return !(*this == other); }
   bool operator> (int other) const
      { return ToInt() > other; }

   // Arithmetic: add / subtract one, multiply / divide by 2.
   Tree Double() const;
   Tree Halve() const;
   Tree Increment() const;
   Tree Decrement() const;

   // Is the lower bit set?
   bool IsOdd() const
      { return !IsNull() && Right().IsNull(); }

   // BitwiseAnd with a bitmask [must be 1].
   int operator& (int bit) const
      {
         assert (bit == 0 || bit == 1);
         return bit && IsOdd();
      }
   // Modulo [n must be 2.]
   int operator% (int n) const
      {
         assert (n == 2);
         return IsOdd();
      }

   // Convert to an int.
   int ToInt() const;

   const Node * it;
};

inline Tree Pair (const Tree & l,
                  const Tree & r)
{
   return Tree (l, r);
}

// The stuff that pure gives us...
Tree Subst (int, Tree, int, Tree);
Tree Apply (Tree, Tree);
Tree Derive (Tree);

// The operator<< is specific to terms...
std::ostream & operator<< (std::ostream & s, Tree tree);
std::ostream & PrintContext (std::ostream & s, Tree tree);
std::ostream & PrintBitstream (std::ostream & s, Tree t);
std::ostream & PrintDerived (std::ostream & s, Tree t);

#endif
