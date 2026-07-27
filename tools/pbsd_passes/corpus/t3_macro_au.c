#define ADD(a, b) ((a) + (b))
#define SUM(x, y) ((x) + (y))
#define WEIRD(a) do { if (a) foo(); else bar(); } while (0)

int main(void) {
  return ADD(1, 2) + SUM(3, 4);
}
