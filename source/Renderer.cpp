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

using namespace dae;

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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float fov = tanf((camera.fovAngle / 2) * TO_RADIANS);

	Matrix camMatrix = camera.CalculateCameraToWorld();
	
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 rayDirection{ };

			rayDirection.x = (((2 * (px + 0.5f)) / m_Width) - 1) * m_AspectRatio * fov;

			rayDirection.y = (1 - ((2 * (py + 0.5f)) / m_Height)) * fov;

			rayDirection.z = 1.f;

			rayDirection.Normalize();

			rayDirection = camMatrix.TransformVector(rayDirection);

			Ray viewRay{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{

				closestHit.origin = closestHit.origin + (closestHit.normal * 0.0001f);

					for (int i{}; i < lights.size(); ++i)
					{

						Vector3 lightDirection = LightUtils::GetDirectionToLight(lights[i], closestHit.origin);

						float magnitude{ lightDirection.Normalize()};

						if (m_ShadowsEnabled)
						{
							Ray shadowRay{ closestHit.origin, lightDirection };

							shadowRay.max = magnitude;

							if (pScene->DoesHit(shadowRay))
							{
								finalColor = finalColor * 0.95f;
								continue;
							}
						}

						float observedArea{ Vector3::Dot(closestHit.normal, lightDirection) };

						switch (m_CurrentLightingMode)
						{
						case dae::Renderer::LightingMode::ObservedArea:
						{

							if (observedArea >= 0.f) finalColor += ColorRGB{ 1.f,1.f,1.f } * observedArea;

						}

						break;
						case dae::Renderer::LightingMode::Radiance:
						{
							finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin);

							break;
						}
						case dae::Renderer::LightingMode::BRDF:

							finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -viewRay.direction);

							break;
						case dae::Renderer::LightingMode::Combined:
						{
							ColorRGB areaColor{};

							if (observedArea >= 0.f)
							{
								areaColor = ColorRGB{ 1.f,1.f,1.f } * observedArea;
							}

							finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * areaColor * materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -viewRay.direction);

							break;
						}
						
						default:
							break;
						}
					}
				
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % 4);
}
