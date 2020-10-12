// Author: Andrea Luzzati
#ifndef SR_GPU_H
#define SR_GPU_H
#include "texture.h" // includes vector,glm,iostream,stb_image
#include <limits>    // for float max (depthbuffer clearing)

using namespace glm;

struct SrVertex {
	vec4 position;
	vec4 normal;
	vec4 tangent;
	vec4 bitangent;
	vec4 color;
	vec2 uv;
};
struct SrTriangle {
	SrVertex a, b, c;
};
struct SrVsOutput {
	vec4 position;      // clip space position (-1,1)
	vec4 worldPosition; // world space position
	vec4 normal;        // world space normal
	vec4 tangent;       // world space tangent
	vec4 color;
	vec2 uv;
};
struct SrFsInput {
	vec3 worldPosition;
	vec3 worldNormal;
	vec3 worldTangent;
	vec2 position; // screen space position
	vec2 uv;
	vec4 color;
};
typedef std::vector<SrTriangle> SrMesh;

class SrGPU {
private:
	// Implementation of triangle raster adapted from scratchpixel.com 
	void rasterizeTriangle(SrVsOutput& o1, SrVsOutput& o2, SrVsOutput& o3);
	void standardRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3);
	// The following private functions are my implementation of an optimized triangle raster, not yet in use because of small glitches to be fixed
	bool horizontalRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3);
	bool verticalRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3);
	vec3 computeBarycentricCoefficients(SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3, vec2 ssp, float trisSSArea);
	vec3 correctBarycentricCoefficients(SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3, vec3& baryCoeffs);

	
public:
	typedef enum CullMode {
		NOCULLING = 0,
		CLOCKWISE = 1,
		COUNTERCLOCKWISE = -1
	};
	SrTexture* depthBuffer; // TODO: would be better to make it private
	std::vector<SrTexture*> samplers; // TODO: would be better to make it private
	SrTexture* backBuffer; // TODO: would be better to make it 

	// Vertex shader function pointer to allow for custom pipeline
	SrVsOutput(*vertexShaderProgram)(SrGPU*,SrVertex&);
	// Fragment shader function pointer to allow for custom pipeline
	vec4(*fragmentShaderProgram)(SrGPU*,SrFsInput&);
	// Initialize the gpu with a given viewport size
	SrGPU(const int viewportWidth, const int viewportHeight);
	~SrGPU();
	// Render a 3D mesh
	void submitMesh(SrMesh& triangle, const CullMode culling = NOCULLING);
	// Clear backbuffer and depthbuffer to initialize the rendering cycle
	void clearBuffers(const vec4 color=vec4(0,0,0,1));
	// Fills the screen through the fragment shader
	void drawFillQuad();
};


// GPU IMPLEMENTATION
SrGPU::SrGPU(const int vpw, const int vph) {
	backBuffer = new SrTexture();
	depthBuffer = new SrTexture();
	backBuffer->textureFromColor(vpw, vph, vec4(0, 0, 0, 1));
	depthBuffer->textureFromColor(vpw, vph, vec4(std::numeric_limits<float>::max()));
}
SrGPU::~SrGPU() {
	delete backBuffer;
	delete depthBuffer;
}
void SrGPU::clearBuffers(const vec4 col) {
	backBuffer->clear(col);
	depthBuffer->clear(vec4(std::numeric_limits<float>::max()));
}
void SrGPU::submitMesh(SrMesh& mesh, const SrGPU::CullMode culling) {
	std::vector<SrTriangle>::iterator it;
	vec2 size = vec2(backBuffer->getTextureWidth(), backBuffer->getTextureHeight());
	SrVsOutput vso1, vso2, vso3;
	SrFsInput fragmentInput;
	for (it = mesh.begin(); it != mesh.end(); it++) {
		vso1 = vertexShaderProgram(this,(*it).a);
		vso2 = vertexShaderProgram(this,(*it).b);
		vso3 = vertexShaderProgram(this,(*it).c);
		
		// Perspective correction
		vso1.position = vec4(vso1.position.xyz * (1.0f / vso1.position.w), vso1.position.w);
		vso2.position = vec4(vso2.position.xyz * (1.0f / vso2.position.w), vso2.position.w);
		vso3.position = vec4(vso3.position.xyz * (1.0f / vso3.position.w), vso3.position.w);

		if (culling != CullMode::NOCULLING) {
			vec3 viewRay(0, 0, culling == CullMode::CLOCKWISE ? 1 : -1);
			vec3 normal = cross(vec3(vso3.position.xyz - vso1.position.xyz), vec3(vso2.position.xyz - vso1.position.xyz));
			if (dot(viewRay, normal) < 0) continue;
		}
		
		rasterizeTriangle(vso1, vso2, vso3);		
	}
}
vec3 SrGPU::computeBarycentricCoefficients(SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3, vec2 ssp, float ABCArea) {
	float CAPArea = length(cross(vec3(ssp - svo1.position.xy, 0.0f), vec3(svo3.position.xy - svo1.position.xy, 0.0f))) * 0.5f;
	float ABPArea = length(cross(vec3(svo2.position.xy - svo1.position.xy, 0.0f), vec3(ssp - svo1.position.xy, 0.0f))) * 0.5f;
	float u, v, w;
	u = CAPArea / ABCArea;
	v = ABPArea / ABCArea;
	w = 1.0f - u - v;
	return vec3(w, u, v);
}
vec3 SrGPU::correctBarycentricCoefficients(SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3, vec3& baryCoeffs) {
	float den = baryCoeffs.x / svo1.position.w + baryCoeffs.y / svo2.position.w + baryCoeffs.z / svo3.position.w;
	float u, v, w;
	w = baryCoeffs.x / (svo1.position.w * den);
	u = baryCoeffs.y / (svo2.position.w * den);
	v = baryCoeffs.z / (svo3.position.w * den);
	return vec3(w, u, v);
}
bool SrGPU::horizontalRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3) {
	vec2 bufferSize(backBuffer->getTextureWidth(), backBuffer->getTextureHeight());
	vec2 A, B, C;
	// Sort the vertices
	if (p1.y == p2.y) {
		if (p1.x < p2.x) {
			A = p1;
			B = p2;
		}
		else {
			A = p2;
			B = p1;
		}
		C = p3;
	}
	else {

		if (p1.y == p3.y) {
			if (p1.x < p3.x) {
				A = p1;
				B = p3;
			}
			else {
				A = p3;
				B = p1;
			}
			C = p2;
		}
		else { // o2.position.y == p3.position.y
			if (p2.x < p3.x) {
				A = p2;
				B = p3;
			}
			else {
				A = p3;
				B = p2;
			}
			C = p1;
		}
	}



	// Draw horizontal scanlines. e0 and e1 will be the edge points of the lines
	// respectively e0 the left edge, e1 the right edge
	vec2 e0 = round(A), e1 = round(B);
	vec2 l = C - A;
	vec2 m = C - B;
	if (abs(l.y) < 0.00001f || abs(m.y) < 0.00001f) return false; // degenerate triangle

	float kl = 1.0f / l.y;
	float km = 1.0f / m.y;
	// Count how many steps it will take to complete
	int steps = (floor((C.y - e0.y) / (kl * l.y)));
	int x0, x1;
	float sign = steps > 0 ? 1.0f : -1.0f;
	float ABCArea = length(cross(vec3(svo2.position.xy - svo1.position.xy, 0.0f), vec3(svo3.position.xy - svo1.position.xy, 0.0f))) * 0.5f;
	//float u, v, w, z, den;
	vec3 bary, pBary, bary0, pBary0, bary1, pBary1;
	//float up, vp, wp;
	float z, puvac; // pixel uv area coverage
	SrFsInput fsInput;
	vec2 uv, uv0, uv1;
	for (int i = 0; i < abs(steps); i++) {
		// Clip pixels outside the backbuffer
		if (e0.y < 0 || e0.y >= backBuffer->getTextureHeight()) continue;
		x0 = clamp((int)(e0.x-1), 0, backBuffer->getTextureWidth() - 1);
		x1 = clamp((int)(e1.x+1), 0, backBuffer->getTextureWidth() - 1);
		// Draw the horizontal scanline
		for (int x = x0; x <= x1; x++) {
			// Compute the barycentric coefficients of x,y with respect to A,B,C
			vec2 p(x, e0.y);
			// barycentric coefficients needed for the calculation of the UV of the two 
			bary0 = computeBarycentricCoefficients(svo1, svo2, svo3, p - vec2(1,1), ABCArea);
			pBary0 = correctBarycentricCoefficients(svo1, svo2, svo3, bary0);
			bary1 = computeBarycentricCoefficients(svo1, svo2, svo3, p + vec2(1, 1), ABCArea);
			pBary1 = correctBarycentricCoefficients(svo1, svo2, svo3, bary1);

			bary = computeBarycentricCoefficients(svo1, svo2, svo3, p, ABCArea);
			pBary = correctBarycentricCoefficients(svo1, svo2, svo3, bary);
			// Interpolate vertex parameters to this fragment
			z = pBary.x * svo1.position.z + pBary.y * svo2.position.z + pBary.z * svo3.position.z;
			if (depthBuffer->read(x, e0.y).x > z) { // Depth test
				depthBuffer->write(x, e0.y, vec4(z, z, z, 1));
				uv0 = pBary0.x * svo1.uv + pBary0.y * svo2.uv + pBary0.z * svo3.uv;
				uv1 = pBary1.x * svo1.uv + pBary1.y * svo2.uv + pBary1.z * svo3.uv;
				puvac = abs((uv1.x - uv0.x) * (uv1.y - uv0.y)) * 0.25f; // pixel uv area coverage for the pixel being drawn
				// the 0.25f is an adjustement introduced because the UV is calculated along the diagonal of a square of 2x2 pixels
				for(std::vector<SrTexture*>::iterator it = samplers.begin(); it!=samplers.end(); it++) 
					(*it)->calculateTrilinearCoefficient(puvac);
				fsInput.uv = pBary.x * svo1.uv + pBary.y * svo2.uv + pBary.z * svo3.uv;
				// Also compute the UVs of the other pixels
				fsInput.worldPosition = (pBary.x * svo1.worldPosition + pBary.y * svo2.worldPosition + pBary.z * svo3.worldPosition).xyz;
				fsInput.worldNormal = (pBary.x * svo1.normal + pBary.y * svo2.normal + pBary.z * svo3.normal).xyz;
				fsInput.worldTangent = (pBary.x * svo1.tangent + pBary.y * svo2.tangent + pBary.z * svo3.tangent).xyz;
				fsInput.position = p / bufferSize * 2.0f - vec2(1.0f, 1.0f);
				fsInput.color = bary.x * svo1.color + bary.y * svo2.color + bary.z * svo3.color;
				backBuffer->write(x, e0.y, fragmentShaderProgram(this, fsInput));
			}
		}
		// move e0 and e1
		e0 += sign * l * kl;
		e1 += sign * m * km;
	}
	return true;
}
bool SrGPU::verticalRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3) {

	vec2 bufferSize(backBuffer->getTextureWidth(), backBuffer->getTextureHeight());
	vec2 A, B, C;
	// Sort the vertices
	if (p1.x == p2.x) {
		if (p1.y < p2.y) {
			A = p1;
			B = p2;
		}
		else {
			A = p2;
			B = p1;
		}
		C = p3;
	}
	else {

		if (p1.x == p3.x) {
			if (p1.y < p3.y) {
				A = p1;
				B = p3;
			}
			else {
				A = p3;
				B = p1;
			}
			C = p2;
		}
		else { // o2.position.x == p3.position.x
			if (p2.y < p3.y) {
				A = p2;
				B = p3;
			}
			else {
				A = p3;
				B = p2;
			}
			C = p1;
		}
	}

	// Draw vertical scanlines. e0 and e1 will be the edge points of the lines
	// respectively e0 the left edge, e1 the right edge
	vec2 e0 = round(A), e1 = round(B);
	vec2 l = C - A;
	vec2 m = C - B;
	if (abs(l.x) < 0.00001f || abs(m.x) < 0.00001f)  return false; // degenerate triangle
	float kl = 1.0f / l.x; // kl*l.x = 1 -> makes me move to the next vertical scanline
	float km = 1.0f / m.x;
	// Count how many steps it will take to complete
	int steps = (floor((C.x - e0.x) / (kl * l.x)));
	int y0, y1;
	float sign = steps > 0 ? 1.0f : -1.0f;
	float ABCArea = length(cross(vec3(svo2.position.xy - svo1.position.xy, 0.0f), vec3(svo3.position.xy - svo1.position.xy, 0.0f))) * 0.5f;
	//float u, v, w, z, den;
	vec3 bary, pBary, bary0, pBary0, bary1, pBary1;
	//float up, vp, wp;
	float z, puvac; // pixel uv area coverage
	SrFsInput fsInput;
	vec2 uv, uv0, uv1;
	for (int i = 0; i < abs(steps); i++) {
		// Clip pixels outside the backbuffer
		if (e0.x < 0 || e0.x >= backBuffer->getTextureWidth()) continue;
		y0 = clamp((int)(e0.y - 1), 0, backBuffer->getTextureWidth() - 1); //overdraw to avoid voids
		y1 = clamp((int)(e1.y + 1), 0, backBuffer->getTextureWidth() - 1);
		// Draw the horizontal scanline

		for (int y = y0; y <= y1; y++) {
			// Compute the barycentric coefficients of x,y with respect to A,B,C
			vec2 p(e0.x, y);
			// barycentric coefficients needed for the calculation of the UV of the two 
			bary0 = computeBarycentricCoefficients(svo1, svo2, svo3, p - vec2(1, 1), ABCArea);
			pBary0 = correctBarycentricCoefficients(svo1, svo2, svo3, bary0);
			bary1 = computeBarycentricCoefficients(svo1, svo2, svo3, p + vec2(1, 1), ABCArea);
			pBary1 = correctBarycentricCoefficients(svo1, svo2, svo3, bary1);

			bary = computeBarycentricCoefficients(svo1, svo2, svo3, p, ABCArea);
			pBary = correctBarycentricCoefficients(svo1, svo2, svo3, bary);
			// Interpolate vertex parameters to this fragment
			z = pBary.x * svo1.position.z + pBary.y * svo2.position.z + pBary.z * svo3.position.z;
			if (depthBuffer->read(e0.x, y).x > z) { // Depth test
				depthBuffer->write(e0.x, y, vec4(z, z, z, 1));
				uv0 = pBary0.x * svo1.uv + pBary0.y * svo2.uv + pBary0.z * svo3.uv;
				uv1 = pBary1.x * svo1.uv + pBary1.y * svo2.uv + pBary1.z * svo3.uv;
				puvac = abs((uv1.x - uv0.x) * (uv1.y - uv0.y)) * 0.25f; // pixel uv area coverage for the pixel being drawn
				// the 0.25f is an adjustement introduced because the UV is calculated along the diagonal of a square of 2x2 pixels
				for (std::vector<SrTexture*>::iterator it = samplers.begin(); it != samplers.end(); it++)
					(*it)->calculateTrilinearCoefficient(puvac);
				fsInput.uv = pBary.x * svo1.uv + pBary.y * svo2.uv + pBary.z * svo3.uv;
				// Also compute the UVs of the other pixels
				fsInput.worldPosition = (pBary.x * svo1.worldPosition + pBary.y * svo2.worldPosition + pBary.z * svo3.worldPosition).xyz;
				fsInput.worldNormal = (pBary.x * svo1.normal + pBary.y * svo2.normal + pBary.z * svo3.normal).xyz;
				fsInput.worldTangent = (pBary.x * svo1.tangent + pBary.y * svo2.tangent + pBary.z * svo3.tangent).xyz;
				fsInput.position = p / bufferSize * 2.0f - vec2(1.0f, 1.0f);
				fsInput.color = bary.x * svo1.color + bary.y * svo2.color + bary.z * svo3.color;
				backBuffer->write(e0.x, y, fragmentShaderProgram(this, fsInput));
			}
		}
		// move e0 and e1
		e0 += sign * l * kl;
		e1 += sign * m * km;
	}
	return true;
}
void SrGPU::rasterizeTriangle(SrVsOutput& o1, SrVsOutput& o2, SrVsOutput& o3) {
	// Viewport adaptation
	vec2 viewportSize(backBuffer->getTextureWidth(), backBuffer->getTextureHeight());
	o1.position.xy = (o1.position.xy + vec2(1.0f, 1.0f)) * 0.5f * viewportSize;
	o2.position.xy = (o2.position.xy + vec2(1.0f, 1.0f)) * 0.5f * viewportSize;
	o3.position.xy = (o3.position.xy + vec2(1.0f, 1.0f)) * 0.5f * viewportSize;

	standardRasterTriangle(o1.position.xy, o3.position.xy, o2.position.xy, o1, o3, o2);
	return;
	// It follows the code for my tests for optimized raster scanline algorithms that I'm not yet using due to some small glitches
	// Compute viewport triangle area
	float triangleArea = length(cross(vec3(o3.position.xy - o1.position.xy, 0), vec3(o2.position.xy - o1.position.xy, 0))) * 0.5f;

	// Decide between vertical and horizontal scanlines
	vec2 topLeft = min(min(vec2(o1.position.xy), vec2(o2.position.xy)), vec2(o3.position.xy));
	vec2 bottomRight = max(max(vec2(o1.position.xy), vec2(o2.position.xy)), vec2(o3.position.xy));
	vec2 size = bottomRight - topLeft;

	bool success = false;
	if (size.x > size.y) { // horizontal scanline
		// We need a triangle where two vertices share the same Y (horizontal edge)
		if (o1.position.y == o2.position.y || o1.position.y == o2.position.y || o3.position.y == o2.position.y)
			success = horizontalRasterTriangle(o1.position.xy, o2.position.xy, o3.position.xy, o1, o2, o3);
		else {
			// Split the triangle in two that have a horizontal edge
			// create p4 along the edge to be split
			vec2 o4; vec2 r; float k;
			if (o1.position.y > topLeft.y && o1.position.y < bottomRight.y) {
				r = o3.position.xy - o2.position.xy;
				k = (o1.position.y - o2.position.y) / r.y;
				o4 = o2.position.xy + r * k; o4.y = o1.position.y;
				success = horizontalRasterTriangle(o1.position.xy, o2.position.xy, o4, o1, o2, o3);
				success = success && horizontalRasterTriangle(o1.position.xy, o3.position.xy, o4, o1, o2, o3);
			}
			else if (o2.position.y > topLeft.y && o2.position.y < bottomRight.y) {
				r = o3.position.xy - o1.position.xy;
				k = (o2.position.y - o1.position.y) / r.y;
				o4 = o1.position.xy + r * k; o4.y = o2.position.y;
				success = horizontalRasterTriangle(o2.position.xy, o1.position.xy, o4, o1, o2, o3);
				success = success && horizontalRasterTriangle(o2.position.xy, o3.position.xy, o4, o1, o2, o3);
			}
			else { // p3.position in the middle
				r = o2.position.xy - o1.position.xy;
				k = (o3.position.y - o1.position.y) / r.y;
				o4 = o1.position.xy + r * k; o4.y = o3.position.y;
				success = horizontalRasterTriangle(o3.position.xy, o2.position.xy, o4, o1, o2, o3);
				success = success && horizontalRasterTriangle(o3.position.xy, o1.position.xy, o4, o1, o2, o3);
			}
		}
	}
	else { //vertical scanline attempt
		// We need a triangle where two vertices share the same X (vertical edge)
		if (o1.position.x == o2.position.x || o1.position.x == o2.position.x || o3.position.x == o2.position.x)
			success = verticalRasterTriangle(o1.position.xy, o2.position.xy, o3.position.xy, o1, o2, o3);
		else {
			// Split the triangle in two that have a horizontal edge
			// create p4 along the edge to be split
			vec2 o4; vec2 r; float k;
			if (o1.position.x > topLeft.x && o1.position.x < bottomRight.x) { // o1 is horizontally in the middle
				r = o3.position.xy - o2.position.xy;
				k = (o1.position.x - o2.position.x) / r.x;
				o4 = o2.position.xy + r * k; o4.x = o1.position.x;
				success = verticalRasterTriangle(o1.position.xy, o2.position.xy, o4, o1, o2, o3);
				success = success && verticalRasterTriangle(o1.position.xy, o3.position.xy, o4, o1, o2, o3);
			}
			else if (o2.position.x > topLeft.x && o2.position.x < bottomRight.x) {
				r = o3.position.xy - o1.position.xy;
				k = (o2.position.x - o1.position.x) / r.x;
				o4 = o1.position.xy + r * k; o4.x = o2.position.x;
				success = verticalRasterTriangle(o2.position.xy, o1.position.xy, o4, o1, o2, o3);
				success = success && verticalRasterTriangle(o2.position.xy, o3.position.xy, o4, o1, o2, o3);
			}
			else { // p3.position in the middle
				r = o2.position.xy - o1.position.xy;
				k = (o3.position.x - o1.position.x) / r.x;
				o4 = o1.position.xy + r * k; o4.x = o3.position.x;
				success = verticalRasterTriangle(o3.position.xy, o2.position.xy, o4, o1, o2, o3);
				success = success && verticalRasterTriangle(o3.position.xy, o1.position.xy, o4, o1, o2, o3);
			}
		}
	}
	if (!success)
		std::cout << "could not rasterize triangle" << std::endl;
}
float edgeFunction(const vec2& a, const vec2& b, const vec2& c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y- a.y) * (b.x - a.x);
}
vec3 _computeBarycentricCoefficients(const vec2& p1,const vec2& p2,const vec2& p3, const vec2& p, const float area) {
	return vec3(
		edgeFunction(p2, p3, p)/area,
		edgeFunction(p3,p1,p)/area,
		edgeFunction(p1,p2,p)/area
	);
}
vec3 _correctBarycentricCoefficients(SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3, const vec3& baryCoeffs) {
	float den = baryCoeffs.x / svo1.position.w + baryCoeffs.y / svo2.position.w + baryCoeffs.z / svo3.position.w;
	float u, v, w;
	w = baryCoeffs.x / (svo1.position.w * den);
	u = baryCoeffs.y / (svo2.position.w * den);
	v = baryCoeffs.z / (svo3.position.w * den);
	return vec3(w, u, v);
}
void SrGPU::standardRasterTriangle(vec2 p1, vec2 p2, vec2 p3, SrVsOutput& svo1, SrVsOutput& svo2, SrVsOutput& svo3)
{
	int tw = backBuffer->getTextureWidth();
	int th = backBuffer->getTextureHeight();
	vec2 bufferSize(backBuffer->getTextureWidth(), backBuffer->getTextureHeight());
	int minx = min(min(p1.x, p2.x), p3.x) - 1;
	int miny = min(min(p1.y, p2.y), p3.y) - 1;
	int maxx = max(max(p1.x, p2.x), p3.x) + 1;
	int maxy = max(max(p1.y, p2.y), p3.y) + 1;
	if (maxx < 0 || minx >= tw || maxy < 0 || miny >= th) return; // clip triangles outside of the screen
	if (minx < 0) minx = 0;
	if (miny < 0) miny = 0;
	if (maxx >= tw) maxx = tw;
	if (maxy >= th) maxy = th;
	//vec2 topLeft = min(min(p1, p2), p3);
	//vec2 bottomRight = max(max(p1, p2), p3);
	float area = edgeFunction(p1, p2, p3);
	vec2 p;
	vec3 bary, pBary, bary0, pBary0, bary1, pBary1;
	vec2 uv0, uv1;
	float z, puvac;
	SrFsInput fsInput;
	for (uint32_t j = miny; j <= maxy; j++) {
		for (uint32_t i = minx; i <= maxx; i++) {
			if (i < 0 || i >= tw || j < 0 || j >= th) continue;
			p = vec2(i + 0.5f, j + 0.5f);
			bary = _computeBarycentricCoefficients(p1, p2, p3, p, area);
			if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
				z = bary.x * svo1.position.z + bary.y * svo2.position.z + bary.z * svo3.position.z;
				if (depthBuffer->read(i, j).x > z) {
					depthBuffer->write(i, j, vec4(z, z, z, 1));

					pBary = _correctBarycentricCoefficients(svo1, svo2, svo3, bary);
					bary0 = _computeBarycentricCoefficients(p1, p2, p3, p - vec2(1, 1), area);
					pBary0 = _correctBarycentricCoefficients(svo1, svo2, svo3, bary0);
					bary1 = _computeBarycentricCoefficients(p1, p2, p3, p + vec2(1, 1), area);
					pBary1 = _correctBarycentricCoefficients(svo1, svo2, svo3, bary1);

					// use perspective corrected barycentric coefficient to calculate uv0,uv1 and uv
					uv0 = pBary0.x * svo1.uv + pBary0.y * svo2.uv + pBary0.z * svo3.uv;
					uv1 = pBary1.x * svo1.uv + pBary1.y * svo2.uv + pBary1.z * svo3.uv;
					fsInput.uv = pBary.x * svo1.uv + pBary.y * svo2.uv + pBary.z * svo3.uv;
					// calculate puvac, the estimated pixel uv area coverage for the pixel we want to draw
					puvac = abs((uv1.x - uv0.x) * (uv1.y - uv0.y)) * 0.25f;
					// the 0.25f is an adjustement introduced because the UV is calculated along the diagonal of a square of 2x2 pixels
					// Update all mipmap interpolation coefficients of the bound samplers to try to achieve the most similar puvac (aka mipmapping)
					for (std::vector<SrTexture*>::iterator it = samplers.begin(); it != samplers.end(); it++)
						(*it)->calculateTrilinearCoefficient(puvac);
					
					// Also compute the UVs of the other pixels
					fsInput.worldPosition = (bary.x * svo1.worldPosition + bary.y * svo2.worldPosition + bary.z * svo3.worldPosition).xyz;
					fsInput.worldNormal = (bary.x * svo1.normal + bary.y * svo2.normal + bary.z * svo3.normal).xyz;
					fsInput.worldTangent = (bary.x * svo1.tangent + bary.y * svo2.tangent + bary.z * svo3.tangent).xyz;
					fsInput.position = p / bufferSize * 2.0f - vec2(1.0f, 1.0f);
					fsInput.color = pBary.x * svo1.color + pBary.y * svo2.color + pBary.z * svo3.color;
					backBuffer->write(i, j, fragmentShaderProgram(this, fsInput));
				}
			}
		}
	}
}
void SrGPU::drawFillQuad() {
	int w = backBuffer->getTextureWidth();
	int h = backBuffer->getTextureHeight();
	SrFsInput input;
	input.worldNormal = vec3(0, 0, 0);
	input.worldPosition = vec3(0, 0, 0);
	input.worldTangent = vec3(0, 0, 0);
	for(int x=0;x<w;x++)
		for (int y = 0; y < h; y++) {
			input.position = vec2((float)x / (float)w * 2.0f - 1.0f, (float)y / (float)h * 2.0f - 1.0f);
			input.color = vec4(1, 1, 1, 1);
			input.uv = vec2((float)x / (float)w, (float)y / (float)h);
			backBuffer->write(x, y, fragmentShaderProgram(this, input));
		}
}
#endif