#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
			//center = (v0 + v1 + v2) / 3;
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		//Vector3 center{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct BVHNode
	{
		Vector3 minAABB{};
		Vector3 maxAABB{};
		size_t leftNode{}, firstIndice{}, IndiceCount{};
		bool IsLeaf() const { return IndiceCount > 0; }
	};

	struct AABB
	{
		Vector3 min{ Vector3::Identity * FLT_MAX }, max{ Vector3::Identity * FLT_MIN };

		void Grow(const Vector3& p)
		{ 
			min = Vector3::Min(min, p);
			max = Vector3::Max(max, p);
		}

		void Grow(const AABB& bounds)
		{
			min = Vector3::Min(min, bounds.min);
			max = Vector3::Max(max, bounds.max);
		}

		float Area() const 
		{
			const Vector3 e{ max - min };
			return e.x * e.y + e.y * e.z + e.z * e.x;
		}
	};

	struct BIN
	{
		AABB bounds{};

		int indiceCount{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB{};
		Vector3 maxAABB{};

		Vector3 transformedMinAABB{};
		Vector3 transformedMaxAABB{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		BVHNode* pBVHNode{};

		size_t rootNodeIdx{};
		size_t nodesUsed{};

		~TriangleMesh()
		{
			delete[] pBVHNode;
		}

		void BuildBVH()
		{
			//initialize root tree
			nodesUsed = 0;
			BVHNode& root = pBVHNode[rootNodeIdx];
			root.leftNode = 0;
			root.firstIndice = 0;
			root.IndiceCount = static_cast<size_t>(indices.size());

			UpdateBVHNodeBounds(rootNodeIdx);

			Subdivide(rootNodeIdx);
		}

		void UpdateBVHNodeBounds(size_t nodeIdx)
		{
			BVHNode& node{ pBVHNode[nodeIdx] };

			node.minAABB = Vector3::Identity * FLT_MAX;
			node.maxAABB = Vector3::Identity * FLT_MIN;

			for (size_t i {node.firstIndice}; i < node.firstIndice + node.IndiceCount; ++i)
			{
				const Vector3& curVertex{ transformedPositions[indices[i]] };

				node.minAABB = Vector3::Min(node.minAABB, curVertex);
				node.maxAABB = Vector3::Max(node.maxAABB, curVertex);
			}
		}

		inline float FindBestSplitPlane(BVHNode& node, int& axis, float& splitPos)
		{

			float bestCost{ FLT_MAX };

			for (int a = 0; a < 3; ++a)
			{

				float boundsMin{ FLT_MAX };
				float boundsMax{ FLT_MIN };

				for (size_t i{}; i < node.IndiceCount; i += 3)
				{
					const size_t indicePlusI{ node.firstIndice + i };

					const Vector3 center{ (transformedPositions[indices[indicePlusI]] + transformedPositions[indices[indicePlusI + 1]] + transformedPositions[indices[indicePlusI + 2]]) / 3 };

					boundsMin = std::min(center[a], boundsMin);
					boundsMax = std::max(center[a], boundsMax);
				}

				if (AreEqual(boundsMin, boundsMax)) continue;

				const int nrOfBins{ 8 };

				const int binsMin1{ nrOfBins - 1 };

				BIN bins[nrOfBins];

				float scale{ nrOfBins / (boundsMax - boundsMin) };

				for (size_t i{}; i < node.IndiceCount; i += 3)
				{

					const size_t indicePlusI{ node.firstIndice + i };

					const Vector3& v0{ transformedPositions[indices[indicePlusI]] };
					const Vector3& v1{ transformedPositions[indices[indicePlusI + 1]] };
					const Vector3& v2{ transformedPositions[indices[indicePlusI + 2]] };

					const Vector3 center{ (v0 + v1 + v2) / 3 };

					const int binIdx{ std::min(binsMin1, static_cast<int>((center[a] - boundsMin) * scale)) };

					bins[binIdx].indiceCount += 3;
					bins[binIdx].bounds.Grow(v0);
					bins[binIdx].bounds.Grow(v1);
					bins[binIdx].bounds.Grow(v2);
				}

				float leftArea[binsMin1]{}, rightArea[binsMin1]{};
				int leftCount[binsMin1]{}, rightCount[binsMin1]{};
				AABB leftBox, rightBox;
				int leftSum = 0, rightSum = 0;

				for (size_t i{}; i < binsMin1; ++i)
				{
					leftSum += bins[i].indiceCount;
					leftCount[i] = leftSum;
					leftBox.Grow(bins[i].bounds);
					leftArea[i] = leftBox.Area();

					const size_t nrBins1minI{ binsMin1 - i };
					const size_t nrBins2minI{ nrBins1minI - 1 };

					rightSum += bins[nrBins1minI].indiceCount;
					rightCount[nrBins2minI] = rightSum;
					rightBox.Grow(bins[nrBins1minI].bounds);
					rightArea[nrBins2minI] = rightBox.Area();
				}

				scale = (boundsMax - boundsMin) / nrOfBins;

				for (size_t i{}; i < binsMin1; ++i)
				{
					const float planeCost{ leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i] };

					if (planeCost < bestCost)
					{
						axis = a;
						splitPos = boundsMin + scale * (i + 1);
						bestCost = planeCost;
					}

				}

			}

			return bestCost;

		}

		float CalculateNodeCost(BVHNode& node)
		{
			const Vector3 e = node.maxAABB - node.minAABB; // extent of the node
			const float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
			return node.IndiceCount * surfaceArea;
		}

		void Subdivide(size_t nodeIdx)
		{
			BVHNode& node{ pBVHNode[nodeIdx] };
			
			if (node.IndiceCount <= 6) return;

			//determine split axis using SAH

			int axis{};
			float splitPos{};
			const float cost{ FindBestSplitPlane(node, axis, splitPos) };

			const float noSplitCost{ CalculateNodeCost(node) };

			if (noSplitCost <  cost) return;

			//in place partition

			size_t i { (node.firstIndice) };
			size_t j { i + (node.IndiceCount) - 1 };


			while(i <= j)
			{

				const Vector3 center{ (transformedPositions[indices[i]] + transformedPositions[indices[i + 1]] + transformedPositions[indices[i + 2]]) / 3 };

				if (center[axis] < splitPos)
				{
					i += 3;
				}
				else
				{

					const size_t jMinus2{ j - 2 };
					const size_t jMinus2Divide3{ jMinus2 / 3 };

					const size_t iDivide3{ i / 3 };

					std::swap(indices[i], indices[jMinus2]);
					std::swap(indices[i + 1], indices[j - 1]);
					std::swap(indices[i + 2], indices[j]);
					std::swap(normals[iDivide3], normals[jMinus2Divide3]);
					std::swap(transformedNormals[iDivide3], transformedNormals[jMinus2Divide3]);

					j -= 3;
				}

			}
			//abort split if one of the sides is empty

			const size_t leftCount { i - node.firstIndice };

			if (leftCount == 0 || leftCount == node.IndiceCount)
			{
				return;
			}

			//create childnodes
			const size_t leftChildIdx{ ++nodesUsed };
			const size_t rightChildIdx{++nodesUsed};

			node.leftNode = leftChildIdx;

			pBVHNode[leftChildIdx].firstIndice = node.firstIndice;
			pBVHNode[leftChildIdx].IndiceCount = leftCount;
			pBVHNode[rightChildIdx].firstIndice = i;
			pBVHNode[rightChildIdx].IndiceCount = node.IndiceCount - leftCount;

			node.IndiceCount = 0;

			UpdateBVHNodeBounds(leftChildIdx);
			UpdateBVHNodeBounds(rightChildIdx);


			Subdivide(leftChildIdx);
			Subdivide(rightChildIdx);
		}

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(const float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{
			normals.clear();
			normals.reserve(indices.size() / 3);

			for (int i{}; i < indices.size();  i +=3)
			{
				Vector3 v0 = positions[indices[i]];

				normals.emplace_back(Vector3::Cross(positions[indices[i + 1]] - v0, positions[indices[i + 2]] - v0).Normalized());
			}
		}

		void UpdateTransforms()
		{
			const Matrix SRT{ scaleTransform  * rotationTransform * translationTransform };
			const Matrix normalRT{ rotationTransform * translationTransform };

			transformedPositions.clear();
			transformedPositions.reserve(positions.size());

			for (int i{}; i < positions.size(); ++i)
			{
				transformedPositions.emplace_back(SRT.TransformPoint(positions[i]));
			}

			transformedNormals.clear();
			transformedNormals.reserve(positions.size());

			for (int i{}; i < normals.size(); ++i)
			{
				transformedNormals.emplace_back(normalRT.TransformVector(normals[i]));
			}

			UpdateTransformedAABB(SRT);
		}

		void UpdateAABB()
		{
			if (positions.size() > 0)
			{
				minAABB = positions[0];
				maxAABB = positions[0];

				for (auto& p : positions)
				{
					minAABB = Vector3::Min(p, minAABB);
					maxAABB = Vector3::Max(p, maxAABB);
				}
			}
		}

		void UpdateTransformedAABB(const Matrix& finalTransform)
		{

			Vector3 tMinAABB = finalTransform.TransformPoint(minAABB);
			Vector3 tMaxAABB = tMinAABB;

			Vector3 tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			transformedMinAABB = tMinAABB;
			transformedMaxAABB = tMaxAABB;

		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };

		Vector3 inverseDirection{1/direction.x, 1 / direction.y , 1 / direction.z };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;
		 
		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}