int printf(cstr s, ...);

double add(double x, double y) {
	return x + y;
}

void main() {
	double x;
	x = add(4.0, 3.0);
	printf("%.2f\n\n", x);
	return;
}
