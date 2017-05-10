﻿#include "stdafx.h"
#include "CppUnitTest.h"
#include <random>

#include <locale> 
#include <codecvt> 
#include <cstdio>

#include <assert.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef unsigned long long UINT64;
typedef unsigned long  UINT32;
typedef signed long  SINT32;

#include <string>
#include <cstdarg>
std::string format_str(const char *fmt, ...) {
	static char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return std::string(buf);
}

std::wstring format_wstr(const char *fmt, ...) {
	static char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	return cv.from_bytes(buf);
}





int ULPs(float a, float b) {
	int aInt = *(int*)&a;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&b;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	return abs(aInt - bInt);
}


UINT32 udiv32(
	const UINT32 z,             //!<[I ]:dividend
	const UINT32 d)             //!<[I ]:divsor
{
	//試行減算による割り算
	UINT32 q = 0;   //quotient  商
	UINT32 r = z;   //remainder 余
	UINT32 n = 32;  //quotientのbit数

	do {
		n--;
		if ((r >> n) >= d) {
			r -= (d << n);
			q += (1 << n);
		}
	} while (n);
	return q;
}


UINT32 udiv24(
	const UINT64 z,             //!<[I ]:dividend
	const UINT64 d)             //!<[I ]:divsor
{
	//試行減算による割り算
	UINT64 q = 0;   //quotient  商
	UINT64 r = z;   //remainder 余
	UINT32 n = 24;  //quotientのbit数

	do {
		n--;
		if ((r >> n) >= d) {
			r -= (d << n);
			q += (1LL << n);
		}
	} while (n);
	return (UINT32)q;
}

//	return z / d;
float fdiv(
	const float z,             //!<[I ]:dividend
	const float d)             //!<[I ]:divsor
{
	SINT32 _z = *(SINT32*)&z;
	SINT32 _d = *(SINT32*)&d;

	if (_z == 0 || _z == 0x80000000) return z;

	UINT32 _z_sign = _z & 0x80000000;
	UINT32 _d_sign = _d & 0x80000000;
	SINT32 _z_exp = (_z >> 23) & 0xff;
	SINT32 _d_exp = (_d >> 23) & 0xff;
	UINT64 _z_fra = (_z & ((1 << 23) - 1)) + (1 << 23);
	UINT32 _d_fra = (_d & ((1 << 23) - 1)) + (1 << 23);

	SINT32 _q_exp = _z_exp - _d_exp + 127;
//	UINT32 _q_fra = (UINT32) ((_z_fra << 23) / _d_fra);
	UINT32 _q_fra = udiv24((_z_fra << 23) , _d_fra);
	UINT32 _q_sign = _z_sign ^ _d_sign;

	//normalize
	assert((_q_fra & 0xff000000) == 0);
#if 0
	while ((_q_fra & (1 << 23)) == 0) {
		_q_fra <<= 1;
		_q_exp--;
	}
#else
	if ((_q_fra & (1 << 23)) == 0) {
		_q_fra <<= 1;
		_q_exp--;
	}
	assert(_q_fra & (1 << 23));
#endif

	SINT32 _q = _q_sign |  (_q_exp << 23) | (_q_fra - (1 << 23));
	float q = *(float*)&_q;

	return q;

}


float fexp(
	const float x             //!<[I ]:dividend
)
{
//	const float LOG2 = log(2.0f);
	const float LOG2 = 0.693147182f;
	const float DIVLOG2 = 1.0f/ LOG2;
	float a = x * DIVLOG2;
	a -= (a < 0);
	int n = (int)a;
	float b = x - (float)n * LOG2;
	float y;
	//horner scheme 5 th order 
	y = 1.185268231308989403584147407056378360798378534739e-2f;
	y *= b;
	y += 3.87412011356070379615759057344100690905653320886699e-2f;
	y *= b;
	y += 0.16775408658617866431779970932853611481292418818223f;
	y *= b;
	y += 0.49981934577169208735732248650232562589934399402426f;
	y *= b;
	y += 1.00001092396453942157124178508842412412025643386873f;
	y *= b;
//	y += 0.99999989311082729779536722205742989232069120354073f;
	y += 1.0f;

	//	float c = pow(2.0,n);
	int _c = (n + 127) << 23;
	float c = *(float*)&_c;

	return y * c;
}

float ftanh(
	const float x
)
{
	float exp2x = fexp(2 * x);
	return fdiv( exp2x - 1.0f , exp2x + 1.0f);
}


#define USE_ULPS


namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(test_udiv32)
		{
			// TODO: テスト コードをここに挿入します

			UINT32 z = 100;
			UINT32 d = 10;

			Assert::AreEqual(z/d, udiv32(z,d));

		}

		TEST_METHOD(test_fdiv1)
		{
			float z = 1.0f;
			float d = 1.0f;
			float expect = z / d;
			float actual = fdiv(z, d);
#ifndef  USE_ULPS
			Assert::AreEqual(expect, actual);
#else
			int ulps = ULPs(expect, actual);
			std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
			Assert::IsTrue(ulps <= 2, msg.c_str());
#endif
		}

		TEST_METHOD(test_fdiv2)
		{
			float z = 1.0f;
			float d = -1.0f;
			float expect = z / d;
			float actual = fdiv(z, d);
#ifndef  USE_ULPS
			Assert::AreEqual(expect, actual);
#else
			int ulps = ULPs(expect, actual);
			std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
			Assert::IsTrue(ulps <= 2, msg.c_str());
#endif
		}

		TEST_METHOD(test_fdiv3)
		{
//			float z = 0.193304241f;
			float z = 0.0f;
			float d = 0.808740497f;
			float expect = z / d;
			float actual = fdiv(z, d);
#ifndef  USE_ULPS
			Assert::AreEqual(expect, actual);
#else
			int ulps = ULPs(expect, actual);
			std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
			Assert::IsTrue(ulps <= 2, msg.c_str());
#endif

		}

		TEST_METHOD(test_fdiv)
		{
			int N = 1000;
			for (int n = 0; n < N; n++) {
				float z = (float)rand() / (float)RAND_MAX;
				float d = (float)rand() / (float)RAND_MAX;
				float expect = z / d;
				float actual = fdiv(z, d);
#ifndef  USE_ULPS
				Assert::AreEqual(expect, actual);
#else
				int ulps = ULPs(expect, actual);
				std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
				Assert::IsTrue(ulps <= 2, msg.c_str());
#endif
			}
		}


		TEST_METHOD(test_fexp1)
		{
			float x = 1.0f;
			float delta = exp(x) * 0.001f;
			Assert::AreEqual(exp(x), fexp(x), delta);
		}

		TEST_METHOD(test_fexp2)
		{
			float x = 0.0f;
			float delta = exp(x) * 0.001f;
			Assert::AreEqual(exp(x), fexp(x), delta);
		}

		TEST_METHOD(test_fexp3)
		{
			float x = -1.0f;
			float delta = exp(x) * 0.001f;
			Assert::AreEqual(exp(x), fexp(x), delta);
		}

		TEST_METHOD(test_fexp)
		{
			std::default_random_engine engine;
			std::uniform_real_distribution<float> dist(-10.0, 10.0);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float x = dist(engine);
				float expect = exp(x);
				float actual = fexp(x);
#if 1
				float delta = expect * 0.00001f;
				Assert::AreEqual(expect, actual, delta);
#else
				float expect = exp(x);
				float actual = fexp(x);
				int ulps = ULPs(expect, actual);
				std::string msg = format_str("expect:%f actual:%f ULPs:%d", expect, actual,ulps);
				std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
				Assert::IsTrue(ulps <= 8, cv.from_bytes(msg).c_str());
#endif
			}
		}

		TEST_METHOD(test_ftanh1)
		{
			float x = 0.0;
			float expect = tanh(x);
			float actual = ftanh(x);
//#ifndef  USE_ULPS
#if 1
			float delta = expect * 0.001f;
			Assert::AreEqual(expect, actual, delta);
#else
			int ulps = ULPs(expect, actual);
			std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
			Assert::IsTrue(ulps <= 2, msg.c_str());
#endif

		}


		TEST_METHOD(test_ftanh)
		{
			std::default_random_engine engine;
			std::uniform_real_distribution<float> dist(-10.0, 10.0);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float x = dist(engine);
				float expect = tanh(x);
				float actual = ftanh(x);
//#ifndef  USE_ULPS
#if 1
				float delta = expect * 0.001f;
				Assert::AreEqual(expect, actual, delta);
#else
				int ulps = ULPs(expect, actual);
				std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
				Assert::IsTrue(ulps <= 16, msg.c_str());
#endif

			}
		}

	};
}