// Author: Andrea Luzzati
#ifndef SR_TEXTURE_H
#define SR_TEXTURE_H
#include <vector>                      // for vector
#define GLM_FORCE_SWIZZLE              // to allow glm vectors to access components with swizzle
#include <glm/glm.hpp>                 // for vec4
#include <iostream>                    // for debugging (cout)
#define STB_IMAGE_IMPLEMENTATION       // to make stb_image.h work
#include "stb_image.h"                 // for stbi_load
#include <glm/gtx/compatibility.hpp>   // for lerp
#define STB_IMAGE_WRITE_IMPLEMENTATION // to make stb_image_write,h work
#include "stb_image_write.h"           // for stbi_write_bmp/png

using namespace glm;

// By default, to simplify things, make all texture be four channels 32bit float so textures can be used
// for basically all applications without the need for care about convoluted texture channel and size specifications
// (not that it wouldn't be a fine addition to this software)
class SrTexture {
public:
	typedef enum CubemapFaceIndex {
		FRONT = 0,
		BACK = 1,
		RIGHT = 2,
		LEFT = 3,
		TOP = 4,
		BOTTOM = 5
	};
private:
	struct textureData {
		float* data;
		int width;
		int height;
	};
	std::vector<textureData> mipmaps;
	std::vector<textureData*> cubemapMipmaps;
	vec4 sampleMipmap(vec2 uv, const bool repeat = false, const bool bilinear = false, const int mipmapLevel = 0, textureData* td = NULL);
	vec4 sampleCubemapMipmap(vec2 uv, CubemapFaceIndex cfi, const bool bilinear, const int mipmapLevel);
public:
	float trilinearCoefficient;
	// TODO: This function was used in a previous version of the software and I should get rid of it - the toImage method
	// would be faster directly using the available buffers.
	unsigned char* generateRawBuffer(int channels, const int mipmapLevel=0);
	SrTexture();
	void disposeData();
	// Loads the texture data with a rectangle filled by a uniform color with the given size.
	void textureFromColor(const int width, const int height, const vec4 color);
	// Loads the texture data from a picture. Specify wether to apply basic gamma correction (this is usually done
	// to the basecolor textures as artists usually work in adobe srgb color space, and not to specialized textures
	// such as displacement maps, normal maps, metallic/roughness/occlusion maps etc...).
	void textureFromImage(const char* fname, const bool gammaCorrect = true);
	// Loads the texture data from a raw buffer file. It is assumed to be just an array of the raw float color 
	// channel values left to right, top to bottom (rgbargbargba...)
	void textureFromBuffer(const char* fname, const int width, const int height, const int channels = 4);
	// Loads the texture data from a cubemap. The specified file is assumed to be an undistorted cubemap face. Use the 
	// arguments to specify the face index and mip level.
	void cubemapFromBuffer(const char* fname, const int width, const int height, const int face, const int mipmapLevel = 0);
	// Returns the texture width when it is used as a texture (as opposed to cubemap).
	int getTextureWidth();
	// Returns the texture height when it is used as a texture (as opposed to cubemap).
	int getTextureHeight();
	~SrTexture();
	// Saves the texture data to an image and returns true if success
	bool toImage(const char* fname, const int mipmapLevel = 0);
	// Get the pixel value at x,y when used as a texture (as opposed to cubemap). For rendering purposes, use instead sample.
	vec4 read(const size_t x, const size_t y);
	// Set the pixel value at x,y when used as a texture (as opposed to cubemap).
	void write(const size_t x, const size_t y, vec4 value);
	/* Sample a color 
		- uv is where in the texture to sample
		- repeat will wrap uvs larger than 1 inside the texture again
		- bilinar enables bilinear filtering as opposed to nearest neighbor sampling
		- trilinear enables trilinear filtering (filtering between mipmaps)
		This function is to be used insetad of read to sample for rendering purposes.
	*/
	vec4 sample(vec2 uv, const bool repeat = false, const bool bilinear = true, const bool trilinear = true);
	vec4 sampleCubemap(vec3 eyeView, const bool bilinear = true, const bool trilinear = true, float trilinearCoefficient = 0.0f);
	// Clear the texture with a given color (no cubemap)
	void clear(vec4 color);
	// Draw a line on the texture (no cubemap) for debugging purposes
	void textureDrawLine(ivec2 a, ivec2 b, vec4 color=vec4(1,1,1,1));
	// Generates the mipmap chain all the way to the 1x1 pixel size (if possible)
	void generateMipmaps();
	// Get generated mipmap chain size
	int getMipmapCount();
	/* Computes the trilinear coefficient to use given the UV area covered by the pixel being drawn. 
	   For example, a coefficient of 3.7 will trilinearly interpolate between mipmap levels 3 and 4 with
	   an interpolation coefficient of 0.7. The trilinear coefficient is used to select the proper mipmap
	   levels so that ideally every pixel drawn correspond to no more than one texel (less than one texel is
	   accepted in that it means that the first mipmap is not enough high resolution) and avoid aliasing of
	   the high frequency components. */
	void calculateTrilinearCoefficient(float pixelUVAreaCoverage);

};


// TEXTURE IMPLENTATION
void SrTexture::textureFromColor(const int width, const int height, const vec4 color)
{
	disposeData();
	textureData td;
	td.width = width;
	td.height = height;
	td.data = new float[width * height * 4];
	mipmaps.push_back(td);
	clear(color);
}
void SrTexture::textureFromImage(const char* fname, const bool correctGamma) {
	disposeData();
	int width, height, n;
	unsigned char* rawData = stbi_load(fname, &width, &height, &n, 0);
	if (rawData == NULL) return;
	float* data = new float[width * height * 4];
	for (unsigned int x = 0; x < width; x++)
		for (unsigned int y = 0; y < height; y++) {
			if (correctGamma)
				for (unsigned int i = 0; i < (n < 4 ? n : 4); i++)
					data[y * width * 4 + x * 4 + i] = pow((float)rawData[x * n + i + y * width * n] / 255.0f, 2.2f);
			else
				for (unsigned int i = 0; i < (n < 4 ? n : 4); i++)
					data[y * width * 4 + x * 4 + i] = (float)rawData[x * n + i + y * width * n] / 255.0f;
			for (unsigned int i = n; i < 4; i++)
				data[y * width * 4 + x * 4 + i] = 0.0f;
		}
	textureData td;
	td.width = width;
	td.height = height;
	td.data = data;
	mipmaps.push_back(td);
	stbi_image_free(rawData);
}
void SrTexture::textureFromBuffer(const char* fname, const int width, const int height, const int channels) {
	disposeData();
	
	textureData td;
	td.width = width;
	td.height = height;
	td.data = new float[width * height * 4];
	mipmaps.push_back(td);

	FILE* pFile;
	fopen_s(&pFile, fname, "rb");
	for(int x = 0; x < width; x++)
		for (int y = 0; y < height; y++) {
			fread(&td.data[x * 4 + y * width * 4], 4, channels, pFile);
			if (channels < 4)
				for (int j = channels; j < 4; j++)
					td.data[x * 4 + j + y * width * 4] = 0.0f;
		}
	fclose(pFile);
}
void SrTexture::cubemapFromBuffer(const char* fname, const int width, const int height, const int face, const int mipmapLevel)
{
	while (mipmapLevel >= cubemapMipmaps.size()) 
		cubemapMipmaps.push_back( new textureData[6] );
	textureData* cubemap = cubemapMipmaps[mipmapLevel];
	cubemap[face].width = width;
	cubemap[face].height = height;
	cubemap[face].data = new float[width * height * 4];

	FILE* pFile;
	fopen_s(&pFile, fname, "rb");
	int channels = 3;
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++) {
			fread(&cubemap[face].data[x * 4 + y * width * 4], 4, channels, pFile);
			if (channels < 4)
				for (int j = channels; j < 4; j++)
					cubemap[face].data[x * 4 + j + y * width * 4] = 0.0f;
		}
	fclose(pFile);
}
SrTexture::SrTexture() {
	trilinearCoefficient = 0.0f;
}
SrTexture::~SrTexture() {
	disposeData();
}
void SrTexture::disposeData() {
	if (mipmaps.size() > 0) {
		for (std::vector<textureData>::iterator it = mipmaps.begin(); it != mipmaps.end(); it++)
			delete (*it).data;
		mipmaps.clear();
	}
	if (cubemapMipmaps.size() > 0) {
		for (std::vector<textureData*>::iterator it = cubemapMipmaps.begin(); it != cubemapMipmaps.end(); it++) {
			delete (*it)[0].data;
			delete (*it)[1].data;
			delete (*it)[2].data;
			delete (*it)[3].data;
			delete (*it)[4].data;
			delete (*it)[5].data;
			delete (*it);
		}
		cubemapMipmaps.clear();
	}
}
int SrTexture::getTextureWidth() {
	return mipmaps[0].width;
}
int SrTexture::getTextureHeight() {
	return mipmaps[0].height;
}
vec4 SrTexture::sampleCubemapMipmap(vec2 uv, SrTexture::CubemapFaceIndex cfi, const bool bilinear, const int mipmapLevel) {
	textureData td = cubemapMipmaps[mipmapLevel][cfi];
	return sampleMipmap(uv, false, bilinear, 0, &td);
}
vec4 SrTexture::sampleMipmap(vec2 uv, const bool repeat, const bool bilinear, const int mipmapLevel, textureData* tdd) {
	textureData td;
	if (tdd != NULL) td = *tdd;
	else td = mipmaps[mipmapLevel];
	int width = td.width;
	int height = td.height;
	vec2 size = vec2(width, height);
	ivec2 iSize = ivec2(width - 1, height - 1);
	if (repeat) uv = mod(uv, vec2(1.0f));
	uv = clamp(uv, vec2(0.0f), vec2(1.0f));
	ivec2 pos = round(size * uv);
	if (pos.x >= width) pos.x = width - 1;
	if (pos.y >= height) pos.y = height - 1;

	// Nearest neighbor sampling
	if (!bilinear) return vec4(
		td.data[pos.x * 4 + 0 + pos.y * width * 4],
		td.data[pos.x * 4 + 1 + pos.y * width * 4],
		td.data[pos.x * 4 + 2 + pos.y * width * 4],
		td.data[pos.x * 4 + 3 + pos.y * width * 4]
	);

	// Bilinear filtering sampling
	ivec2 q11, q12, q22, q21;
	vec2 p = size * uv;
	q11 = clamp(ivec2(floor(p)), ivec2(0, 0), iSize); // top left
	q22 = clamp(q11 + ivec2(1, 1), ivec2(0, 0), iSize); // bottom right
	q12 = clamp(q11 + ivec2(0, 1), ivec2(0, 0), iSize); // bottom left
	q21 = clamp(q11 + ivec2(1, 0), ivec2(0, 0), iSize); // top right
	vec4 R2, R1;
	vec4 d11(
		td.data[q11.x * 4 + width * q11.y * 4 + 0],
		td.data[q11.x * 4 + width * q11.y * 4 + 1],
		td.data[q11.x * 4 + width * q11.y * 4 + 2],
		td.data[q11.x * 4 + width * q11.y * 4 + 3]
	);
	vec4 d21(
		td.data[q21.x * 4 + width * q21.y * 4 + 0],
		td.data[q21.x * 4 + width * q21.y * 4 + 1],
		td.data[q21.x * 4 + width * q21.y * 4 + 2],
		td.data[q21.x * 4 + width * q21.y * 4 + 3]
	);
	vec4 d12(
		td.data[q12.x * 4 + width * q12.y * 4 + 0],
		td.data[q12.x * 4 + width * q12.y * 4 + 1],
		td.data[q12.x * 4 + width * q12.y * 4 + 2],
		td.data[q12.x * 4 + width * q12.y * 4 + 3]
	);
	vec4 d22(
		td.data[q22.x * 4 + width * q22.y * 4 + 0],
		td.data[q22.x * 4 + width * q22.y * 4 + 1],
		td.data[q22.x * 4 + width * q22.y * 4 + 2],
		td.data[q22.x * 4 + width * q22.y * 4 + 3]
	);
	R1 = lerp(d11, d21, fract(p.x)); // top sample
	R2 = lerp(d12, d22, fract(p.x)); // bottom sample
	return lerp(R1, R2, fract(p.y));
}
vec4 SrTexture::sample(vec2 uv, const bool repeat, const bool bilinear , const bool trilinear) {
	int mipmapLow = floor(max(trilinearCoefficient,0.0f));
	if (mipmapLow >= mipmaps.size())
		mipmapLow = mipmaps.size() - 1;
	if (!trilinear) return sampleMipmap(uv, repeat, bilinear, mipmapLow);
	int mipmapHigh = mipmapLow + 1;
	if (mipmapHigh >= mipmaps.size())
		mipmapHigh = mipmaps.size() - 1;
	return lerp(
		sampleMipmap(uv, repeat, bilinear, mipmapLow),
		sampleMipmap(uv, repeat, bilinear, mipmapHigh),
		fract(trilinearCoefficient)
	);
}
void SrTexture::clear(vec4 color) {
	float fColor[4] = { color.r,color.g,color.b,color.a };
	float* data = mipmaps[0].data;
	for (size_t i = 0; i < mipmaps[0].width * mipmaps[0].height * 4; i += 4)
		memcpy(&data[i], fColor, 4*4);
}
bool SrTexture::toImage(const char* filename, const int mipmapLevel) {
	unsigned char* raw;
	bool success;
	size_t len = strlen(filename);
	if (filename[len - 4] == '.') { // detect extension
		// BMP
		if ((filename[len - 3] == 'b' || filename[len - 3] == 'B') &&
			(filename[len - 2] == 'm' || filename[len - 2] == 'M') &&
			(filename[len - 1] == 'p' || filename[len - 1] == 'P')) {
			raw = generateRawBuffer(3,mipmapLevel);
			success = stbi_write_bmp(filename, mipmaps[mipmapLevel].width, mipmaps[mipmapLevel].height, 3, raw) == 0;
		}
		// Default to PNG
		else {
			raw = generateRawBuffer(4,mipmapLevel);
			success = stbi_write_png(filename, mipmaps[mipmapLevel].width, mipmaps[mipmapLevel].height, 4, raw, 0) == 0;
		}
	}
	else { // Default to PNG
		raw = generateRawBuffer(4,mipmapLevel);
		success = stbi_write_png(filename, mipmaps[mipmapLevel].width, mipmaps[mipmapLevel].height, 4, raw, 4) == 0;
	}
	delete raw;
	return success;
}
unsigned char* SrTexture::generateRawBuffer(int channels, const int mipmapLevel) {
	channels = clamp(channels, 0, 4);
	float* data = mipmaps[mipmapLevel].data;
	int width = mipmaps[mipmapLevel].width, height = mipmaps[mipmapLevel].height;
	unsigned char* buff = new unsigned char[width * height * channels];
	for (unsigned int x = 0; x < width; x++)
		for (unsigned int y = 0; y < height; y++)
			for (unsigned int i = 0; i < channels; i++)
				buff[x * channels + i + y * width * channels] = (unsigned char)(clamp(data[x*4+y*width*4+i], 0.0f, 1.0f) * 255.0f);
	return buff;
}
void SrTexture::textureDrawLine(ivec2 a, ivec2 b, vec4 color) {
	// Simplest draw line implementation for debugging purposes
	vec2 r = vec2(b) - vec2(a);
	float distance = length(r);
	r = normalize(r);
	vec2 p = a;
	int x, y;
	while (length(p - vec2(a)) <= distance) {
		x = round(p.x); y = round(p.y);
		if (x >= 0 && x < mipmaps[0].width && y >= 0 && y < mipmaps[0].height)
			write(x, y, color);
		p += r;
	}
}
vec4 SrTexture::read(const size_t x, const size_t y) {
	float* data = mipmaps[0].data;
	int width = mipmaps[0].width, height = mipmaps[0].height; 
	return vec4(
		data[x * 4 + y * width * 4 + 0],
		data[x * 4 + y * width * 4 + 1],
		data[x * 4 + y * width * 4 + 2],
		data[x * 4 + y * width * 4 + 3]
	);
}
void SrTexture::write(const size_t x, const size_t y, vec4 value) {
	float* data = mipmaps[0].data;
	int width = mipmaps[0].width, height = mipmaps[0].height;
	data[x * 4 + y * width * 4 + 0] = value.x;
	data[x * 4 + y * width * 4 + 1] = value.y;
	data[x * 4 + y * width * 4 + 2] = value.z;
	data[x * 4 + y * width * 4 + 3] = value.w;
}
void SrTexture::generateMipmaps() {
	if (size(mipmaps) != 1) return;
	int i = 0;
	int newWidth, newHeight;
	while (mipmaps[i].width >= 2 && mipmaps[i].height >= 2) {
		newWidth = mipmaps[i].width >> 1;
		newHeight = mipmaps[i].height >> 1;
		textureData mipmap;
		mipmap.width = newWidth;
		mipmap.height = newHeight;
		mipmap.data = new float[newWidth * newHeight * 4];
		for (int x = 0; x < newWidth; x++)
			for (int y = 0; y < newHeight; y++) {
				for (int k = 0; k < 4; k++) {
					mipmap.data[x * 4 + y * newWidth * 4 + k] = (
						mipmaps[i].data[x * 2 * 4 + y * 2 * mipmaps[i].width * 4 + k] +
						mipmaps[i].data[(x * 2 + 1) * 4 + y *2* mipmaps[i].width * 4 + k] +
						mipmaps[i].data[x * 2 * 4 + (y * 2 + 1) * mipmaps[i].width * 4 + k] +
						mipmaps[i].data[(x * 2 + 1) * 4 + (y * 2 + 1) * mipmaps[i].width * 4 + k]) * 0.25f;
				}
			}
		mipmaps.push_back(mipmap);
		i += 1;
	}
	return;
}
int SrTexture::getMipmapCount() {
	return mipmaps.size();
}
void SrTexture::calculateTrilinearCoefficient(float puvac) {
	// Cycle through the puvac (pixelUVAreaCoverage) of all mipmap levels
	// if puvac <= puvac(0) sets the coefficient to 0.0f
	// if puvac >= puvac(N) sets the coefficient to (float)N
	// Select two levels l0, l1 such that puvac(l0) <= puvac <= puvacl(l1)
	int mms = mipmaps.size();
	if (mms == 1) { trilinearCoefficient = 0; return; }
	float mPuvac = 0.0f, mPuvacPrev;
	for (int l = 0; l < mms; l++) {
		mPuvacPrev = mPuvac;
		mPuvac = 1.0f / (float)(mipmaps[l].width * mipmaps[l].height);
		if (puvac <= mPuvac) {
			if (l == 0) { trilinearCoefficient = 0; return; }
			trilinearCoefficient = (float)(l-1) + (puvac-mPuvacPrev)/(mPuvac-mPuvacPrev);
			return;
		}
	}
	trilinearCoefficient = mms - 1;
	return;

}

vec4 SrTexture::sampleCubemap(vec3 eyePos, const bool bilinear, const bool trilinear, float trilinearCoefficient) {
	#pragma region Compute CFI and UV
	CubemapFaceIndex cfi;
	vec3 spacePos = eyePos;
	vec2 uv;
	if (spacePos.x < 0.0) { spacePos.x = -spacePos.x; }
	if (spacePos.y < 0.0) { spacePos.y = -spacePos.y; }
	if (spacePos.z < 0.0) { spacePos.z = -spacePos.z; }
	if (spacePos.x >= spacePos.y && spacePos.x >= spacePos.z) { // Left and right
		if (eyePos.x > 0.0) {
			cfi = CubemapFaceIndex::LEFT;
			uv = vec2(0.5f - eyePos.y / eyePos.x, 0.5f - eyePos.z / eyePos.x);
		}
		else {
			cfi = CubemapFaceIndex::RIGHT;
			uv = vec2(0.5f - eyePos.y / eyePos.x, 0.5f + eyePos.z / eyePos.x);
		}
	}
	else { // Back and front
		if (spacePos.y > spacePos.x && spacePos.y >= spacePos.z) {
			if (eyePos.y > 0.0) {
				cfi = CubemapFaceIndex::BACK;
				uv = vec2(0.5f + eyePos.x / eyePos.y , 0.5f-eyePos.z / eyePos.y);
			}
			else {
				cfi = CubemapFaceIndex::FRONT;
				uv = vec2(0.5f + eyePos.x / eyePos.y, 0.5f + eyePos.z / eyePos.y);
			}
		}
		else { // Top and bottom
			if (eyePos.z > 0.0) {
				cfi = CubemapFaceIndex::TOP;
				uv = vec2(0.5f - eyePos.x / eyePos.z , 0.5f - eyePos.y / eyePos.z );
			}
			else {
				cfi = CubemapFaceIndex::BOTTOM;
				uv = vec2(0.5f + eyePos.x / eyePos.z ,0.5f -eyePos.y / eyePos.z);
			}
		}
	}
	uv = (uv + vec2(0.5f)) * 0.5f;
	// Not sure why i need this modification
	if (cfi == BOTTOM) cfi = TOP;
	else if (cfi == TOP) cfi = BOTTOM;
	if (cfi != BOTTOM && cfi != TOP)
		uv = vec2(1.0f - uv.y, uv.x);
	if (cfi == TOP)
		uv = vec2(1.0f - uv.x, 1.0f - uv.y);
	#pragma endregion

	int mipmapLow = floor(max(trilinearCoefficient, 0.0f));
	if (mipmapLow >= cubemapMipmaps.size())
		mipmapLow = cubemapMipmaps.size() - 1;
	if (!trilinear) return sampleCubemapMipmap(uv, cfi, bilinear, mipmapLow);
	int mipmapHigh = mipmapLow + 1;
	if (mipmapHigh >= cubemapMipmaps.size())
		mipmapHigh = cubemapMipmaps.size() - 1;
	return lerp(
		sampleCubemapMipmap(uv, cfi, bilinear, mipmapLow),
		sampleCubemapMipmap(uv, cfi, bilinear, mipmapHigh),
		fract(trilinearCoefficient)
	);
}
#endif