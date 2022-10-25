#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{

			right = Vector3::Cross(Vector3::UnitY, forward);

			up = Vector3::Cross(forward, right);

			Matrix output
			{
				right,
				up,
				forward,
				origin
			};

			return output;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			
			float moveSpeed{ 10.f * deltaTime };
			float rotSpeed{ (10.f * TO_RADIANS) * deltaTime };

			origin += pKeyboardState[SDL_SCANCODE_W] * forward * moveSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_S] * forward * moveSpeed;

			origin -= pKeyboardState[SDL_SCANCODE_A] * right * moveSpeed;
			origin += pKeyboardState[SDL_SCANCODE_D] * right * moveSpeed;

			origin -= pKeyboardState[SDL_SCANCODE_Q] * up * moveSpeed;
			origin += pKeyboardState[SDL_SCANCODE_E] * up * moveSpeed;

			bool lmb = mouseState == SDL_BUTTON_LMASK;
			bool rmb = mouseState == SDL_BUTTON_RMASK;

			bool lrmb = mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK);

			//std::cout << mouseState << '\n';

			//origin -= lmb * forward * moveSpeed * float(mouseY);
			origin -= lrmb * up * (moveSpeed / 3) * float(mouseY);

			totalPitch -= lmb * rotSpeed * mouseY;
			totalPitch -= rmb * rotSpeed * mouseY;
			totalYaw += lmb * rotSpeed * mouseX;
			totalYaw += rmb * rotSpeed * mouseX;
			
			Matrix totalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };

			forward = totalRotation.TransformVector(Vector3::UnitZ);

			forward.Normalize();
		}
	};
}
