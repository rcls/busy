// Calculate the height of the exponential tower given by the initial bootstrap.

// Compile pure.c with the recursive search enabled.
#define DESCEND xx

#include "tree.cc"

int height (Tree h)
{
   if (h.IsNull()) {
      return 0;
   }

   int left = height (h.Left());
   int right = height (h.Right()) + 1;

   return left > right ? left : right;
}   

int main()
{
   // We subtract 1 to take account of the fact that 2^(2^(2^0)) = 2^2 etc...
   std::cout << "The bootstrap tower has height: "
             << height (Derive (99)) -1 << std::endl;
   return 0;
}

