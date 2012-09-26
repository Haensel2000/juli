int printf(char[] s, ...);

double sqrt(double x);

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

int sqr(int v) 
{
	return v*v;
}

int factor(int v)
{
	int i = 2;
	int r;
	while (i <= sqrt(v))
	{
		//printf("v = %d, i = %d\n", v, i);
		r = v / i;
		if (r*i == v)
		{
			printf("%d, ", i);
			v = r;
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
