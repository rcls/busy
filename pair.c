
typedef int Tree;
typedef int INT;
typedef int TREE;
typedef int BitStream;
#define DESCEND xx

Tree lastRight, accumulate;

// A bijective pairing.
TREE Pair (TREE yy, TREE xx)
{
   // x - ~x = x - (-1 - x) = 2 * x + 1
   return yy - ~yy << xx;
}

// The second component of a pair.
TREE Right (TREE xx)
{
   return lastRight = xx % 2 ? 0 : 1 + Right (xx / 2);
}

// The first component.  Note that we leave the other component in lastRight.
TREE Left (TREE xx)
{
   return xx / 2 >> Right (xx);
}
