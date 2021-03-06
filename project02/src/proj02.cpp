//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include <cmath>
#include <vector>
#include "texture.hpp"

// mat4 -> 4X4 Matrix, vec4 -> 4X4 Vector Matrix
glm::mat4 projectMat;
glm::mat4 viewMat;
glm::mat4 modelMat;
glm::mat4 pvmMat;

// Vertex shader에서 pvM으로 넘겨줄 uniform variable
GLuint pvmMatrixID;

// 회전각
float leftArmAngle = 0.0f;
float rightArmAngle = 0.6f;
float legAngle = 0.5f;

// 서 있는 사람을 그릴 것인가?
int isStaticHuman = true;
// 다리 회전각도 증감연산
int isLegReturn = false;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

// shade and texture
enum eShadeMode { NO_LIGHT, GOURAUD, PHONG, NUM_LIGHT_MODE };
int shadeMode = NO_LIGHT;
int isTexture = false;
int isRotate = false;
GLuint projectMatrixID, viewMatrixID, modelMatrixID, shadeModeID, textureModeID, TextureID;
GLuint program;
GLuint headTexture, bodyTexture, armTexture, legTexture;
glm::vec2 texcoord[4];
vector<glm::vec4> verts, normals;
vector<glm::vec2> texCoords;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 1.0, 1.0, 1.0)  // white
};

//----------------------------------------------------------------------------
// Compute normal
void computeNormals() {
	for (int i = 0; i < verts.size(); i++) {
		glm::vec4 n;
		for (int k = 0; k < 3; k++) {
			n[k] = verts[i][k];
		}
		n[3] = 0.0;
		glm::normalize(n);
		normals.push_back(n);
	}
}

// Compute Texture Coordination
void computeTexCoordQuad(glm::vec2 texcoord[4], float u, float v, float u2, float v2)
{
	const int U = 0, V = 1;

	// v0=(u, v)    v1=(u2, v)   <= quadangle
	// v2=(u, v2)   v3=(u2, v2)

	texcoord[0][U] = texcoord[2][U] = (float)u;
	texcoord[1][U] = texcoord[3][U] = (float)u2;

	texcoord[0][V] = texcoord[1][V] = (float)v;
	texcoord[2][V] = texcoord[3][V] = (float)v2;

	if (u2 == 0) // last column
	{
		texcoord[1][U] = texcoord[3][U] = 1.0;
	}
}

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
// and make triangles
int Index = 0;
void quad(int a, int b, int c, int d)
{
	points[Index] = vertices[a];  verts.push_back(vertices[a]); Index++;
	points[Index] = vertices[b];  verts.push_back(vertices[b]); Index++;
	points[Index] = vertices[c];  verts.push_back(vertices[c]); Index++;
	points[Index] = vertices[a];  verts.push_back(vertices[a]); Index++;
	points[Index] = vertices[c];  verts.push_back(vertices[c]); Index++;
	points[Index] = vertices[d];  verts.push_back(vertices[d]); Index++;

}

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
	quad(1, 0, 3, 2);
	// v0=(u, v)    v1=(u2, v)   <= quadangle
	// v2=(u, v2)   v3=(u2, v2)
	computeTexCoordQuad(texcoord, vertices[1][0], vertices[1][1], vertices[3][0], vertices[3][1]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]);
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	quad(2, 3, 7, 6);
	computeTexCoordQuad(texcoord, vertices[3][1], vertices[3][2], vertices[6][1], vertices[6][2]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]);
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	quad(3, 0, 4, 7);
	computeTexCoordQuad(texcoord, vertices[0][0], vertices[0][2], vertices[7][0], vertices[7][2]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]);
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	quad(6, 5, 1, 2);
	computeTexCoordQuad(texcoord, vertices[1][0], vertices[1][2], vertices[6][0], vertices[6][2]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]); 
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	quad(4, 5, 6, 7);
	computeTexCoordQuad(texcoord, vertices[5][0], vertices[5][1], vertices[7][0], vertices[7][1]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]);
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	quad(5, 4, 0, 1);
	computeTexCoordQuad(texcoord, vertices[0][0], vertices[0][2], vertices[5][0], vertices[5][2]);
	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[2]);
	texCoords.push_back(texcoord[3]);

	texCoords.push_back(texcoord[0]);
	texCoords.push_back(texcoord[3]);
	texCoords.push_back(texcoord[1]);

	computeNormals();
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init()
{
	colorcube();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	int vertSize = sizeof(verts[0]) * verts.size();
	int normalSize = sizeof(normals[0]) * normals.size();
	int texSize = sizeof(texCoords[0]) * texCoords.size();

	glBufferData(GL_ARRAY_BUFFER, vertSize + normalSize + texSize,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, verts.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize, normalSize, normals.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize + normalSize, texSize, texCoords.data());

	// Load shaders and use the resulting shader program
	program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(vertSize));

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(vertSize + normalSize));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM");

	projectMatrixID = glGetUniformLocation(program, "mProject");
	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	viewMatrixID = glGetUniformLocation(program, "mView");
	viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

	modelMatrixID = glGetUniformLocation(program, "mModel");
	modelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);

	shadeModeID = glGetUniformLocation(program, "shadeMode");
	glUniform1i(shadeModeID, shadeMode);

	textureModeID = glGetUniformLocation(program, "isTexture");
	glUniform1i(textureModeID, isTexture);


	// Load the texture using any two methods
	headTexture = loadBMP_custom("brick.bmp");
	bodyTexture = loadBMP_custom("water.bmp");
	armTexture = loadBMP_custom("marble.bmp");
	legTexture = loadBMP_custom("tile.bmp");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(program, "cubeTexture");

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawHuman(glm::mat4 humanMat)
{
	// Head
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, headTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 4));
	modelMat = glm::scale(modelMat, glm::vec3(1, 1, 1));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// Body
	// Bind our texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bodyTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 1
	glUniform1i(TextureID, 1);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 2));
	modelMat = glm::scale(modelMat, glm::vec3(1, 2, 3));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Forearm
	// Bind our texture in Texture Unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, armTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 2
	glUniform1i(TextureID, 2);

	modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 2.8));
	modelMat = glm::rotate(modelMat, 1.0f, glm::vec3(1, 0, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Arm
	modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 2.1));
	modelMat = glm::rotate(modelMat, -0.8f, glm::vec3(1, 0, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Forearm
	modelMat = glm::translate(humanMat, glm::vec3(0, -1.3, 2.6));
	modelMat = glm::rotate(modelMat, -0.6f, glm::vec3(1, 0, 0));
	modelMat = glm::rotate(modelMat, -0.2f, glm::vec3(0, 1, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Arm
	modelMat = glm::translate(humanMat, glm::vec3(0.3, -1.6, 1.7));
	modelMat = glm::rotate(modelMat, 0.3f, glm::vec3(1, 0, 0));
	modelMat = glm::rotate(modelMat, -0.4f, glm::vec3(0, 1, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Upper Leg
	// Bind our texture in Texture Unit 3
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, legTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 3
	glUniform1i(TextureID, 3);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -0.2));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -1.7));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Upper Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.5, -0.2));
	modelMat = glm::rotate(modelMat, -0.2f, glm::vec3(1, 0, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.6, -1.6));
	modelMat = glm::rotate(modelMat, 0.1f, glm::vec3(1, 0, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.8));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}


void swimmingAnim(glm::mat4 humanMat)
{
	// Head
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, headTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 4));
	modelMat = glm::scale(modelMat, glm::vec3(1, 1, 1));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// Body
	// Bind our texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bodyTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 1
	glUniform1i(TextureID, 1);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 2));
	modelMat = glm::scale(modelMat, glm::vec3(1, 2, 3));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Forearm
	// Bind our texture in Texture Unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, armTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 2
	glUniform1i(TextureID, 2);

	modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 2.7));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 0.4));
	modelMat = glm::rotate(modelMat, leftArmAngle * 5.0f, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -0.4));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Arm
	if (leftArmAngle >= 0 && leftArmAngle < 0.45) {
		modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8 * sin(-leftArmAngle * 5.0f), 0, -cos(leftArmAngle * 5.0f) + 1.0));
	}
	else if (leftArmAngle >= 0.45 && leftArmAngle < 0.64) {
		modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8 * sin(-leftArmAngle * 5.0f), 0, -cos(leftArmAngle * 5.0f) + 2.2));
	}
	else {
		modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.6));
		modelMat = glm::rotate(modelMat, leftArmAngle * 5.0f, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.6));
	}
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Forearm
	modelMat = glm::translate(humanMat, glm::vec3(0, -1.3, 2.7));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 0.4));
	modelMat = glm::rotate(modelMat, rightArmAngle * 5.0f, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -0.4));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Arm
	if (rightArmAngle >= 0 && rightArmAngle < 0.45) {
		modelMat = glm::translate(humanMat, glm::vec3(0, -1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8 * sin(-rightArmAngle * 5.0f), 0, -cos(rightArmAngle * 5.0f) + 1.0));
	}
	else if (rightArmAngle >= 0.45 && rightArmAngle < 0.64) {
		modelMat = glm::translate(humanMat, glm::vec3(0, -1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8 * sin(-rightArmAngle * 5.0f), 0, -cos(rightArmAngle * 5.0f) + 2.2));
	}
	else {
		modelMat = glm::translate(humanMat, glm::vec3(0, -1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.6));
		modelMat = glm::rotate(modelMat, rightArmAngle * 5.0f, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.6));
	}
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Upper Leg
	// Bind our texture in Texture Unit 3
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, legTexture);
	// Set our "myTextureSampler" sampler to use Texture Unit 3
	glUniform1i(TextureID, 3);

	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -0.2));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.5));
	if (legAngle > 0.3f) {
		modelMat = glm::rotate(modelMat, 0.3f, glm::vec3(0, 1, 0));
	}
	else {
		modelMat = glm::rotate(modelMat, legAngle, glm::vec3(0, 1, 0));
	}
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.5));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -1.6));
	if (legAngle > 0.3f) {
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 2.2));
		modelMat = glm::rotate(modelMat, legAngle*1.3f, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -2.2));
	}
	else {
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 3.0));
		modelMat = glm::rotate(modelMat, legAngle, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -3.0));
	}
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.8));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Upper Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.5, -0.2));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.5));
	if (-legAngle > 0.3f) {
		modelMat = glm::rotate(modelMat, 0.3f, glm::vec3(0, 1, 0));
	}
	else {
		modelMat = glm::rotate(modelMat, -legAngle, glm::vec3(0, 1, 0));
	}
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.5));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.5, -1.6));
	if (-legAngle > 0.3f) {
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 2.2));
		modelMat = glm::rotate(modelMat, -legAngle * 1.3f, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -2.2));
	}
	else {
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, 3.0));
		modelMat = glm::rotate(modelMat, -legAngle, glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, 0, -3.0));
	}
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.8));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void display(void)
{
	glm::mat4 worldMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldMat = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1, 0, 0));

	if (isStaticHuman)
	{
		viewMat = glm::lookAt(glm::vec3(8, -2, 7), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
		drawHuman(worldMat);
		glutPostRedisplay();
	}
	else
	{
		viewMat = glm::lookAt(glm::vec3(2, 10, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		viewMat = glm::rotate(viewMat, 1.5708f, glm::vec3(0, 1, 0));
		swimmingAnim(worldMat);
		glutPostRedisplay();
	}

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
	// 시간 값 가져오기
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	// 10ms 마다 한번씩 실행
	if (abs(currTime - prevTime) >= 10)
	{
		float t = abs(currTime - prevTime);
		// 시간 변화량*10초에 한바퀴
		if (leftArmAngle > 1.26f) { leftArmAngle = 0.0f; }
		if (rightArmAngle > 1.26f) { rightArmAngle = 0.0f; }
		leftArmAngle += glm::radians(t * 360.0f / 10000.0f);
		rightArmAngle += glm::radians(t * 360.0f / 10000.0f);
		if (isLegReturn) {
			if (legAngle < -0.5f) {
				isLegReturn = !isLegReturn;
			}
			else {
				legAngle -= glm::radians(t * 3 * 360.0f / 10000.0f);
			}
		}
		else {
			if (legAngle > 0.5f) {
				isLegReturn = !isLegReturn;
			}
			else {
				legAngle += glm::radians(t * 3 * 360.0f / 10000.0f);
			}
		}
		prevTime = currTime;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'h': case 'H':
		isStaticHuman = !isStaticHuman;
		break;
	case 'l': case 'L':
		shadeMode = (++shadeMode % NUM_LIGHT_MODE);
		glUniform1i(shadeModeID, shadeMode);
		glutPostRedisplay();
		break;
	case 't': case 'T':
		isTexture = !isTexture;
		glUniform1i(textureModeID, isTexture);
		glutPostRedisplay();
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Project02_20172979_이효중");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}