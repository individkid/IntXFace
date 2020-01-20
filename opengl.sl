/*
*    opengl.sl
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "face.h"

#version 330 core

#ifdef VERTEX
layout (location=0) in ivec3 tags;
layout (location=1) in vec3 plane[3];
layout (location=4) in ivec3 versor;
layout (location=5) in vec2 coord[3];
layout (location=8) in vec4 color[3];
layout (location=11) in ivec3 texid;
layout (location=12) in ivec3 facid;
layout (location=13) in int matid;
#endif

layout (std140) uniform Uniform
{
	mat3 basis[3];
	mat4 subject;
	mat4 object[NUMFILE];
	mat4 feature;
	vec3 feather;
	vec3 arrow;
	vec3 cloud[NUMFEND];
	int hand;
	int tag;
};

vec3[3] constructVector(vec3 plane, int versor, mat3 basis[3])
{
	vec3 retval[3];
	for (int i = 0; i < 3; i++) retval[i] = basis[versor][i];
	for (int i = 0; i < 3; i++) retval[i][versor] = plane[i];
	return retval;
}

vec3 transformVector(vec3 point, mat4 matrix)
{
	vec4 vector = matrix*vec4(point.x,point.y,point.z,1.0);
	return vec3(vector.x,vector.y,vector.z);
}

bool infiniteVector(vec3 vector)
{
	return (length(vector) >= SATURATE);
}

vec3 normalVector(vec3 point[3])
{
	return cross(point[1]-point[0],point[2]-point[0]);
}

vec3 solveVector(vec3 point, vec3 normal, vec3 project)
{
	return point+(project-point-normal*((project-point)*normal));
}

vec3 pierceVector(vec3 point, vec3 normal, vec3 feather, vec3 arrow)
{
	vec3 solve0 = feather-solveVector(point,normal,feather);
	vec3 solve1 = arrow-solveVector(point,normal,arrow);
	return feather+(arrow-feather)*solve0.z/(solve0.z-solve1.z);
}

vec3 intersectVector(vec3 plane[3], ivec3 versor, mat3 basis[3])
{
	return vec3(0.0,0.0,0.0);
}
