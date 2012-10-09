C int printf(char[] s, ...);

double[,] mul(double[,] x, double[,] y)
{
  double[,] result = new double[2,2];
  result[0,0] = x[0,0]*y[0,0] + x[0, 1]*y[1,0];
  result[0,1] = x[0,0]*y[0,1] + x[0, 1]*y[1,1];
  result[1,0] = x[1,0]*y[0,0] + x[1, 1]*y[1,0];
  result[1,1] = x[1,0]*y[0,1] + x[1, 1]*y[1,1];
  return result;
}

double[] mul(double[,] x, double[] y)
{
  double[] result = new double[2];
  result[0] = x[0,0]*y[0] + x[0, 1]*y[1];
  result[1] = x[1,0]*y[0] + x[1, 1]*y[1];
  return result;
}

void dump(double[,] m)
{
  printf("%dx%d:\n%.2f, %.2f\n%.2f, %.2f\n\n", m.length[0], m.length[1], m[0,0], m[0,1], m[1,0], m[1,1]);
  return;
}

void dump(double[] v)
{
  printf("%d:\n%.2f, %.2f\n\n", v.length, v[0], v[1]);
  return;
}

void dump(int[] iv)
{
  int i = 0;
  while (i < iv.length)
  {
    printf("%d ", iv[i]);
    i = i + 1;
  }
  printf("\n");
  return;
}

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
    printf("a and b\n");
  } 
  else 
  {
    printf("else\n");
  }
  
  
  
  
  double[,] x = new double[2,2];
  x[0,0] = 1.0;
  x[0,1] = 0.0;
  x[1,0] = 0.0;
  x[1,1] = 2.0;
  
  double[,] y = new double[2,2];
  y[0,0] = 1.0;
  y[0,1] = 0.0;
  y[1,0] = 0.0;
  y[1,1] = 2.0;
  
  double[,] z = mul(x, y);
  
  double[] v = new double[2];
  v[0] = 2.0;
  v[1] = 1.0;
  
  dump(x);
  dump(y);
  dump(z);
  
  dump(mul(z, v));
  
  char[][] words = new char[][3];
  words[0] = "test";
  words[1] = "w2";
  words[2] = "w3";
  
  int[][] ni = new int[][2];
  ni[0] = new int[3];
  ni[1] = new int[5];
  
  ni[0][0] = 1;
  ni[0][1] = 0;
  ni[0][2] = -1;
  
  dump(ni[0]);
  dump(ni[1]);
  
  
  char[] r = new char[3];
  r[0] = 'A';
  r[1] = 'B';
  r[2] = 'C';
  printf("r == %s, r.length == %d \n", r, r.length);
  
  printf("%s, %s, %s\n", words[0], words[1], words[2]);
  
  printf("Dimensions of z: %d\n", z.length.length);
  
  printf("%d, %d, %f\n", x.length[0], x.length[1], x[1,2]);
  //int i = args.length;
  //printf("%s, %c, %d, %d\n", args[0], args[0][2], i, args[1].length);
  return 0;
}
