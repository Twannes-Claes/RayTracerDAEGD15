#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>
#include <algorithm>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, const float _fovAngle):
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

		const float minPitch{ -89.99f * TO_RADIANS };
		const float maxPitch{ 89.99f * TO_RADIANS };

		const float speed{ 10.f };

		const int sprintSpeedMultiplier{ 3 };

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();

			up = Vector3::Cross(forward, right);

			cameraToWorld = Matrix
			{
				right,
				up,
				forward,
				origin
			};

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			
			float moveSpeed{ speed * deltaTime };
			float rotSpeed{ (speed * TO_RADIANS) * deltaTime };

			moveSpeed = (pKeyboardState[SDL_SCANCODE_LSHIFT] * sprintSpeedMultiplier * moveSpeed) + moveSpeed;

			origin += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])  * forward * moveSpeed;
			origin -= (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN]) * forward * moveSpeed;

			origin -= (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT]) * right * moveSpeed;
			origin += (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT]) * right * moveSpeed;

			origin -= pKeyboardState[SDL_SCANCODE_Q] * up * moveSpeed;
			origin += pKeyboardState[SDL_SCANCODE_E] * up * moveSpeed;

			bool lmb = mouseState == SDL_BUTTON_LMASK;
			bool rmb = mouseState == SDL_BUTTON_RMASK;

			bool lrmb = mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK);

			origin -= lmb * forward * moveSpeed * float(mouseY);
			origin -= lrmb * up * (moveSpeed / 3) * float(mouseY);

			totalPitch -= rmb * rotSpeed * mouseY;
			totalPitch = std::clamp(totalPitch, minPitch, maxPitch);

			totalYaw += lmb * rotSpeed * mouseX;
			totalYaw += rmb * rotSpeed * mouseX;

			forward = (Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw)).TransformVector(Vector3::UnitZ);

		}
	};
}
