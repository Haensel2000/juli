C int printf(char[] s, ...);

int main(char[][] args) 
{
  double x = 3;
  int i = args.length;
  printf("%s, %c, %d, %d\n", args[0], args[0][2], i, args[1].length);
  return 0;
}
