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

int main(char[][] args) 
{
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
  
  printf("Dimensions of z: %d\n", z.length.length);
  
  printf("%d, %d, %f", x.length[0], x.length[1], x[1,2]);
  //int i = args.length;
  //printf("%s, %c, %d, %d\n", args[0], args[0][2], i, args[1].length);
  return 0;
}
