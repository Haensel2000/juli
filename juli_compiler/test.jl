int printf(cstr s, ...);

double add(double x, double y) {
	return x + y;
}

void main() {
	double x;
	x = add(4.0, 3.0);
	if (x == 4.0 + 3.0) {
		printf("%.2f\n\n", x);
	} else if (x == 3.0 + 4.0) {
		printf("dont call else if");
	} else if (x == 0.0) {
		printf("hm");
	}
	return;
}
