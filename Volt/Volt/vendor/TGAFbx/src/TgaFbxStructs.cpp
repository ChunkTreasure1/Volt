#include "TGAFbx.pch.h"
#include "TgaFbxStructs.h"

#include <array>

const float& TGA::FBX::Matrix::operator()(const int aRow, const int aColumn) const
{
	return Data[(aRow - 1) * 4 + (aColumn - 1)];
}

float& TGA::FBX::Matrix::operator()(const int aRow, const int aColumn)
{
	return Data[(aRow - 1) * 4 + (aColumn - 1)];
}

float& TGA::FBX::Matrix::operator[](const unsigned& aIndex)
{
	return Data[aIndex];
}

const float& TGA::FBX::Matrix::operator[](const unsigned& aIndex) const
{
	return Data[aIndex];
}

TGA::FBX::Matrix TGA::FBX::Matrix::operator*(const Matrix& aRightMatrix) const
{
	Matrix result;
	const __m128& a = aRightMatrix.m1;
	const __m128& b = aRightMatrix.m2;
	const __m128& c = aRightMatrix.m3;
	const __m128& d = aRightMatrix.m4;

	__m128 t1, t2;

	t1 = _mm_set1_ps((*this)[0]);
	t2 = _mm_mul_ps(a, t1);
	t1 = _mm_set1_ps((*this)[1]);
	t2 = _mm_add_ps(_mm_mul_ps(b, t1), t2);
	t1 = _mm_set1_ps((*this)[2]);
	t2 = _mm_add_ps(_mm_mul_ps(c, t1), t2);
	t1 = _mm_set1_ps((*this)[3]);
	t2 = _mm_add_ps(_mm_mul_ps(d, t1), t2);

	_mm_store_ps(&result[0], t2);

	t1 = _mm_set1_ps((*this)[4]);
	t2 = _mm_mul_ps(a, t1);
	t1 = _mm_set1_ps((*this)[5]);
	t2 = _mm_add_ps(_mm_mul_ps(b, t1), t2);
	t1 = _mm_set1_ps((*this)[6]);
	t2 = _mm_add_ps(_mm_mul_ps(c, t1), t2);
	t1 = _mm_set1_ps((*this)[7]);
	t2 = _mm_add_ps(_mm_mul_ps(d, t1), t2);

	_mm_store_ps(&result[4], t2);

	t1 = _mm_set1_ps((*this)[8]);
	t2 = _mm_mul_ps(a, t1);
	t1 = _mm_set1_ps((*this)[9]);
	t2 = _mm_add_ps(_mm_mul_ps(b, t1), t2);
	t1 = _mm_set1_ps((*this)[10]);
	t2 = _mm_add_ps(_mm_mul_ps(c, t1), t2);
	t1 = _mm_set1_ps((*this)[11]);
	t2 = _mm_add_ps(_mm_mul_ps(d, t1), t2);

	_mm_store_ps(&result[8], t2);

	t1 = _mm_set1_ps((*this)[12]);
	t2 = _mm_mul_ps(a, t1);
	t1 = _mm_set1_ps((*this)[13]);
	t2 = _mm_add_ps(_mm_mul_ps(b, t1), t2);
	t1 = _mm_set1_ps((*this)[14]);
	t2 = _mm_add_ps(_mm_mul_ps(c, t1), t2);
	t1 = _mm_set1_ps((*this)[15]);
	t2 = _mm_add_ps(_mm_mul_ps(d, t1), t2);

	_mm_store_ps(&result[12], t2);
	return result;
}

TGA::FBX::Matrix& TGA::FBX::Matrix::operator*=(const Matrix& aMatrix)
{
	Matrix result;
	for (int i = 1; i <= 4; i++)
	{
		for (auto j = 1; j <= 4; j++)
		{
			float product{ 0 };
			for (auto k = 1; k <= 4; k++)
			{
				product += this->operator()(i, k) * aMatrix(k, j);
			}
			result(i, j) = product;
		}
	}
	std::memcpy(this->Data, result.Data, sizeof(float) * 16);
	return *this;
}

TGA::FBX::Box& TGA::FBX::Box::operator+=(const std::array<float, 3> aVector)
{
	if(IsValid)
	{
		Min[0] = std::min(Min[0], aVector[0]);
		Min[1] = std::min(Min[1], aVector[1]);
		Min[2] = std::min(Min[2], aVector[2]);

		Max[0] = std::max(Max[0], aVector[0]);
		Max[1] = std::max(Max[1], aVector[1]);
		Max[2] = std::max(Max[2], aVector[2]);
	}
	else
	{
		Min[0] = Max[0] = aVector[0];
		Min[1] = Max[1] = aVector[1];
		Min[2] = Max[2] = aVector[2];
		IsValid = true;
	}

	return *this;
}

TGA::FBX::Box TGA::FBX::Box::FromAABB(const std::array<float, 3> anOrigin, const std::array<float, 3> anExtent)
{
	Box result;
	result.Min[0] = anOrigin[0] - anExtent[0];
	result.Min[1] = anOrigin[1] - anExtent[1];
	result.Min[2] = anOrigin[2] - anExtent[2];

	result.Max[0] = anOrigin[0] + anExtent[0];
	result.Max[1] = anOrigin[1] + anExtent[1];
	result.Max[2] = anOrigin[2] + anExtent[2];
	result.IsValid = true;

	return result;
}

TGA::FBX::BoxSphereBounds TGA::FBX::BoxSphereBounds::operator+(const BoxSphereBounds& aBounds) const
{
	BoxSphereBounds result;

	Box boundingBox;

	std::array<float, 3> arg;
	arg[0] = Center[0] - BoxExtents[0];
	arg[1] = Center[1] - BoxExtents[1];
	arg[2] = Center[2] - BoxExtents[2];
	boundingBox += (arg);

	arg[0] = Center[0] + BoxExtents[0];
	arg[1] = Center[1] + BoxExtents[1];
	arg[2] = Center[2] + BoxExtents[2];
	boundingBox += (arg);

	arg[0] = aBounds.Center[0] - aBounds.BoxExtents[0];
	arg[1] = aBounds.Center[1] - aBounds.BoxExtents[1];
	arg[2] = aBounds.Center[2] - aBounds.BoxExtents[2];
	boundingBox += (arg);

	arg[0] = aBounds.Center[0] + aBounds.BoxExtents[0];
	arg[1] = aBounds.Center[1] + aBounds.BoxExtents[1];
	arg[2] = aBounds.Center[2] + aBounds.BoxExtents[2];
	boundingBox += (arg);

	result.Center[0] = 0.5f * (boundingBox.Min[0] + boundingBox.Max[0]);
	result.Center[1] = 0.5f * (boundingBox.Min[1] + boundingBox.Max[1]);
	result.Center[2] = 0.5f * (boundingBox.Min[2] + boundingBox.Max[2]);

	result.BoxExtents[0] = 0.5f * (boundingBox.Max[0] - boundingBox.Min[0]);
	result.BoxExtents[1] = 0.5f * (boundingBox.Max[1] - boundingBox.Min[1]);
	result.BoxExtents[2] = 0.5f * (boundingBox.Max[2] - boundingBox.Min[2]);

	result.Radius = std::max(BoxExtents[0], std::max(BoxExtents[1], BoxExtents[2]));

	return result;
}
