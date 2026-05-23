#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;
#include <iostream>
namespace dae
{
	class Scene;
	class Matrix; 
	class Vector3; 

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const; 
		bool SaveBufferToImage() const;

		void ToggleLightingModes() 
		{
			const int maxLightingValue = static_cast<int>(LightingMode::Combined) + 1;
			int curLightingValue = static_cast<int>(m_CurrentLightingMode);

			m_CurrentLightingMode = static_cast<LightingMode>(++curLightingValue % maxLightingValue);
			switch (m_CurrentLightingMode)
			{
			case Renderer::LightingMode::ObservedArea:
				std::cout << "LightMode: Observed Area\n";
				break;
			case Renderer::LightingMode::Radiance:
				std::cout << "LightMode: Radiance\n";
				break;
			case Renderer::LightingMode::BRDF:
				std::cout << "LightMode: BRDF\n";
				break;
			case Renderer::LightingMode::Combined:
				std::cout << "LightMode: Combined\n";
				break;
			}
		};

		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled;  }

	private:

		enum class LightingMode
		{
			ObservedArea, 
			Radiance, 
			BRDF, 
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined}; 
		bool m_ShadowsEnabled{ true }; 

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		float ascending;
	};
}
