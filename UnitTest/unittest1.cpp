#include "stdafx.h"
#include "CppUnitTest.h"
#include <random>

#include <locale> 
#include <codecvt> 
#include <cstdio>
#include <algorithm>
#include <assert.h>

#include <cfenv>
#ifdef _MSC_VER
#pragma fenv_access (on)
#endif


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
	const UINT32 z,             //!<[I ]:dividend
	const UINT32 d)             //!<[I ]:divsor
{
	//試行減算による割り算
	UINT32 q = 0;   //quotient  商
	UINT32 r = z;   //remainder 余
	UINT32 n = 24;  //quotientのbit数

	do {
		n--;
		if ((r >> n) >= d) {
			r -= (d << n);
			q += (1 << n);
		}
	} while (n);
	return q;
}


UINT32 udiv48(
	const UINT64 z,             //!<[I ]:dividend
	const UINT32 d)             //!<[I ]:divsor
{
	//試行減算による割り算
	UINT32 n = 24;  //quotientのbit数
	UINT32 q = 0;   //quotient  商
	UINT64 r = z;   //remainder 余

#if 0
	do {
		n--;
		if ((r >> n) >= d) {
			r -= (((UINT64)d) << n);
			q += (1LL << n);
		}
	} while (n);
#elif 1
	SINT32 rh = r >> 32;
	SINT32 rl = r & 0xFFFFFFFF;
	SINT32 dh = d >> (32 - 24);
	SINT32 dl = d << 24;

	UINT32 _rl = r & 0xFFFFFFFF;
	UINT32 _dl = d << 24;

	assert((UINT32)dl == _dl);
	assert((UINT32)rl == _rl);


	do {
		n--;

		//sht right dh & dl
		dl = (UINT32)dl >> 1;
		_dl = (UINT32)_dl >> 1;
		assert((UINT32)dl == _dl);

		if (dh & 1) dl |= 0x80000000;
		if (dh & 1) _dl |= 0x80000000;
		assert((UINT32)dl == _dl);

		dh = (UINT32)dh >> 1;
		q <<= 1;

		bool rhGTdh = (rh > dh);
		bool rhEQdh = (rh == dh);
		bool rlGEdl = (rl >= dl);
		bool rlLTdl = (rl < dl);
		if ((rl^dl) & 0x80000000) rlGEdl = !rlGEdl;
		if ((rl^dl) & 0x80000000) rlLTdl = !rlLTdl;

		assert(((UINT32)rh > (UINT32)dh) == rhGTdh);
		assert(((UINT32)rh ==(UINT32)dh) == rhEQdh);
		assert(((UINT32)rl >= (UINT32)dl) == rlGEdl);
		assert(((UINT32)rl <  (UINT32)dl) == rlLTdl);


		if (rhGTdh || (rhEQdh && rlGEdl)) {
			rh = rh - dh;
			if (rlLTdl) rh--;
			rl = rl - dl;
			q += 1;

			_rl = _rl - _dl;

			assert((UINT32)rl == _rl);
			assert((UINT32)dl == _dl);


		}
	} while (n);

#elif 1
	UINT32 rh = r >> 32;
	UINT32 rl = r & 0xFFFFFFFF;
	UINT32 dh = d >> (32 - 24);
	UINT32 dl = d << 24;

	do {
		n--;

		//sht right dh & dl
		dl >>= 1;
		if (dh & 1) dl |= 0x80000000;
		dh >>= 1;
		q <<= 1;

		if ((rh > dh) || ((rh == dh) && (rl >= dl))) {
			rh = rh - dh;
			if (rl < dl) rh--;
			rl = rl - dl;
			q += 1;
		}
	} while (n);


#else 

	UINT64 d1 = ((UINT64)d) << n;
	UINT32 rh = r >> 32;
	UINT32 rl = r & 0xFFFFFFFF;
	UINT32 dh = d >> (32 - 24);
	UINT32 dl = d << 24;

	assert(d1 >> 32 == dh);
	assert((d1 & 0xFFFFFFFF) == dl);

	do {
		n--;
		d1 >>= 1;

		//sht right dh & dl
		dl >>= 1;
		if (dh & 1) dl |= 0x80000000;
		dh >>= 1;

		assert(d1 >> 32 == dh);
		assert((d1 & 0xFFFFFFFF) == dl);

		q <<= 1;

		rh = r >> 32;
		rl = r & 0xFFFFFFFF;

		assert(
			((rh > dh) || ((rh == dh) && (rl >= dl))) == 
			(r >= d1)
		);

//		if (r >= d1) {
		if ((rh > dh) || ((rh == dh) && (rl >= dl))) {

			r -= d1;

			rh = rh - dh;
			if (rl < dl) rh--;
			rl = rl - dl;

			assert(rh == (r >> 32));
			assert(rl == (r & 0xFFFFFFFF));

			q += 1;
		}
	} while (n);
#endif
	return q;
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
	UINT32 _z_fra = (_z & ((1 << 23) - 1)) + (1 << 23);
	UINT32 _d_fra = (_d & ((1 << 23) - 1)) + (1 << 23);

	SINT32 _q_exp = _z_exp - _d_exp + 127;
#if 0
	UINT32 _q_fra = ((((UINT64)_z_fra) << 23) / _d_fra);
#elif 1
	UINT32 _q_fra = udiv48(((UINT64)_z_fra) << 23 , _d_fra);
#else //誤差が大きすぎる
	UINT32 _q_fra = udiv24(_z_fra << 8, _d_fra) << (23 -8);
#endif
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

	//	_q_exp = _q_exp < 1 ? 1 : _q_exp > 254 ? 254 : _q_exp;

	if (_q_exp < 1) {
		_q_exp = 0;
		_q_fra = (1 << 23);
	}
	else if(_q_exp >254){
		_q_exp = 254;
		_q_fra = (1 << 24) - 1;
	}



	SINT32 _q = _q_sign |  (_q_exp << 23) | (_q_fra - (1 << 23));
	float q = *(float*)&_q;

	return q;

}


float ffloor(const float x) {
	int _x = *(int*)&x;
//	if (_x == 0 || _x == 0x80000000) return 0;
	if (_x == 0x80000000) _x = 0;

	int e = ((_x >> 23) & 0xff) - 127;
	if (e < 0) {
		_x = 0;
	}
	else {
		int msk = 0xFF800000;
		msk >>= e;
		_x &= msk;
	}

	float ans = *(float*)&_x;
	if (x < 0) ans -= 1.0f;
	return ans;
}

#if 0
float fexp(
	const float x             //!<[I ]:dividend
)
{
//	const float LOG2 = log(2.0f);
	const float LOG2 = 0.693147182f;
	const float DIVLOG2 = 1.0f/ LOG2;
	float a = x * DIVLOG2;
#if 0
	a -= (a < 0);
	int n = (int)a;
	float b = x - (float)n * LOG2;
#else
//	float fa = std::floor(a);
	float fa = ffloor(a);
	float b = x - fa * LOG2;
	int n = (int)fa;

#endif
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
#else
float fexp(
	const float x             //!<[I ]:dividend
)
{
	//丸めモードの指定
	if (std::fegetround() != FE_TOWARDZERO)	std::fesetround(FE_TOWARDZERO);		// ゼロ方向への丸め

	//	const float LOG2 = log(2.0f);
	const float LOG2 = 0.693147182f;
	const float DIVLOG2 = 1.44269502f; // 1.0f / LOG2;
	float a = x * DIVLOG2;
	if (x < 0) a -= 1.0f;

	int _a = *(int*)&a;
	int n = (_a & ((1 << 23) - 1)) + (1 << 23);
	int e = ((_a >> 23) & 0xff) - 127;
	if (e < 0) {
		_a = 0;
		n = 0;
	}
	else {
		int sft = 23 - e;
		sft = sft < 0 ? 0 : sft;
		_a &= ~((1 << sft) - 1);
		n >>= sft;
	}
	if (x < 0) n *=-1;
	
	float fa = *(float*)&_a;
	float b = x - fa * LOG2;


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
	y += 1.0f;

	//	float c = pow(2.0,n);
	n += 127;
	//if (n < 1) {
	//	y = 0;
	//	n = 0;
	//}
	//else if (n > 254) {
	//	n = 255;
	//	y = 1.0;
	//}
	n = n < 1 ? 0 : n > 254 ? 254: n;
	int _c = n << 23;
	float c = *(float*)&_c;

	return y * c;
}
#endif


float ftanh(
	const float x
)
{
	float exp2x = fexp(2 * x);
	return fdiv( exp2x - 1.0f , exp2x + 1.0f);
}

int toInt(const float x) {
	int _x = *(int*)&x;
	if (_x == 0 || _x == 0x80000000) return 0;

	int e = ((_x >> 23) & 0xff) - 127;
	int s = _x & (1<<31);
	int f = (_x & ((1 << 23) - 1)) + (1 << 23);

	e -= 23;
	if (31 > e && e >= 0) {
		f <<= e;
	}
	else if(-31 < e){
		f >>=-e;
	}
	else f = 0;

	return (s) ? -f : f;
}


//---------------------------------------------------------------------------
//popcount
int popcount(unsigned int bits)
{
	bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
	bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
	bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
	bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
	return (bits & 0x0000ffff) + (bits >> 16 & 0x0000ffff);
}

//---------------------------------------------------------------------------
//count leading zeros
int CLZ(unsigned int x) {
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return popcount(~x);
}


float toFloat(const int x) {
	if (x == 0) return 0.0f;
	int sgn = x < 0 ? 1 << 31 : 0;
	int fra = x < 0 ? -x : x ;
	int clz = CLZ(fra);
	int sft = 24 - 32 + clz;
	if (sft >= 0) {
		fra <<= sft;
	}
	else {
		fra >>= -sft;
	}
	int exp = 23 - sft;
	exp += 127;
	exp &= 0xff;

	fra -= 1 << 23;
	fra &= (1 << 23) - 1;

	int ret = sgn | (exp << 23) | fra;
	return *(float*)&ret;
}



#define USE_ULPS


bool ucmple(int8_t a, int8_t b) {
#if 0
	return (uint8_t)a <= (uint8_t)b;
#elif 0
	return ((a ^ b) & 0x80) ?  a > b : a <= b;
#elif 0
	return ((a ^ b) & 0x80) ? b <= a : a <= b;
#elif 1
	return ((a ^ b) & 0x80) ? !(a <= b) : a <= b;
#else
	if ((a ^ b) & 0x80) {
		a = a ^ b;
		b = a ^ b;
		a = a ^ b;
	}
	return a <= b;
#endif
}

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(test_udiv32)
		{
			std::default_random_engine engine;
			std::uniform_int_distribution<UINT32> dist(1, INT_MAX);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				UINT32 z = dist(engine);
				UINT32 d = dist(engine);
				d >>= 16;
//				if (d > z) std::swap(z,d);

				UINT32 expect = z / d;
				UINT32 actual = udiv32(z, d);

				std::wstring msg = format_wstr("z:%d d:%d",z ,d);
				Assert::AreEqual(expect, actual, msg.c_str());
			}

		}

		TEST_METHOD(test_udiv32_0)
		{
			// TODO: テスト コードをここに挿入します
			UINT32 z = 100;
			UINT32 d = 10;
			std::wstring msg = format_wstr("z:%d d:%d", z, d);
			Assert::AreEqual(z/d, udiv32(z,d), msg.c_str());

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
//			float z = 1.0f;
//			float d = 3.0f;
//			float d = 0.808740497f;
#if 0
			int zi = 0x3f20b1a0;
			int di = 0x3f4b5ecb;
			float z = *(float*)&zi;
			float d = *(float*)&di;
#else
//			float z = 0.0;
//			float d = FLT_MAX;
//			float z = FLT_MIN;
			float z = FLT_EPSILON;
			float d = FLT_MAX;
#endif

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
			std::default_random_engine engine;
//			std::uniform_real_distribution<float> dist(-1000.0, 1000.0);
			std::uniform_real_distribution<float> dist(-FLT_MAX/2, FLT_MAX/2);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float z = dist(engine);
				float d;
				do { d = dist(engine);}
				while (std::fabs(d) < FLT_MIN);


				float expect = z / d;
				float actual = fdiv(z, d);
#ifndef  USE_ULPS
				Assert::AreEqual(expect, actual);
#else
				int ulps = ULPs(expect, actual);
				std::wstring msg = format_wstr("z:%f d:%f expect:%f actual:%f ULPs:%d", z, d, expect, actual, ulps);
				Assert::IsTrue(ulps <= 2, msg.c_str());
//				Assert::IsTrue(ulps <= 100, msg.c_str());
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
//			float x = -1.0f;
			float x = -FLT_MAX;
//			float x = FLT_EPSILON;
			float delta = exp(x) * 0.001f;
			Assert::AreEqual(exp(x), fexp(x), delta);
		}


		TEST_METHOD(test_fexp)
		{
			std::default_random_engine engine;
			std::uniform_real_distribution<float> dist(-1000.0, 1000.0);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float x = dist(engine);
				float expect = exp(x);
				float actual = fexp(x);
				std::wstring msg = format_wstr("exp(%f)", x);
#if 1
				float delta = std::max(expect * 0.00001f , FLT_MIN * 4);
				Assert::AreEqual(expect, actual, delta , msg.c_str());
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


		TEST_METHOD(test_toInt)
		{
			std::default_random_engine engine;
			std::uniform_real_distribution<float> dist(-10.0, 10.0);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float x = dist(engine);
				//float x = 0.000448226929;
				int expect = (int)x;
				int actual = toInt(x);
				Assert::AreEqual(expect, actual);
			}
		}


		TEST_METHOD(test_toFloat)
		{
			std::default_random_engine engine;
			std::uniform_int_distribution<int> dist(INT_MIN, INT_MAX);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				int x = dist(engine);
//				int x = 1;
				float expect = (float)x;
				float actual = toFloat(x);
#if 0
				Assert::AreEqual(expect, actual);
#else
				int ulps = ULPs(expect, actual);
				std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
				Assert::IsTrue(ulps <= 1, msg.c_str());
#endif
			}
		}

		TEST_METHOD(test_ffloor)
		{
			std::default_random_engine engine;
			std::uniform_real_distribution<float> dist(-10.0, 10.0);

			int N = 1000;
			for (int n = 0; n < N; n++) {
				float x = dist(engine);
				//				int x = 1;
				std::wstring msg = format_wstr("at %f", x);
				float expect = std::floor(x);
				float actual = ffloor(x);
#if 1
				Assert::AreEqual(expect, actual,msg.c_str());
#else
				int ulps = ULPs(expect, actual);
				std::wstring msg = format_wstr("expect:%f actual:%f ULPs:%d", expect, actual, ulps);
				Assert::IsTrue(ulps <= 1, msg.c_str());
#endif
			}
		}


		TEST_METHOD(test_ffloor1)
		{
			{
				float x = 1.0;
				float expect = std::floor(x);
				float actual = ffloor(x);
				Assert::AreEqual(expect, actual);
			}
		}
		TEST_METHOD(test_ffloor2)
		{
			{
				float x = 1.5;
				float expect = std::floor(x);
				float actual = ffloor(x);
				Assert::AreEqual(expect, actual);
			}
		}
		TEST_METHOD(test_ffloor3)
		{
			{
				float x = -1.5;
				float expect = std::floor(x);
				float actual = ffloor(x);
				Assert::AreEqual(expect, actual);
			}
		}
		TEST_METHOD(test_ffloor4)
		{
			{
				float x = -0;
				float expect = std::floor(x);
				float actual = ffloor(x);
				Assert::AreEqual(expect, actual);
			}
		}
		TEST_METHOD(test_ffloor5)
		{
			{
				float x = -0.5;
				float expect = std::floor(x);
				float actual = ffloor(x);
				Assert::AreEqual(expect, actual);
			}
		}


		TEST_METHOD(test_ucmple)
		{
			{
				uint8_t  ua;
				uint8_t  ub;

				ua = 0x00; ub = 0x00;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua,ub).c_str());

				ua = 0x00; ub = 0x7f;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x00; ub = 0x80;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x00; ub = 0xff;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x7f; ub = 0x00;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x7f; ub = 0x7f;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x7f; ub = 0x80;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x7f; ub = 0xff;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0x80; ub = 0xff;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

				ua = 0xff; ub = 0xff;
				Assert::AreEqual(ua <= ub, ucmple(ua, ub), format_wstr("a:0x%02x b:0x%02x", ua, ub).c_str());

			}
		}


	};
}