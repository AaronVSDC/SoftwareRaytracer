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
		float fovAngle{45.f};
		float FOV = tanf(fovAngle / 2.f);


		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward); 
			up = Vector3::Cross(forward, right);
			right.Normalize(); 
			up.Normalize(); 

			return
			{
				{right.x, right.y, right.z, 0},
				{up.x, up.y, up.z, 0},
				{forward.x, forward.y, forward.z, 0 },
				{origin.x, origin.y, origin.z, 1}
			}; 
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const float SPEED{ 10.f }; 
			float velocity = deltaTime * SPEED; 

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += velocity * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= velocity * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= velocity * right;
	
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += velocity * right;
		
			}


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float mouseSensitivity{ 0.01f }; 

			switch (mouseState)
			{	
			
			case SDL_BUTTON_X2:
			{
				origin.y += mouseY * mouseSensitivity;
				break;
			}
			case SDL_BUTTON_RMASK: 
			{				
				totalPitch += mouseY * mouseSensitivity;
				totalYaw += mouseX * mouseSensitivity;
				break;
			}
			case SDL_BUTTON_LMASK:
			{
				origin.z -= mouseY * forward.z;
				origin.x -= mouseY * forward.x;
				totalYaw += mouseX * mouseSensitivity;
				break;
			}

			}
	
			
			Matrix rotationMatrix = Matrix::CreateRotation(totalPitch, totalYaw, 0.f); 

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);

			//todo: W2
			cameraToWorld = CalculateCameraToWorld(); 
		}
	};
}
