C int printf(char[] s, ...);

void x(char[] s) {
  printf(s);
  printf("\n");
  return;
}

void x(int s) {
  printf("i = %d\n", s);
  return;
}

void main(int argc, char[][] argv) 
{
  int i = 323;
  printf("haha %d\n", -argc);
  x(i);
  x("test");
  
  x(argv[1]);

  return;
}
