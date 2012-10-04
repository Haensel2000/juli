C int printf(char[] s, ...);

int main(char[][] args)
{
  boolean a = true;
  boolean b = false;
  
  if (a and b)
  {
    printf("a and b\n");
  } 
  else if (a or b) 
  {
    printf("a or b\n");
  } 
  else 
  {
    printf("else\n");
  }
  return 0;
}