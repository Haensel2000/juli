int printf(char[] s, ...);

int strlen(char[] str) {
	int l;
	l = 0;
	while (str[l] != 0) {
		l = l + 1;	
	}
	return l;
}

int digit(char c) {
	return c - 48;
}

int parse(char[] str) {
	int i;
	int v;
	int b;
	i = strlen(str) - 1;
	v = 0;
	b = 1;
	while (i >= 0) {
		v = v + b*digit(str[i]);
		i = i - 1;
		b = b * 10;
	}
	return v;
}

void main(int argc, char[][] argv) {
	printf("%d %d\n", strlen(argv[1]), parse(argv[1]));
	return;
}
