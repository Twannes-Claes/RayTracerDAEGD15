#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <xmmintrin.h>

namespace dae
{
	namespace Utils
	{
		//https://geometrian.com/programming/tutorials/fastsqrt/index.php

		inline float FastSqrt(float arg)
		{
			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ps1(arg)));
		}
	}

	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//return false;

#pragma region normalSphereTest

			//float A{1}, B{}, C{}, D{};

			//const Vector3 rayMinSphereOrigin{ ray.origin - sphere.origin };

			//A = Vector3::Dot(ray.direction, ray.direction);

			//B = Vector3::Dot(2 * ray.direction, rayMinSphereOrigin);

			//C = Vector3::Dot(rayMinSphereOrigin, rayMinSphereOrigin) - Square(sphere.radius);

			//D = Square(B) - (4 * A * C);

			///*B = Vector3::Dot(2 * ray.direction, rayMinSphereOrigin);

			//C = Vector3::Dot(rayMinSphereOrigin, rayMinSphereOrigin) - Square(sphere.radius);

			//D = Square(B) - (4 * C);*/

			////remaking the code because there is way tooooo many ifs, it could be way efficienter

			////first calculating if it doesnt hit anything or the things it hit are negative or not allowed in case return false the rest you can return true and assign 
			//// the hitrecord

			//if (D < 0)
			//{
			//	return false;
			//}

			//D = sqrtf(D);
			//// 

			//float Atimes2{ 0.5f }; // = A = 1 A/2 = 0.5


			//float t{ (-B - D) * Atimes2 };
			////check if first intersection is allowed
			//if (t < ray.min || t > ray.max)
			//{
			//	//if not check the second one
			//	t = (-B + D) * Atimes2;

			//	if (t < ray.min || t > ray.max)
			//	{
			//		//both arent allowed so return false
			//		return false;
			//	}
			//}

			//if (!ignoreHitRecord)
			//{
			//	hitRecord.didHit = true;
			//	hitRecord.materialIndex = sphere.materialIndex;
			//	hitRecord.origin = ray.origin + (ray.direction * t);
			//	hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			//	hitRecord.t = t;
			//}

			//return true;

#pragma endregion

			const Vector3 ToCenter { sphere.origin - ray.origin };

			const float ray2PointLength { Vector3::Dot(ToCenter, ray.direction) };

			const float center2PointSquared { ToCenter.SqrMagnitude() - Square(ray2PointLength) };

			const float pointToHitPointSquared { Square(sphere.radius) - center2PointSquared };

			if (pointToHitPointSquared < 0) return false;

			const float pointToHitPoint{ Utils::FastSqrt(pointToHitPointSquared) };

			const float t { ray2PointLength - pointToHitPoint };

			if (t < ray.min || t > ray.max) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * t);
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
				hitRecord.t = t;
			}

			return true;

#pragma region Unoptimized code
			/*if (D > FLT_EPSILON)
			{
				D = sqrtf(D);

				float t1{}, t2{};
				t1 = (-B + D) / (2 * A);
				t2 = (-B - D) / (2 * A);

				if (t1 < 0 && t2 < 0)
				{
					return false;
				}

				if (t1 > t2)
				{
					hitRecord.t = t2;
				}
				else
				{
					hitRecord.t = t1;
				}

				if (t1 < 0)
				{
					hitRecord.t = t2;
				}
				else if (t2 < 0)
				{
					hitRecord.t = t1;
				}

				if (hitRecord.t > ray.min && hitRecord.t < ray.max)
				{
					if (ignoreHitRecord)
					{
						return true;
					}

					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();

					return true;
				}
			}
			else if (AreEqual(D, 0.f))
			{

				float t = (-B) / (2 * A);;

				if (t < 0)
				{
					return false;
				}

				if (t > ray.min && t < ray.max)
				{
					if (ignoreHitRecord)
					{
						return true;
					}

					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();

					return true;
				}
			}
			return false;*/
#pragma endregion
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion

#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//return false;

			float t = { (Vector3::Dot(plane.origin - ray.origin, plane.normal)) / (Vector3::Dot(ray.direction, plane.normal)) };

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * t);
				hitRecord.normal = plane.normal;
				hitRecord.t = t;
			}
			
			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//return false;

			float dotNV{ Vector3::Dot(triangle.normal, ray.direction) };

			if (dotNV == 0) { return false; }

			TriangleCullMode cullMode{ triangle.cullMode };


			if (ignoreHitRecord)
			{
				if (cullMode == TriangleCullMode::BackFaceCulling) { cullMode = TriangleCullMode::FrontFaceCulling; }
				else if (cullMode == TriangleCullMode::FrontFaceCulling) { cullMode = TriangleCullMode::BackFaceCulling; }
			}

			switch (cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
			{
				if (dotNV < 0) { return false; }
			}
			break;
			case TriangleCullMode::BackFaceCulling:
			{
				if (dotNV > 0) { return false; }
			}
			break;
			default:
				break;
			}

#pragma region FirstImplimentation
			
			/*Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };

			float t = Vector3::Dot(center - ray.origin, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal);

			if (t < ray.min || t > ray.max) { return false; }

			Vector3 p{ ray.origin + (t * ray.direction) };

			Vector3 poinToSide{ p - triangle.v0 };

			if (Vector3::Dot(triangle.normal, Vector3::Cross(triangle.v1 - triangle.v0, poinToSide)) < 0) { return false; }

			poinToSide = p - triangle.v1;

			if (Vector3::Dot(triangle.normal, Vector3::Cross(triangle.v2 - triangle.v1, poinToSide)) < 0) { return false; }

			poinToSide = p - triangle.v2;

			if (Vector3::Dot(triangle.normal, Vector3::Cross(triangle.v0 - triangle.v2, poinToSide)) < 0) { return false; }

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = p;
				hitRecord.normal = triangle.normal;
				hitRecord.t = t;
			}

			return true;*/

#pragma endregion

			const float EPSILON{ 0.001f };

			Vector3 edge1{}, edge2{}, h{}, s{}, q{};

			float D{}, f{}, u{}, v{};

			edge1 = triangle.v1 - triangle.v0;
			edge2 = triangle.v2 - triangle.v0;

			h = Vector3::Cross(ray.direction, edge2);

			D = Vector3::Dot(edge1, h);

			if (abs(D) < EPSILON) return false;

			f = 1 / D;

			s = ray.origin - triangle.v0;

			u = f * Vector3::Dot(s, h);

			if (u < 0 || u > 1) return false;

			q = Vector3::Cross(s, edge1);

			v = f * Vector3::Dot(ray.direction, q);

			if (v < 0 || u + v > 1) return false;

			float t{ f * Vector3::Dot(edge2,q) };

			if (t < ray.min || t > ray.max) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * t);
				hitRecord.normal = triangle.normal;
				hitRecord.t = t;
			}

			return true;

		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float inverseX { 1 / ray.direction.x};
			float inverseY {1 / ray.direction.y};
			float inverseZ {1 / ray.direction.z};

			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) * inverseX;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) * inverseX;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) * inverseY;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) * inverseY;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) * inverseZ;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) * inverseZ;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//return false;

			if (!SlabTest_TriangleMesh(mesh, ray))
			{ 
				return false; 
			}

			HitRecord tempHit{};

			bool hasHit{ false };

			Triangle tempTriangle{};

			tempTriangle.cullMode = mesh.cullMode;
			tempTriangle.materialIndex = mesh.materialIndex;

			for (int i{}; i < mesh.indices.size(); i+= 3)
			{
				tempTriangle.normal = mesh.transformedNormals[i / 3];

				tempTriangle.v0 = mesh.transformedPositions[mesh.indices[i]];
				tempTriangle.v1 = mesh.transformedPositions[mesh.indices[i + 1]];
				tempTriangle.v2 = mesh.transformedPositions[mesh.indices[i + 2]];

				if (HitTest_Triangle(tempTriangle,ray, tempHit, ignoreHitRecord))
				{
					hasHit = true;

					if (ignoreHitRecord)
					{
						return true;
					}

					if (tempHit.t < hitRecord.t)
					{
						hitRecord = tempHit;
					}
				}
			}

			return hasHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}

#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{

			if (light.type == LightType::Directional)
			{
				return {};
			}
			else
			{
				//Lighttype Point
				return light.origin - origin;
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			
			if (light.type == LightType::Point)
			{
				Vector3 irridanceVector{ light.origin - target };

				return light.color * (light.intensity / Vector3::Dot(irridanceVector, irridanceVector));
			}
			else
			{
				return light.color * light.intensity;
			}
		}
	}

	namespace Utils
	{

		//// Fast Sqrt	Source: https://geometrian.com/programming/tutorials/fastsqrt/index.php
		//inline float FastSqrt(float arg)
		//{
		//	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ps1(arg)));
		//}

		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)

		

	}
}