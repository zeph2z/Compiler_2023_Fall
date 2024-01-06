int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

int mul(int a, int b) {
  return a * b;
}

int div(int a, int b) {
  return a / b;
}

int main() {
  int x = 1;
  x = add(1, 2);
  int y = add(1 || 0, 0 && sub(1, x) || mul(3, div(x || add(1, 2) > 10, 5)));
  return x + y;
}