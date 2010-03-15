#include <iostream>
#if defined(_WIN32)
#else
#include <sys/time.h> 
#include <sys/resource.h>
#endif

#define _TOSTR(s) #s
#define TOSTR(s) _TOSTR(s)
#if defined(MASM_TEST)
#define TEST_M(func_name) {test_impl<func_name>(TOSTR(func_name), masm_ ## func_name);}
#else
#define TEST_M(func_name)
#endif
#if defined(NASM_TEST)
#define TEST_N(func_name) {test_impl<func_name>(TOSTR(func_name), nasm_ ## func_name);}
#else
#define TEST_N(func_name)
#endif
#define TEST_EQUAL(actual, expected) {test_equal_impl(TOSTR(actual), (actual), (expected));}

extern size_t	g_test_succeeded;
extern size_t	g_test_failed;
extern double	g_assemble_time;

inline double get_time()
{
#if defined(_WIN32)
	LARGE_INTEGER t, f;
	::QueryPerformanceCounter(&t);
	::QueryPerformanceFrequency(&f);
	return static_cast<double>(t.QuadPart) / f.QuadPart;
#else
	rusage t;
	getrusage(RUSAGE_SELF, &t);
	return t.ru_utime.tv_sec + static_cast<double>(t.ru_utime.tv_usec) * 1e-6;
#endif
}

template<class Fn1, class Fn2>
void test_impl(const char* func_name, Fn2 fn2)
{
	Fn1 fn1;

	const double beg_time = get_time();
	fn1.Assemble();
	const double end_time = get_time();
	g_assemble_time += (end_time - beg_time) * 1000000.0;

	size_t size = fn1.GetCodeSize();

	unsigned char* p1 = (unsigned char*) fn1.GetCode();
	unsigned char* p2 = (unsigned char*) fn2;
	if (*p2 == 0xE9) p2 = (unsigned char*) (p2 + (unsigned long&) *(p2 + 1) + 5);

	for (size_t i = 0; i < size; i++) {
		if (p1[i] != p2[i]) {
			size_t min_i = i > 10 ? i - 10 : 0;
			size_t max_i = min_i + 20 < size ? min_i + 20 : size;

			printf("<%s> ... failed (dump %d-%d)\n", func_name, min_i, max_i);

			printf("    expected -> ");
			for (size_t j = min_i; j < max_i; j++) printf("%02X,", p2[j]);
			printf("\n");

			printf("      actual -> ");
			for (size_t j = min_i; j < max_i; j++) printf("%02X,", p1[j]);
			printf("\n");

			printf("               %*c^^\n", (i - min_i) * 3 + 1, ' ');

			g_test_failed++;
			return;
		}
	}
	g_test_succeeded++;
}

template<class T>
void test_equal_impl(const char* func_name, T actual, T expected)
{
	if (actual == expected) {
		g_test_succeeded++;
	} else {
		std::cout << "<" << func_name << "> ... failed.";
		std::cout << " expected: " << expected;
		std::cout << " actual: " << actual << std::endl;
		g_test_failed++;
	}
}
