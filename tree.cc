
#include "tree.hh"

#include <assert.h>
#include <set>
#include <iostream>
#include <unistd.h>

struct NodeCompare
{
   bool operator() (const Node & a, const Node & b) const
      {
         return a.left != b.left ? a.left < b.left : a.right < b.right;
      }
};

static std::set <Node, NodeCompare> CanonicalNodeSet;

const Node * Pair (const Node * l,
                   const Node * r)
{
   return &*CanonicalNodeSet.insert (Node (l, r)).first;
}

int Tree::ToInt() const
{
   if (IsNull()) {
      return 0;
   }
   int left = Left().ToInt();
   int right = Right().ToInt();
   assert (right < 32 &&
           ((0x80000000 >> right) & left) == 0);
   return (2 * left + 1) << right;
}

static int iRight (int xx)
{
   int res = 0;
   while ((xx & 1) == 0) {
      ++res;
      xx >>= 1;
   }
   return res;
}

static int iLeft (int xx)
{
   while ((xx & 1) == 0) {
      xx >>= 1;
   }
   return xx >> 1;
}

Tree::Tree (int xx) :
   it (xx ? Pair (Tree (iLeft (xx)).it,
                  Tree (iRight (xx)).it)
       : NULL)
{
}

bool Tree::operator== (int n) const
{
   if (n == 0) {
      return it == NULL;
   }
   else {
      return !IsNull()
         &&  Left() == iLeft (n)
         &&  Right() == iRight (n);
   }
}

// An array containing small Trees (up to 20).
struct SmallTree {
   static const size_t limit = 20;
   SmallTree() 
      {
         for (size_t i = 0; i != limit; ++i) {
            trees[i] = i;
         }
      }
   Tree operator[] (int n) const
      { return trees[n]; }
   Tree trees[limit];
} smallTree;

// Increment, Double and Decrement are mutually recursive.
Tree Tree::Increment() const
{
   if (IsNull()) {
      return smallTree[1];
   }
   if (!Right().IsNull()) {
      return Pair (Pair (Left(), Right().Decrement()),
                   smallTree[0]);
   }
   return Left().Increment().Double();
}

Tree Tree::Double() const
{
   if (IsNull()) {
      return *this;
   }

   return Pair (Left(), Right().Increment());
}

Tree Tree::Decrement() const
{
   Tree result = Left().Double();
   for (int n = Right().ToInt(); n; --n) {
      result = Pair (result, smallTree[0]);
   }
   return result;
}

// Divide by 2.
Tree Tree::Halve() const
{
   if (IsNull()) {
      return *this;
   }
   else if (Right().IsNull()) {
      return Left();
   }
   else {
      return Pair (Left(), Right().Decrement());
   }
}

static Tree lastRight, accumulate;

static inline Tree Left (Tree t)
{
   lastRight = t.Right();
   return t.Left();
}

inline Tree Right (Tree t)
{
   return lastRight = t.Right();
}

inline Tree operator<< (int n, Tree t)
{
   assert (n > 0 && (n & 1));
   return Tree (n >> 1, t);
}

class TreeMinusInt
{
public:
   TreeMinusInt (Tree t, int n) :
      tree (t),
      number (n)
      { }
   operator const void *() const
      { return tree == number ? NULL : this; }
   operator Tree() const
      {
         return tree.ToInt() - number;
      }
   Tree tree;
   int number;
};
         
static inline TreeMinusInt operator- (Tree t, int n)
{
   return TreeMinusInt (t, n);
}

class TreeMinusTree
{
public:
   TreeMinusTree (Tree l, Tree r) :
      equal (l == r) { }
   operator const void *() const
      { return equal ? NULL : this; }
private:
   bool equal;
};

static inline TreeMinusTree operator- (Tree l, Tree r)
{
   return TreeMinusTree (l, r);
}

// We compute t/2 lazily: store t in a TreeSlashTwo object, and then either just
// test the bottom bit, of actually do the halving.
class TreeSlashTwo
{
public:
   TreeSlashTwo (Tree t) :
      tree (t)
      { }
   int operator& (int n) const
      {
         assert (n == 0 || n == 1);
         return n && !tree.IsNull() &&
            (tree.Right() == smallTree[1] ||
             (tree.Right().IsNull() && tree.Left().Right().IsNull()));
      }
   operator Tree() const
      { return tree.Halve(); }
private:
   Tree tree;
};

static inline TreeSlashTwo operator/ (Tree t, int n)
{
   assert (n == 2);
   return TreeSlashTwo (t);
}

static inline Tree operator/= (Tree & t, int n)
{
   assert (n == 2);
   return t = TreeSlashTwo (t);
}

class BitwiseNotTree
{
public:
   BitwiseNotTree (Tree t) : tree (t) { }
   int operator& (int n) const
      {
         assert (n == 2);
         return !tree.IsNull() &&
            (tree.Right() == smallTree[1] ||
             (tree.Right().IsNull() && tree.Left().Right().IsNull())) ? 0 : 2;
      }
private:
   Tree tree;
};

static inline BitwiseNotTree operator~ (Tree t)
{
   return t;
}

typedef int INT;
typedef Tree TREE;
typedef Tree BitStream;
TREE Apply (TREE, TREE);

#define main MAIN
#ifndef DESCEND
#define DESCEND 0
#endif
#include "pure.c"
#undef main

// The operator<< is specific to terms...
std::ostream & operator<< (std::ostream & s, Tree tree)
{
   if (tree.IsNull()) {
      return s << 0;
   }

   Tree opcode = tree.Left();
   Tree body = tree.Right();
      
   if (opcode.IsNull()) {
      if (!body.IsNull()) {
         return s << "PI(" << body.Left() << "," << body.Right() << ")";
      }
   }
   else if (opcode == smallTree[1]) {
      if (!body.IsNull()) {
         return s << "LAMBDA(" << body.Left() << "," << body.Right() << ")";
      }
   }
   else if (opcode == smallTree[2]) {
      if (!body.IsNull()) {
         return s << "APPLY(" << body.Left() << "," << body.Right() << ")";
      }
   }
   else if (opcode == smallTree[3]) {
      if (body.IsNull()) {
         return s << "STAR";
      }
      else if (body == smallTree[1]) {
         return s << "BOX";
      }
   }
   else {
      if (body.IsNull()) {
         return s << "VAR " << tree.Left().ToInt() / 2 - 2;
      }
   }

   return s << "Pair(" << tree.Left() << "," << tree.Right() << ")";
}

std::ostream & PrintContext (std::ostream & s, Tree tree)
{
   if (tree.IsNull()) {
      return s << "<>";
   }
   else {
      return PrintContext (s,tree.Right()) << "," << tree.Left();
   }
}

std::ostream & PrintBitstream (std::ostream & s, Tree t)
{
   if (t.IsNull()) {
      return s.put ('0');
   }
   if (!t.Left().IsNull()) {
      PrintBitstream (s, t.Left());
   }
   s.put ('1');
   for (int n = t.Right().ToInt(); n; --n) {
      s.put ('0');
   }
   return s;
}

std::ostream & PrintDerived (std::ostream & s, Tree t)
{
   if (t.IsNull()) {
      return s << ".\n";
   }
   s << t.Left().Left() << " : " << t.Left().Right().Left() <<
      " [ ";
   PrintContext (s, t.Left().Right().Right().Right()) << " ] ";
   PrintBitstream (s, t.Left().Right().Right().Left()) << std::endl;
//   return PrintDerived (s, t.Right());
   return s;
}

// int main (int argc, const char *const * argv)
// {
//    BitStream bits = 0;
   
//    // Convert argv[1] from hex.
//    for (const char * p = argv[1];
//         p && *p; ++p) {
//       int nibble = (*p & ~32) - ('0' & ~32);
//       if (nibble > 9) {
//          nibble -= 'A' - '9' - 1;
//       }
//       bits = bits.Double();
//       if (nibble & 8) {
//          bits = bits.Increment();
//       }
//       bits = bits.Double();
//       if (nibble & 4) {
//          bits = bits.Increment();
//       }
//       bits = bits.Double();
//       if (nibble & 2) {
//          bits = bits.Increment();
//       }
//       bits = bits.Double();
//       if (nibble & 1) {
//          bits = bits.Increment();
//       }
//    }

//    PrintBitstream (std::cout << "Bits: ", bits) << std::endl;

//    PrintDerived (std::cout, Derive (bits));
//    return 0;
// }

