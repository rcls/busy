#! /bin/perl

undef $/;

print "#define R { return\n";
# Macros don't expand recursively...
print "#define P P (\n";
print "#define L L (\n";
print "#define T S (v, y, c,\n";
print "#define C ),\n";

print "#define X x)\n";
print "#define F );}\n";

while (<>) {
  s,//.*,,g;
  s,typedef.*,,g;
  s,#define DESCEND.*,,g;
  s,DESCEND,xx,g;
  s,Tree,int,g;
  s,TREE ,,g;
  s,INT ,,g;
  s,BitStream ,,g;

  s,{?( *\n)* *return,\n   R,g;
  s/#define Lift.*/#define U = Subst(4,13,-4,/g;
  s/#define MAYBE (.*)/#define B \1 (/,;
  s%Subst \(vv, yy, context,%T %g;

  s,Apply,A,g;
  s,MAYBE *\(,B ,g;
  s%\) *,% C %g;
  s,Derive,D,g;
  s,\);[ \n]*}, F,g;
  s,Left *\(,L ,g;
  s,Pair *\(,P ,g;
  s,Subst,S,g;
  s,= *Lift *\(, U ,g;
  s,xx\),X ,g;
  s,\bRight,Z,g;

  s,accumulate,a,g;
  s,bits,b,g;
  s,context,c,g;
  s,auxTerm,d,g;
  s,aux,f,g;
  s,lastRight,r,g;
  s,term,t,g;
  s,type,u,g;
  s,vv,v,g;
  s,xx,x,g;
  s,yy,y,g;

  s, *\n( *\n)+,\n,g;

  print;
}
