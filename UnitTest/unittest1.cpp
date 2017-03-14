#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef unsigned long long UINT64;
typedef unsigned long  UINT32;
typedef signed long  SINT32;


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


float fdiv(
	const float z,             //!<[I ]:dividend
	const float d)             //!<[I ]:divsor
{
//	return z / d;
	SINT32 _z = *(SINT32*)&z;
	SINT32 _d = *(SINT32*)&d;

	UINT32 _z_sign = _z & 0x80000000;
	UINT32 _d_sign = _d & 0x80000000;
	SINT32 _z_exp = (_z >> 23) & 0xff;
	SINT32 _d_exp = (_d >> 23) & 0xff;
	UINT64 _z_fra = (_z & ((1 << 23) - 1)) + (1 << 23);
	UINT32 _d_fra = (_d & ((1 << 23) - 1)) + (1 << 23);

	SINT32 _q_exp = _z_exp - _d_exp + 127;
	UINT32 _q_fra = (UINT32) ((_z_fra << 23) / _d_fra);
	UINT32 _q_sign = _z_sign ^ _d_sign;

	SINT32 _q = _q_sign |  (_q_exp << 23) | (_q_fra - (1 << 23));

	return *(float*)&_q;

}




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
			float z = 100.0f;
			float d = 1.0f;
			Assert::AreEqual(z / d, fdiv(z, d));
		}

		TEST_METHOD(test_fdiv2)
		{
			float z = 3.14f;
			float d = 2.718f;
			Assert::AreEqual(z / d, fdiv(z, d));
		}

		TEST_METHOD(test_fdiv3)
		{
			float z = 1.0f;
			float d = -1.0f;
			Assert::AreEqual(z / d, fdiv(z, d));
		}

	};
}