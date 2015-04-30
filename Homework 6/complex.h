// complex struct declaration
typedef struct complex {
  double real;
  double imag;
} complex;

//(a + bi) + (c + di) = (a + c) + (b + d)i
complex addc(complex a, complex b) {
  complex c;
  c.real = a.real + b.real;
  c.imag = a.imag + b.imag;
  return c;
}

//(a + bi)(c + di) = (ac - bd) + (ad + bc)i
complex multc(complex a, complex b) {
  complex c;
  c.real = (a.real * b.real) - (a.imag * b.imag);
  c.imag = (a.real * b.imag) + (a.imag * b.real);
  return c;
}

double norm_squared(complex a) {
  return a.real*a.real + a.imag*a.imag;
}
