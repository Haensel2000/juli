int printf(char[] s, ...);

double add(double x, double y) {
	return x + y;
}

void main(int argc, char[][] argv) {
	printf("%d, %d\n", argc, argv[1][add(1,2)]);
	double y;
	y = add(2.0, 1, 3);
	return;
}
