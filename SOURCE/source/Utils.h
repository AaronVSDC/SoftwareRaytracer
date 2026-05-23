#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>
namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//Defining a, b, and c to plug in to the quadratic formula
			float a = Vector3::Dot(ray.direction, ray.direction); 
			float b = Vector3::Dot(2.f * ray.direction, ray.origin - sphere.origin); 
			float c = Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - sphere.radius * sphere.radius;

			//Discriminant is B^2 - 4ac
			float discriminant = b * b - 4.f * a * c; 
			
			if (discriminant > 0)
			{		


				float t = (-b - sqrtf(discriminant)) / (2.f * a);

				if (t > ray.min)
				{
					if (t < ray.max)
					{
						hitRecord.didHit = true;
						hitRecord.materialIndex = sphere.materialIndex;
						hitRecord.t = t;
						hitRecord.origin = ray.origin + ray.direction * t;
						hitRecord.normal = Vector3{ sphere.origin, ray.origin + ray.direction * t }.Normalized();
	

						return true;
					}
					else
					{
						hitRecord.didHit = false;
						return false;
					}

					}
				else
				{
				
					float t2 = (-b + sqrtf(discriminant)) / (2.f * a);

					if (t2 > ray.min and t2 < ray.max)
					{


						hitRecord.didHit = true;
						hitRecord.materialIndex = sphere.materialIndex;
						hitRecord.t = t2;
						hitRecord.origin = ray.origin + ray.direction * t2;	
						hitRecord.normal = Vector3{ sphere.origin, ray.origin + ray.direction * t2 }.Normalized();
						return true;
					}
					else
					{
						hitRecord.didHit = false;
						return false;
					}
				}
			}
			else
			{
				hitRecord.didHit = false;
				return false; 
			}
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
			//todo W1
			float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal); 
			if (t > ray.min and t < ray.max)
			{
				hitRecord.t = t; 
				hitRecord.didHit = true; 
				hitRecord.materialIndex = plane.materialIndex; 
				hitRecord.origin = ray.origin + t * ray.direction; 
				hitRecord.normal = plane.normal; 
				return true; 
			}
		
			return false; 
			
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
			TriangleCullMode currentCullMode{ triangle.cullMode };
			if (ignoreHitRecord)
			{
				switch (currentCullMode)
				{
				case dae::TriangleCullMode::FrontFaceCulling:
					currentCullMode = TriangleCullMode::BackFaceCulling;
					break;
				case dae::TriangleCullMode::BackFaceCulling:
					currentCullMode = TriangleCullMode::FrontFaceCulling;
					break;
				}
			}
			switch (currentCullMode)
			{
			case TriangleCullMode::BackFaceCulling:
			{
				if (Vector3::Dot(ray.direction, triangle.normal) > 0)
				{
					return false;
				}

				break;
			}
			case TriangleCullMode::FrontFaceCulling:
			{
				if (Vector3::Dot(triangle.normal, ray.direction) < 0)
				{
					return false;
				}

				break;
			}
			}

			const Vector3 L{ triangle.v0 - ray.origin };
			const float t = Vector3::Dot(L, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal);
			if (t < ray.min or t > ray.max) return false; 

			Vector3 intersectPoint = ray.origin + ray.direction * t; 

			if (not(Vector3::Dot(Vector3::Cross(triangle.v1 - triangle.v0, intersectPoint - triangle.v0), triangle.normal) > 0 and
				Vector3::Dot(Vector3::Cross(triangle.v2 - triangle.v1, intersectPoint - triangle.v1), triangle.normal) > 0 and
				Vector3::Dot(Vector3::Cross(triangle.v0 - triangle.v2, intersectPoint - triangle.v2), triangle.normal) > 0))
			{
				return false; 
			}


			if (ignoreHitRecord) return true; 

			hitRecord.didHit = true; 
			hitRecord.materialIndex = triangle.materialIndex; 
			hitRecord.normal = triangle.normal; 
			hitRecord.origin = intersectPoint; 
			hitRecord.t = t; 
			return true; 



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
			HitRecord tempHit{};
			bool hasHit{};
			Triangle tempTriangle{};

			tempTriangle.cullMode = mesh.cullMode;
			tempTriangle.materialIndex = mesh.materialIndex;

			for (size_t index{}; index < mesh.indices.size(); index += 3)
			{

				tempTriangle.v0 = mesh.transformedPositions[mesh.indices[index]];
				tempTriangle.v1 = mesh.transformedPositions[mesh.indices[index + 1]];
				tempTriangle.v2 = mesh.transformedPositions[mesh.indices[index + 2]];
				tempTriangle.normal = mesh.transformedNormals[index / 3];


				if (!HitTest_Triangle(tempTriangle, ray, tempHit, ignoreHitRecord)) continue;


				if (ignoreHitRecord) return true;


				if (hitRecord.t > tempHit.t)
				{
					hitRecord = tempHit;
				}

				hasHit = true;
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
			//todo W3

			switch (light.type)
			{
			case LightType::Point:
			{			
				return 
				{	light.origin.x - origin.x,
					light.origin.y - origin.y,
					light.origin.z - origin.z
				}; 
				break; 
			}
			case LightType::Directional:
			{
				return -light.direction;
				break; 
			}
			}

			return {};


		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3

			switch (light.type)
			{
				case LightType::Point:
				{
					Vector3 directionToLight = GetDirectionToLight(light, target);
					return light.color * light.intensity / directionToLight.SqrMagnitude();
				}
				case LightType::Directional:
				{
					return light.color * light.intensity;
				}
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