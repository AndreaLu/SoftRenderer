// Author: Andrea Luzzati
#include <iostream>
#include "texture.h"
#include "gpu.h"
#include <glm/ext.hpp>
#include "utils.h"
#include <string>


vec3 eye;
vec3 to;
vec3 up;
vec3 forward;
vec3 right;
float znear, zfar;
float resWidth, resHeight;
float fovx, fovy;
float drawingBackground;
mat4 matWorld, matView, matProjection;
SrVsOutput basicVertexShader(SrGPU* gpu, SrVertex& input);
vec4 PBRFragmentShader(SrGPU* gpu, SrFsInput& input);
int main(int argc, char** argv) {
	resWidth = 1024.0f;
	resHeight = 1024.0f;
	float aspect = resWidth / resHeight;

	// Load all the textures
	SrTexture albedo, normal, mro, radiance, irradiance, brdflut;
	albedo.textureFromImage("cerberus-albedo.png");
	normal.textureFromImage("cerberus-normal.png",false);
	mro.textureFromImage("cerberus-mro.png",false); // metallic-roughness-occlusion
	brdflut.textureFromBuffer("brdf.buff", 512, 512, 2);
	albedo.generateMipmaps();
	normal.generateMipmaps();
	mro.generateMipmaps();


	// Load the radiance and irradiancec environment cubemaps 
	std::string inFname;
	const char* faceMap[6] = { "front","back","right","left","top","bottom" };
	float f = 1.0f;
	for (int mip = 0; mip < 8; mip++) {
		for (int face = 0; face < 6; face++) {
			inFname = "emap\\radiance-";
			inFname.append(std::to_string(mip));
			inFname.append("-");
			inFname.append(faceMap[face]);
			inFname.append(".buff");
			radiance.cubemapFromBuffer(inFname.c_str(), 512 * f, 512 * f, face, mip);
			if( mip == 0 ) { // Load the irradiance, which has no mipmaps
				inFname = "emap\\irradiance-";
				inFname.append(faceMap[face]);
				inFname.append(".buff");
				irradiance.cubemapFromBuffer(inFname.c_str(), 32, 32, face, 0);
			}
		}
		f /= 2.0f;
	}

	// Load the cerberus gun mesh
	SrMesh meshCerberus = loadMeshBuffer("cerberus-mesh.buff");

	// Initialize the software renderer virtual GPU
	SrGPU gpu(resWidth, resHeight);
	matWorld = mat4(1.0f);
	znear = 0.005f;
	zfar = 200.0f;
	fovx = radians(90.0f); fovy = fovx;
	matProjection = perspectiveFov(radians(270.0f), resWidth, resHeight, znear,zfar);
	gpu.samplers.push_back(&albedo);
	gpu.samplers.push_back(&normal);
	gpu.samplers.push_back(&mro);
	gpu.samplers.push_back(&radiance);
	gpu.samplers.push_back(&irradiance);
	gpu.samplers.push_back(&brdflut);
	gpu.vertexShaderProgram = basicVertexShader;
	gpu.fragmentShaderProgram = PBRFragmentShader;


	/* Rendering --
	 * Generate an offline animation by saving the backbuffer to a picture file for each frame.
	 * You can later use ffmpeg to combine the frames into a video with a command such as:
	 * ffmpeg -f image2 -i .\screen%d.bmp videofname.mp4
	 * This is being done untill I'll create a window with GLFW/OpenGL to display the results of the soft renderer.
	 * (this technique can also be used to create an offline animation for complex scenes 
	 * that require too much rendering time to be in real time) */
	std::string screenshotFname;
	float dist;
	float tMax = 200.0f;
	for (float t = 0.0f; t < tMax;  t += 1.0f) {
		if ((int)t != 15) continue;

		// Update the view matrix
		float angle = t * 2.0f * 3.14159265359f / tMax; // Compute angle for uniform velocity rotation
		float dist = 0.5f + 1.2f*cos(angle)*cos(angle) * ((cos(angle) + 1.0f) * 0.25f + 0.5f) * 1.2f; // Make the camera go back and forth
		eye = normalize(vec3(cos(angle), sin(angle), cos(angle)*0.3f)) * dist; // Rotate the camera around the origin 
		to = vec3(0, 0, 0); // Look at the origin of the world, where the model is
		// Compute up vector starting from 0,0,1 and removing the parallel component to the view ray
		up = vec3(0.0f, 0.0f, 1.0f);
		forward = normalize(to - eye);
		up = normalize(up - forward * dot(up, forward));
		right = normalize(cross(forward, up));
		matView = lookAt(eye, to, up);

		// Rendedr the cube
		gpu.clearBuffers();
		drawingBackground = true;
		gpu.drawFillQuad();
		drawingBackground = false;
		gpu.submitMesh(meshCerberus,SrGPU::CullMode::COUNTERCLOCKWISE);

		// Save the screenshot
		screenshotFname = "output-frame-";
		screenshotFname.append(std::to_string((int)t));
		screenshotFname.append(".png");
		gpu.backBuffer->toImage(screenshotFname.c_str());
	}
	return 0;
}

SrVsOutput basicVertexShader( SrGPU* gpu, SrVertex& input ) {
	SrVsOutput out;
	vec4 pos = vec4(input.position.xyz, 1.0f);
	out.position = matProjection * (matView * (matWorld * pos));
	out.worldPosition = matWorld * pos;
	out.color = input.color;
	out.normal = matWorld * input.normal;
	out.tangent = matWorld * input.tangent;
	out.uv = input.uv;
	return out;
}
vec3 tonemap(vec3 color) {
	return vec3(1.0) - exp(-color);
}
vec3 linearToSrgb(vec3 color) {
	return pow(color, vec3(1.0f / 2.2f));
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	vec3 oneMinusR = vec3(1.0f - roughness);
	return F0 + (max(oneMinusR, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}
// Many thanks to Joey De Vries for his helpful work
vec4 PBRFragmentShader( SrGPU* gpu, SrFsInput& input) {
	SrTexture* albedoSampler, * normalSampler, * mroSampler, * radianceSampler, * irradianceSampler, * brdflutSampler;
	albedoSampler = gpu->samplers[0];
	normalSampler = gpu->samplers[1];
	mroSampler = gpu->samplers[2];
	radianceSampler = gpu->samplers[3];
	irradianceSampler = gpu->samplers[4];
	brdflutSampler = gpu->samplers[5];

	// Calculate view ray (view world direction)
	float x = znear / cos(fovx*0.5f);
	float y = znear / cos(fovy*0.5f);
	float w = sin(fovx * 0.5f) * x;
	float h = sin(fovy * 0.5f) * y;
	vec3 N = normalize(input.worldNormal);
	vec3 V = normalize(-forward * znear + up * input.position.y * h + right * input.position.x * w);
	if (drawingBackground) // display environment background
		return vec4(linearToSrgb(tonemap(radianceSampler->sampleCubemap(V).xyz)) , 1.0f);
	
	vec3 albedo = albedoSampler->sample(input.uv,true).rgb;
	vec3 mro = mroSampler->sample(input.uv,true).rgb;
	float metallic = mro.r;
	float roughness = mro.g;
	float occlusion = mro.b; 

	// Normal mapping (adjust N with the TBN and normal map component)
	vec3 tangentNormal = normalSampler->sample(input.uv,true).xyz * 2.0f - vec3(1, 1, 1);
	tangentNormal *= vec3(1.0f, -1.0f, 1.0f);
	vec3 T = normalize(input.worldTangent - N * dot(input.worldTangent, N));
	vec3 B = normalize(cross(T, N));
	mat3 TBN = mat3(T, B, N);
	N = normalize(TBN*tangentNormal); // recompute the normal
	vec3 R = reflect(V, N); // get the reflection vector

	// Compute the diffuse and specular coefficients
	vec3 F0 = lerp(vec3(0.04f), albedo, vec3(metallic));
	float mdnvz = max(dot(N, V), 0.0f);
	vec3 ks = fresnelSchlickRoughness(mdnvz, F0, roughness);
	vec3 kd = (vec3(1.0f) - ks) * (1.0f - metallic);

	// Compute the diffuse color component with the irradiance cubemap
	vec3 irradiance = irradianceSampler->sampleCubemap(-N);
	vec3 diffuse = irradiance * albedo;
	
	// Compute the specular color component with the radiance cubemap
	float trilinearCoefficient = (radianceSampler->getMipmapCount()-1) * roughness;
	vec3 radiance = radianceSampler->sampleCubemap(R, true, true, trilinearCoefficient);

	// TODO: brdf coefficients are fishy
	vec2 brdfuv = float2(1.0f-mdnvz,1.0f-roughness); 
	vec2 brdf = brdflutSampler->sample(brdfuv).rg;
	float brdfx = brdf.x;
	brdfuv = float2(mdnvz, roughness);
	float brdfy = brdflutSampler->sample(brdfuv).y;
	vec3 specular = radiance * (ks*brdfx+brdfy);  
	vec3 ambient = (kd * diffuse + specular) * occlusion; 
	return vec4(linearToSrgb(tonemap(ambient)), 1.0f);
}