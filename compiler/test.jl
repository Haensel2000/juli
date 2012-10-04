C int printf(char[] s, ...);

struct A
{
  int x;
}

void print(A a)
{
  printf("%d\n", a.x);
  return;
}

int main(char[][] args) 
{
  A a = new A;
  if (a != null)
  {
    a.x = 5;
    print(a);
  }
  return 0;
}
