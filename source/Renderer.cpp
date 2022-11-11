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
#include <iostream>
#include <thread>
#include <future>
#include <ppl.h>

using namespace dae;

//#define ASYNC
#define PARAREL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = m_Width / static_cast<float>(m_Height);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const auto& materials = pScene->GetMaterials();
	const auto& lights = pScene->GetLights();

	float fov = tanf((camera.fovAngle / 2) * TO_RADIANS);

	camera.CalculateCameraToWorld();

	const uint32_t numPixel{ static_cast<uint32_t>(m_Width * m_Height) };
	
#if defined(ASYNC)

	const uint32_t numCores{ std::thread::hardware_concurrency() };

	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelsPerTask{ numPixel / numCores };
	uint32_t numAssignedPixels{ numPixel % numCores };
	uint32_t currPixelIndex{0};

	for (uint32_t coreid = 0; coreid < numCores; ++coreid)
	{
		uint32_t taskSize{ numPixelsPerTask };

		if (numAssignedPixels > 0)
		{
			++taskSize;
			--numAssignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this]
				{
					const uint32_t pixelIndexEnd{ currPixelIndex + taskSize };

					for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
					{
						RenderPixel(pScene, pixelIndex, fov, m_AspectRatio, camera, lights, materials);
					}
				})
		);

		currPixelIndex += taskSize;
	}

	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}


#elif defined(PARAREL_FOR)

	concurrency::parallel_for(0u, numPixel, [=, this](int i) {
		RenderPixel(pScene, i, fov, m_AspectRatio, camera, lights, materials);
		});

#else
	for (uint32_t i{}; i < numPixel; ++i)
	{
		RenderPixel(pScene, i, fov, m_AspectRatio, camera, lights, materials);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* scenePtr, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{

	const int px{ static_cast<int>(pixelIndex) % m_Width };
	const int py{ static_cast<int>(pixelIndex) / m_Width };

	const float halfPixel{ 0.5f };

	float cx = (((2 * ( px + halfPixel)) / m_Width) - 1) * m_AspectRatio * fov;
	float cy = (1 - ((2 * ( py + halfPixel)) / m_Height)) * fov;

	Vector3 rayDirection{ camera.cameraToWorld.TransformVector({cx, cy, 1.f }).Normalized()};

	const Ray viewRay{ camera.origin, rayDirection };

	ColorRGB finalColor{};

	HitRecord closestHit{};

	scenePtr->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		const float epsilon{ 0.001f };

		closestHit.origin += (closestHit.normal * epsilon);

		for (const Light& light : lights)
		{

			Vector3 lightDirection = LightUtils::GetDirectionToLight(light, closestHit.origin);

			const float magnitude{ lightDirection.Normalize() };

			if (m_ShadowsEnabled)
			{
				const Ray shadowRay{ closestHit.origin, lightDirection, epsilon, magnitude };

				if (scenePtr->DoesHit(shadowRay))
				{
					continue;
				}
			}

			const float observedArea{ Vector3::Dot(closestHit.normal, lightDirection) };

			if (observedArea < epsilon) continue;

			switch (m_CurrentLightingMode)
			{
				case dae::Renderer::LightingMode::ObservedArea:
				{

					finalColor += ColorRGB{ 1.f,1.f,1.f } * observedArea;

				}

				break;
				case dae::Renderer::LightingMode::Radiance:
				{
					finalColor += LightUtils::GetRadiance(light, closestHit.origin);

					break;
				}
				case dae::Renderer::LightingMode::BRDF:

					finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -viewRay.direction);

					break;
				case dae::Renderer::LightingMode::Combined:
				{
					ColorRGB areaColor{ ColorRGB{ 1.f,1.f,1.f } * observedArea };

					finalColor += LightUtils::GetRadiance(light, closestHit.origin) * areaColor * materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -viewRay.direction);

					break;
				}

					default:
					break;
			}

		}

	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + ( py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
