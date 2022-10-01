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

	float fov = tanf(camera.fovAngle / 2);

	Matrix camMatrix = camera.CalculateCameraToWorld();

	
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 rayDirection{ };

			rayDirection.x = (((2 * (px + 0.5f)) / m_Width) - 1) * m_AspectRatio * fov;

			rayDirection.y = (1 - ((2 * (py + 0.5f)) / m_Height)) * fov;

			rayDirection.z = 1.f;

			rayDirection = camMatrix.TransformVector(rayDirection);

			Ray viewRay{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				closestHit.origin = closestHit.origin + (closestHit.normal * 0.01f);

				for (int i{}; i < lights.size(); ++i)
				{

					Vector3 lightRay = LightUtils::GetDirectionToLight(lights[i], closestHit.origin);

					float magnitude = lightRay.Normalize();

					Ray shadowRay{ closestHit.origin, lightRay };

					shadowRay.max = magnitude;

					if (pScene->DoesHit(shadowRay))
					{
						finalColor = finalColor / 2;
					}
				}
				/*const float scaled_t = closestHit.t / 500.f;
				finalColor = { scaled_t, scaled_t, scaled_t };*/
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}


	//test if dot and cross product implementation works

		/*float dotResult{};

		dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitX);
		dotResult = Vector3::Dot(Vector3::UnitX, -Vector3::UnitX);
		dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitY);


		Vector3 crossResult{};
		crossResult = Vector3::Cross(Vector3::UnitZ, Vector3::UnitX);
		crossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ);*/

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
