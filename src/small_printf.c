#include "platform.h"

void uart_putchar(char c);

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

static int v_printf_str_to_num(const char *fmt, int *num) {
  const char *p;
  int res, d, isd;

  res = 0;
  for (p = fmt; *fmt != '\0'; p++) {
    isd = (*p >= '0' && *p <= '9');
    if (!isd)
      break;
    d = *p - '0';
    res *= 10;
    res += d;
  }
  *num = res;
  return ((int)(p - fmt));
}

static void v_printf_num_to_str(uint32_t a, int ish, int pl, int pc) {
  char buf[32];
  uint32_t base;
  int idx, i, t;

  for (i = 0; i < sizeof(buf); i++)
    buf[i] = pc;
  base = 10;
  if (ish)
    base = 16;

  idx = 0;
  do {
    t = a % base;
    if (t >= 10)
      buf[idx] = t - 10 + 'a';
    else
      buf[idx] = t + '0';
    a /= base;
    idx++;
  } while (a > 0);

  if (pl > 0) {
    if (pl >= sizeof(buf))
      pl = sizeof(buf) - 1;
    if (idx < pl)
      idx = pl;
  }
  buf[idx] = '\0';

  for (i = idx - 1; i >= 0; i--)
    uart_putchar(buf[i]);
}

static int v_printf(const char *fmt, va_list va) {
  const char *p, *q;
  int f, c, vai, pl, pc, i;
  unsigned char t;

  pc = ' ';
  for (p = fmt; *p != '\0'; p++) {
    f = 0;
    pl = 0;
    c = *p;
    q = p;
    if (*p == '%') {
      q = p;
      p++;
      if (*p >= '0' && *p <= '9')
        p += v_printf_str_to_num(p, &pl);
      f = *p;
    }
    if ((f == 'd') || (f == 'x')) {
      vai = va_arg(va, int);
      v_printf_num_to_str(vai, f == 'x', pl, pc);
    } else {
      for (i = 0; i < (p - q); i++)
        uart_putchar(q[i]);
      t = (unsigned char)(f != 0 ? f : c);
      uart_putchar(t);
    }
  }
  return 0;
}

int small_printf(const char *fmt, ...) {
  va_list va;

  va_start(va, fmt);
  v_printf(fmt, va);
  va_end(va);
  return 0;
}
