#include <assert.h>

int Pair (int, int);
int Left (int);
int Right (int);
extern int lastRight;

int main()
{
   for (int i = 0; i != 256; ++i) {
      for (int j = 0; j != 16; ++j) {
         int p = Pair (i, j);
         assert (Left (p) == i && lastRight == j &&
                 Right (p) == j && lastRight == j);
      }
   }
   for (int i = 1; i != 1000000; ++i) {
      assert (i == Pair (Left (i), Right (i)));
   }
   return 0;
}
