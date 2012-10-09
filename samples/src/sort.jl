C int printf(char[] s, ...);

int strlen(char[] str) 
{
  int l = 0;
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
  int i = strlen(str) - 1;
  int v = 0;
  int b = 1;
  while (i >= 0) 
  {
    v = v + b*digit(str[i]);
    i = i - 1;
    b = b * 10;
  }
  return v;
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

void insertion_sort(int[] numbers)
{
    int j = 1;
    while (j < numbers.length)
    {
      int i = j;
      while (i > 0 and numbers[i-1] > numbers[i])
      {
	int tmp = numbers[i-1];
	numbers[i-1] = numbers[i];
	numbers[i] = tmp;
	i = i - 1;
      }
      j = j + 1;
    }
    return;
}

int main(char[][] args) 
{
  int i = 1;
  int[] numbers = new int[args.length-1];
  while (i < args.length)
  {
    numbers[i-1] = parse(args[i]);
    i = i + 1;
  }
  dump(numbers);
  insertion_sort(numbers);
  dump(numbers);
  return 0;
}