//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <execution>

using namespace dae;

#define PARALLEL_EXECUTION


Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow)),
	ascending{}
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{

	Camera& camera = pScene->GetCamera();
	const Matrix cameraToWorld = camera.CalculateCameraToWorld(); 

	const float aspectRatio = m_Width / static_cast<float>(m_Height); 

	const float fovAngle = camera.fovAngle * TO_RADIANS;



#if defined(PARALLEL_EXECUTION)
	//parallel logic
	 
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) }; 
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels); 
	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index); 
	
	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i)
		{
			RenderPixel(pScene, i, camera.FOV, aspectRatio, cameraToWorld, camera.origin);
		}); 

	

#else
	//Synchronous logic (no threading) 
	uint32_t amountOfPixels{ uint32_t (m_Width * m_Height) }; 

	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; pixelIndex++)
	{
		RenderPixel(pScene, pixelIndex, camera.FOV, aspectRatio, cameraToWorld, camera.origin);
	}


#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);


}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width }; 

	float rx{ px + 0.5f }, ry{ py + 0.5f }; 
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov }; 
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov }; 


	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();


	float pxCamera = (2.f * ((static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width)) - 1.f) * aspectRatio * fov;
	float pyCamera = (1.f - 2.f * ((static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height))) * fov;
	Vector3 viewRayDirection = { pxCamera, pyCamera, 1.f };
	//viewRayDirection.Normalize(); 
	
	//TODO: just use viewraydirection instead of seperate variable worldSpaceDirection
	Vector3 worldSpaceDirection = cameraToWorld.TransformVector(viewRayDirection);
	worldSpaceDirection.Normalize();
	Ray viewRay = Ray{ cameraOrigin , worldSpaceDirection };

	//Geometry hit test
	HitRecord closestHit{};

	//Update Color in Buffer
	ColorRGB finalColor{};
	pScene->GetClosestHit(viewRay, closestHit);
	if (closestHit.didHit)
	{
		for (size_t index{}; index < lights.size(); ++index)
		{



			if (m_ShadowsEnabled) 
			{
				Ray hitTowardsLight{};
				hitTowardsLight.origin = closestHit.origin + closestHit.normal * 0.001f;
				hitTowardsLight.direction = LightUtils::GetDirectionToLight(lights[index], hitTowardsLight.origin);
				hitTowardsLight.max = hitTowardsLight.direction.Magnitude();
				hitTowardsLight.direction.Normalize();

				if (pScene->DoesHit(hitTowardsLight))
				{
					continue;
				}

				switch (m_CurrentLightingMode)
				{
					case LightingMode::ObservedArea:
					{
						float observedArea = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(lights[index], closestHit.origin).Normalized());

						if (observedArea < 0)
						{
							continue;
						}
						finalColor += ColorRGB{ observedArea, observedArea, observedArea };
						break;
					}
					case LightingMode::BRDF:
					{
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, hitTowardsLight.direction, -worldSpaceDirection);
						break;
					}
					case LightingMode::Combined:
					{
						float observedArea = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(lights[index], closestHit.origin).Normalized());

						if (observedArea < 0)
						{
							continue;
						}
						finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin) *
							materials[closestHit.materialIndex]->Shade(closestHit, hitTowardsLight.direction, -worldSpaceDirection) *
							observedArea;
						break;
					}
					case LightingMode::Radiance:
					{
						finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin);
						break;
					}
				}
				
			}
			else
			{


				switch (m_CurrentLightingMode)
				{				
				
					case LightingMode::Radiance: 
					{
						finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin); 
						break;
					}
					case LightingMode::ObservedArea:
					{	float observedArea = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(lights[index], closestHit.origin).Normalized());

						if (observedArea < 0)
						{
							continue;
						}
						finalColor += ColorRGB{ observedArea, observedArea, observedArea };
						break; 
					}
					case LightingMode::BRDF:
					{	
						Ray hitTowardsLight{};
							hitTowardsLight.origin = closestHit.origin + closestHit.normal * 0.001f;
							hitTowardsLight.direction = LightUtils::GetDirectionToLight(lights[index], hitTowardsLight.origin);
							hitTowardsLight.max = hitTowardsLight.direction.Magnitude();
							hitTowardsLight.direction.Normalize();


						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, hitTowardsLight.direction, -worldSpaceDirection); 
						break;
					}
					case LightingMode::Combined:
					{	
						float observedArea = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(lights[index], closestHit.origin).Normalized());

						if (observedArea < 0)
						{
							continue;
						}

						Ray hitTowardsLight{};
						hitTowardsLight.origin = closestHit.origin + closestHit.normal * 0.001f;
						hitTowardsLight.direction = LightUtils::GetDirectionToLight(lights[index], hitTowardsLight.origin);
						hitTowardsLight.max = hitTowardsLight.direction.Magnitude();
						hitTowardsLight.direction.Normalize();

						finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin) * 
									  materials[closestHit.materialIndex]->Shade(closestHit, hitTowardsLight.direction, -worldSpaceDirection) *
									  observedArea;
						break;
					}

				}

				
			}


		}
	}
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));


}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}


