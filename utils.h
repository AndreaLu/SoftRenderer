// Author: Andrea Luzzati
#ifndef SR_UTILS_H
#define SR_UTILS_H

#include "gpu.h"

using namespace glm;

// help functions
vec3 trisComputeTangent(SrVertex& v1, SrVertex& v2, SrVertex& v3) {
	vec3 p1 = v1.position.xyz, p2 = v2.position.xyz, p3 = v3.position.xyz;
	vec2 u1 = v1.uv, u2 = v2.uv, u3 = v3.uv;
	vec3 e1, e2, e3;
	e1 = p2 - p1;
	e2 = p3 - p2;
	e3 = p1 - p3;
	vec2 duv1, duv2, duv3;
	duv1 = u2 - u1;
	duv2 = u3 - u2;
	duv3 = u1 - u3;
	float determinant = duv1.x * duv2.y - duv1.y * duv2.x, f;
	// Determinant cannot be 0
	f = 1 / determinant;
	return e1 * (f * duv2.y) - e2 * (f * duv1.y);
}
vec3 removeParallelComponent(vec3 vecA, vec3 vecB) {
	// returns vecA withotu the parallel component to vecB
	float dotP = dot(vecA, vecB);
	float modBSquare = dot(vecB, vecB);
	float coeff = dotP / modBSquare;
	return vec3(vecA - vecB * coeff);
}
// Loads a basic cube mesh for testing purposes
SrMesh loadCube() {
	SrMesh cube;
	SrTriangle tris;

	vec4 color = vec4(1, 0, 0, 1);
	vec3 tangent, bitangent;
	tris.a.position = vec4(-.5, .5, .5, 1);
	tris.a.normal = vec4(0, 0, 1, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(.5, .5, .5, 1);
	tris.b.normal = vec4(0, 0, 1, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(-.5, -.5, .5, 1);
	tris.c.normal = vec4(0, 0, 1, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent,0.0f);
	tris.a.bitangent = vec4(bitangent,0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(-.5, -.5, .5, 1);
	tris.a.normal = vec4(0, 0, 1, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 1);
	tris.b.position = vec4(.5, .5, .5, 1);
	tris.b.normal = vec4(0, 0, 1, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(.5, -.5, .5, 1);
	tris.c.normal = vec4(0, 0, 1, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	color = vec4(1, 1, 0, 1);

	tris.a.position = vec4(-.5, .5, -.5, 1);
	tris.a.normal = vec4(0, 0, -1, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(-.5, -.5, -.5, 1);
	tris.b.normal = vec4(0, 0, -1, 0);
	tris.b.color = color;
	tris.b.uv = vec2(0, 1);
	tris.c.position = vec4(.5, -.5, -.5, 1);
	tris.c.normal = vec4(0, 0, -1, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(.5, -.5, -.5, 1);
	tris.a.normal = vec4(0, 0, -1, 0);
	tris.a.color = color;
	tris.a.uv = vec2(1, 1);
	tris.b.position = vec4(.5, .5, -.5, 1);
	tris.b.normal = vec4(0, 0, -1, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(-.5, .5, -.5, 1);
	tris.c.normal = vec4(0, 0, -1, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 0);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	color = vec4(1, 1, 1, 1);

	tris.a.position = vec4(-.5, -.5, .5, 1);
	tris.a.normal = vec4(0, -1, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(.5, -.5, .5, 1);
	tris.b.normal = vec4(0, -1, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(-.5, -.5, -.5, 1);
	tris.c.normal = vec4(0, -1, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(-.5, -.5, -.5, 1);
	tris.a.normal = vec4(0, -1, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 1);
	tris.b.position = vec4(.5, -.5, .5, 1);
	tris.b.normal = vec4(0, -1, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(.5, -.5, -.5, 1);
	tris.c.normal = vec4(0, -1, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	color = vec4(1, 0, 1, 1);

	tris.a.position = vec4(-.5, .5, .5, 1);
	tris.a.normal = vec4(-1, 0, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(-.5, -.5, .5, 1);
	tris.b.normal = vec4(-1, 0, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(-.5, -.5, -.5, 1);
	tris.c.normal = vec4(-1, 0, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(-.5, -.5, -.5, 1);
	tris.a.normal = vec4(-1, 0, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(1, 1);
	tris.b.position = vec4(-.5, .5, -.5, 1);
	tris.b.normal = vec4(-1, 0, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(0, 1);
	tris.c.position = vec4(-.5, .5, .5, 1);
	tris.c.normal = vec4(-1, 0, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 0);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	color = vec4(0, 1, 1, 1);

	tris.a.position = vec4(.5, .5, .5, 1);
	tris.a.normal = vec4(0, 1, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(-.5, .5, .5, 1);
	tris.b.normal = vec4(0, 1, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(-.5, .5, -.5, 1);
	tris.c.normal = vec4(0, 1, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(-.5, .5, -.5, 1);
	tris.a.normal = vec4(0, 1, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(1, 1);
	tris.b.position = vec4(.5, .5, -.5, 1);
	tris.b.normal = vec4(0, 1, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(0, 1);
	tris.c.position = vec4(.5, .5, .5, 1);
	tris.c.normal = vec4(0, 1, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 0);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	color = vec4(0, 1, 0, 1);

	tris.a.position = vec4(.5, -.5, .5, 1);
	tris.a.normal = vec4(1, 0, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(0, 0);
	tris.b.position = vec4(.5, .5, .5, 1);
	tris.b.normal = vec4(1, 0, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(1, 0);
	tris.c.position = vec4(.5, .5, -.5, 1);
	tris.c.normal = vec4(1, 0, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(1, 1);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	tris.a.position = vec4(.5, .5, -.5, 1);
	tris.a.normal = vec4(1, 0, 0, 0);
	tris.a.color = color;
	tris.a.uv = vec2(1, 1);
	tris.b.position = vec4(.5, -.5, -.5, 1);
	tris.b.normal = vec4(1, 0, 0, 0);
	tris.b.color = color;
	tris.b.uv = vec2(0, 1);
	tris.c.position = vec4(.5, -.5, .5, 1);
	tris.c.normal = vec4(1, 0, 0, 0);
	tris.c.color = color;
	tris.c.uv = vec2(0, 0);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.a.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.a.normal.xyz)));
	tris.a.tangent = vec4(tangent, 0.0f);
	tris.a.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.b.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.b.normal.xyz)));
	tris.b.tangent = vec4(tangent, 0.0f);
	tris.b.bitangent = vec4(bitangent, 0.0f);
	tangent = trisComputeTangent(tris.a, tris.b, tris.c);
	tangent = normalize(removeParallelComponent(tangent, tris.c.normal.xyz));
	bitangent = normalize(cross(tangent, vec3(tris.c.normal.xyz)));
	tris.c.tangent = vec4(tangent, 0.0f);
	tris.c.bitangent = vec4(bitangent, 0.0f);
	cube.push_back(tris);

	return cube;
}
/* Load a mesh from a buffer file. The buffer file is raw data containing the series
of vertices, one by one. The vertices are assumed to be in trianglelist style (three vertices for each triangle).
The vertex structure is assumed to be as follows:
3xfloat (12 bytes) model space position (x,y,z)
4xchar  (4 bytes) color in rgba u8 format
3xfloat (12 bytes) model space normal (x,y,z)
2xfloat (8 bytes) uv texture coordinates
3xfloat (12 bytes) model space tangent
3xfloat (12 bytes) model space bitangent
This is 60 bytes per vertex, not the best in terms of memory efficiency.
I plan do add assimp library in the future to support standard 3D model formats.
*/
SrMesh loadMeshBuffer(const char* filename) {
	FILE* pFile;
	fopen_s(&pFile, filename, "rb");
	fseek(pFile, 0, SEEK_END);
	size_t numBytes = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	SrVertex v;
	SrMesh retMesh;
	SrTriangle tris;
	float data[11];
	unsigned char rgba[4];
	for (int i = 0; i < numBytes / 180; i++) {
		// read tris by tris
		fread(data, 4, 3, pFile);
		fread(rgba, 1, 4, pFile);
		tris.a.position.x = data[0];
		tris.a.position.y = data[1];
		tris.a.position.z = data[2];
		tris.a.position.w = 1.0f;
		tris.a.color.r = (float)rgba[0] / 255.0f;
		tris.a.color.g = (float)rgba[1] / 255.0f;
		tris.a.color.b = (float)rgba[2] / 255.0f;
		tris.a.color.a = (float)rgba[3] / 255.0f;
		fread(data, 4, 11, pFile);
		tris.a.normal.x = data[0];
		tris.a.normal.y = data[1];
		tris.a.normal.z = data[2];
		tris.a.normal.w = 0.0f;
		tris.a.uv.x = data[3];
		tris.a.uv.y = data[4];
		tris.a.tangent.x = data[5];
		tris.a.tangent.y = data[6];
		tris.a.tangent.z = data[7];
		tris.a.tangent.w = 0.0f;
		tris.a.bitangent.x = data[8];
		tris.a.bitangent.y = data[9];
		tris.a.bitangent.z = data[10];
		tris.a.bitangent.w = 0.0f;

		fread(data, 4, 3, pFile);
		fread(rgba, 1, 4, pFile);
		tris.b.position.x = data[0];
		tris.b.position.y = data[1];
		tris.b.position.z = data[2];
		tris.b.position.w = 1.0f;
		tris.b.color.r = (float)rgba[0] / 255.0f;
		tris.b.color.g = (float)rgba[1] / 255.0f;
		tris.b.color.b = (float)rgba[2] / 255.0f;
		tris.b.color.a = (float)rgba[3] / 255.0f;
		fread(data, 4, 11, pFile);
		tris.b.normal.x = data[0];
		tris.b.normal.y = data[1];
		tris.b.normal.z = data[2];
		tris.b.normal.w = 0.0f;
		tris.b.uv.x = data[3];
		tris.b.uv.y = data[4];
		tris.b.tangent.x = data[5];
		tris.b.tangent.y = data[6];
		tris.b.tangent.z = data[7];
		tris.b.tangent.w = 0.0f;
		tris.b.bitangent.x = data[8];
		tris.b.bitangent.y = data[9];
		tris.b.bitangent.z = data[10];
		tris.b.bitangent.w = 0.0f;

		fread(data, 4, 3, pFile);
		fread(rgba, 1, 4, pFile);
		tris.c.position.x = data[0];
		tris.c.position.y = data[1];
		tris.c.position.z = data[2];
		tris.c.position.w = 1.0f;
		tris.c.color.r = (float)rgba[0] / 255.0f;
		tris.c.color.g = (float)rgba[1] / 255.0f;
		tris.c.color.b = (float)rgba[2] / 255.0f;
		tris.c.color.a = (float)rgba[3] / 255.0f;
		fread(data, 4, 11, pFile);
		tris.c.normal.x = data[0];
		tris.c.normal.y = data[1];
		tris.c.normal.z = data[2];
		tris.c.normal.w = 0.0f;
		tris.c.uv.x = data[3];
		tris.c.uv.y = data[4];
		tris.c.tangent.x = data[5];
		tris.c.tangent.y = data[6];
		tris.c.tangent.z = data[7];
		tris.c.tangent.w = 0.0f;
		tris.c.bitangent.x = data[8];
		tris.c.bitangent.y = data[9];
		tris.c.bitangent.z = data[10];
		tris.c.bitangent.w = 0.0f;

		retMesh.push_back(tris);
	}
	fclose(pFile);
	return retMesh;
}
#endif