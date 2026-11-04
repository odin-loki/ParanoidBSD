#define ADD(a, b) ((a) + (b))
#define SQR(x) ((x) * (x))
#define BAD(a) do { foo(a); } while (0)

int main(void) {
  return ADD(1, 2) + SQR(3);
}
