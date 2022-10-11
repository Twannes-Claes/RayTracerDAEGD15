#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float A{}, B{}, C{}, D{};

			A = Vector3::Dot(ray.direction, ray.direction);

			B = Vector3::Dot(2 * ray.direction, ray.origin-sphere.origin);

			C = Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - (sphere.radius * sphere.radius);

			D = (B * B) - (4 * A * C);

			//remaking the code because there is way tooooo many ifs, it could be way efficienter

			//first calculating if it doesnt hit anything or the things it hit are negative or not allowed in case return false the rest you can return true and assign 
			// the hitrecord

			if (D < 0)
			{
				return false;
			}

			D = sqrtf(D);

			float t{ (-B - D) / (2 * A) };
			//check if first intersection is allowed
			if (t < ray.min || t > ray.max)
			{
				//if not check the second one
				t = (-B + D) / (2 * A);

				if (t < ray.min || t > ray.max)
				{
					//both arent allowed so return false
					return false;
				}
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
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
			float t{};

			t = (Vector3::Dot(plane.origin - ray.origin,plane.normal)) / (Vector3::Dot(ray.direction, plane.normal));

			if (t > ray.min && t < ray.max)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
				hitRecord.normal = plane.normal;
			}
			
			return hitRecord.didHit;
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
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
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