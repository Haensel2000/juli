int printf(char[] s, ...);

double add(double x, double y) {
	return x + y;
}

void main(int argc, char[][] argv) {
	printf("%d, %d\n", argc, argv[1][0]);
	return;
}
