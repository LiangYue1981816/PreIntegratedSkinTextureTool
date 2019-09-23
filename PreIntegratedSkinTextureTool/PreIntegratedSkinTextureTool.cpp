// 参考 https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch14.html
//

#include "pch.h"

#include "gli/gli.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

#define PI 3.1415926535897932384626433832795f

float clamp(float value, float minValue, float maxValue)
{
	value = max(value, minValue);
	value = min(value, maxValue);
	return value;
}

glm::vec3 Linear2Gamma(glm::vec3 linear)
{
	return glm::pow(linear, glm::vec3(1.0f / 2.2f));
}

float Gaussian(float v, float r)
{
	return 1.0f / sqrt(2.0f * PI * v) * exp(-(r * r) / (2.0f * v));
}

glm::vec3 Scatter(float r)
{
	// Values from GPU Gems 3 "Advanced Skin Rendering"
	// Originally taken from real life samples
	return
		Gaussian(0.0064f * 1.414f, r) * glm::vec3(0.233f, 0.455f, 0.649f) +
		Gaussian(0.0484f * 1.414f, r) * glm::vec3(0.100f, 0.336f, 0.344f) +
		Gaussian(0.1870f * 1.414f, r) * glm::vec3(0.118f, 0.198f, 0.000f) +
		Gaussian(0.5670f * 1.414f, r) * glm::vec3(0.113f, 0.007f, 0.007f) +
		Gaussian(1.9900f * 1.414f, r) * glm::vec3(0.358f, 0.004f, 0.000f) +
		Gaussian(7.4100f * 1.414f, r) * glm::vec3(0.078f, 0.000f, 0.000f);
}

glm::vec3 IntegrateDiffuseScatteringOnRing(float cosTheta, float skinRadius)
{
	float a = -PI / 2.0f;
	float inc = 0.05f;
	float theta = acos(cosTheta);

	glm::vec3 totalLight(0.0f, 0.0f, 0.0f);
	glm::vec3 totalWeight(0.0f, 0.0f, 0.0f);

	while (a <= PI /2.0f) {
		float angle = theta + a;
		float light = clamp(cos(angle), 0.0f, 1.0f);
		float distance = fabs(2.0f * skinRadius * sin(a * 0.5f));
		glm::vec3 weight = Scatter(distance);

		totalLight += light * weight;
		totalWeight += weight;
		a += inc;
	}

	return totalLight / totalWeight;
}

float PHBeckmann(float cosTheta, float m)
{
	float theta = acos(cosTheta);
	float ta = tan(theta);
	return 1.0f / (m * m * pow(cosTheta, 4.0f)) * exp(-(ta * ta) / (m * m));
}

template <typename gen_type>
void SetTexturePixelColor(gli::texture2d &texture, int x, int y, const gen_type &color)
{
	texture.store(gli::extent2d(x, y), 0, color);
}

int main()
{
	const int resolution = 128;

	gli::texture2d texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(resolution, resolution));
	{
		for (int y = 0; y < resolution; y++) {
			for (int x = 0; x < resolution; x++) {
				float ndl = glm::mix(-1.0f, 1.0f, 1.0f * x / resolution);
				float invr = 2.0f / ((y + 1.0f) / (float)resolution); //1.0f * y / resolution;

				float spec = 0.5f * pow(PHBeckmann(1.0f * x / resolution, 1.0f - 1.0f * y / (float)resolution), 0.1f);
				glm::vec3 diffuse = IntegrateDiffuseScatteringOnRing(ndl, invr);

				int r = (int)(diffuse.r * 255.0f + 0.5f);
				int g = (int)(diffuse.g * 255.0f + 0.5f);
				int b = (int)(diffuse.b * 255.0f + 0.5f);
				int a = (int)(spec * 255.0f + 0.5f);

				if (r < 0) r = 0; if (r > 255) r = 255;
				if (g < 0) g = 0; if (g > 255) g = 255;
				if (b < 0) b = 0; if (b > 255) b = 255;
				if (a < 0) a = 0; if (a > 255) a = 255;

				SetTexturePixelColor(texture, x, y, glm::u8vec4(r, g, b, a));
			}
		}
	}
	texture = gli::flip(texture);
	gli::save_dds(texture, "PreIntegratedSkinLUT.dds");

	return 0;
}
