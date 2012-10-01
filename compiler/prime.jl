C int printf(char[] s, ...);

C double sqrt(double x);

int strlen(char[] str) 
{
  int l;
  l = 0;
  while (str[l] != 0) 
  {
    l = l + 1;
  }
  return l;
}

int digit(char c) 
{
  return c - 48;
}

int parse(char[] str) 
{
  int i;
  int v;
  int b;
  i = strlen(str) - 1;
  v = 0;
  b = 1;
  while (i >= 0) 
  {
    v = v + b*digit(str[i]);
    i = i - 1;
    b = b * 10;
  }
  return v;
}

int sqr(int v) 
{
  return v*v;
}

int factor(int v)
{
  int i;
  int r;
  i = 2;
  while (i <= sqrt(v))
  {
    //printf("v = %d, i = %d\n", v, i);
    if (v % i == 0)
    {
      printf("%d, ", i);
      v = v / i;
      i = 1;
    }
    i = i + 1;
  }
  printf("%d\n", v);
  return 0;
}

void main(int argc, char[][] argv) 
{
  factor(parse(argv[1]));
  return;
}
