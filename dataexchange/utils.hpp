#include<stdlib.h>

// code steal from https://github.com/funny/crypto/blob/master/dh64/c/dh64.c, for test only.
#define P 23
#define G 5

static inline uint64_t
mul_mod_p(uint64_t a, uint64_t b) {
	uint64_t m = 0;
	while (b) {
		if (b&1) {
			uint64_t t = P - a;
			if (m >= t) {
				m -= t;
			} else {
				m += a;
			}
		}
		if (a >= P - a) {
			a = a * 2 - P;
		} else {
			a = a * 2;
		}
		b >>= 1;
	}
	return m;
}

static inline uint64_t
pow_mod_p(uint64_t a, uint64_t b) {
	if (b == 1) {
		return a;
	}
	uint64_t t = pow_mod_p(a, b>>1);
	t = mul_mod_p(t, t);
	if (b % 2) {
		t = mul_mod_p(t, a);
	}
	return t;
}

// calc a^b % p
static inline uint64_t
powmodp(uint64_t a, uint64_t b) {
	if (a == 0)
		return 1;
	if (b == 0)
		return 1;
	if (a > P)
		a %= P;
	return pow_mod_p(a, b);
}