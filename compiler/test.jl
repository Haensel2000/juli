int printf(char[] s, ...);

int parse(char[] str) {
	int i;
	int v;
	i = 0;
	v = 0;
	while (str[i] != 0) {
		v = v + str[i];
	}
}

void main(int argc, char[][] argv) {
	printf("%d, %d\n", argc, argv[1][0]);
	double y;
	y = add(2.0, argc);
	return;
}
