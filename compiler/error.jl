C int printf(char[] s, ...);

struct A
{
  B x;
}

struct B
{
  int y; 
}

int main(char[][] args)
{
  A a = new A;
  B b = new B;
  a.x = b;
  b.y = 7;
  printf("%d", a.x.y);
  return 0;
}