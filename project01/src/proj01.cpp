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

// mat4 -> 4X4 Matrix, vec4 -> 4X4 Vector Matrix
glm::mat4 projectMat;
glm::mat4 viewMat;

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

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

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

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM");

	// visible angle, aspect ratio, near plane, far plane
	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	// 0,0,2 에서 0,0,0을 바라보고, y축(0,1,0)이 윗방향이 됨
	viewMat = glm::lookAt(glm::vec3(10, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawHuman(glm::mat4 humanMat)
{
	glm::mat4 modelMat, pvmMat;

	// Head
	modelMat = glm::scale(humanMat, glm::vec3(1, 1, 1));  //Projection*View*Carmat*Translation*Scaling*vertex
	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 4));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// Body
	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 2));  
	modelMat = glm::scale(modelMat, glm::vec3(1, 2, 3));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Forearm
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
	glm::mat4 modelMat, pvmMat;

	// Head
	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 4));
	modelMat = glm::scale(modelMat, glm::vec3(1, 1, 1));  //Projection*View*Carmat*Translation*Scaling*vertex
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// Body
	modelMat = glm::translate(humanMat, glm::vec3(0, 0, 2));  
	modelMat = glm::scale(modelMat, glm::vec3(1, 2, 3));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Forearm
	modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 2.7));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 0.4));
	modelMat = glm::rotate(modelMat, leftArmAngle*5.0f, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -0.4));
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.5, 1.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Arm
	if (leftArmAngle >= 0 && leftArmAngle < 0.45) {
		modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8*sin(-leftArmAngle * 5.0f), 0, -cos(leftArmAngle * 5.0f)+1.0));
	}
	else if (leftArmAngle >= 0.45 && leftArmAngle < 0.64) {
		modelMat = glm::translate(humanMat, glm::vec3(0, 1.3, 1.5));
		modelMat = glm::translate(modelMat, glm::vec3(0.8 * sin(-leftArmAngle * 5.0f), 0, -cos(leftArmAngle * 5.0f)+2.2));
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
	modelMat = glm::rotate(modelMat, rightArmAngle*5.0f, glm::vec3(0, 1, 0));
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
	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -0.2));  
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.5));
	modelMat = glm::rotate(modelMat, legAngle,glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.5));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// L Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, 0.5, -1.6));  
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 3.0));
	modelMat = glm::rotate(modelMat, legAngle, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -3.0));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.8));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Upper Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.5, -0.2));  
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 1.5));
	modelMat = glm::rotate(modelMat, -legAngle, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -1.5));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.5));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// R Lower Leg
	modelMat = glm::translate(humanMat, glm::vec3(0, -0.5, -1.6));  
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, 3.0));
	modelMat = glm::rotate(modelMat, -legAngle, glm::vec3(0, 1, 0));
	modelMat = glm::translate(modelMat, glm::vec3(0, 0, -3.0));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 1.8));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 회전 Matrix (Identity Matrix, angle, 회전축(1,1,0, 왼쪽아래에서 오른쪽 위로가는 직선)
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

	// 20ms 마다 한번씩 실행
	if (abs(currTime - prevTime) >= 10)
	{
		float t = abs(currTime - prevTime);
		// 시간 변화량*10초에 한바퀴
		if (leftArmAngle > 1.26f) { leftArmAngle = 0.0f; }
		if (rightArmAngle > 1.26f) { rightArmAngle = 0.0f; }
		leftArmAngle += glm::radians(t*360.0f / 10000.0f);
		rightArmAngle += glm::radians(t * 360.0f / 10000.0f);
		if (isLegReturn) {
			if (legAngle < -0.3f) {
				isLegReturn = !isLegReturn;
			}
			else {
				legAngle -= glm::radians(t*3 * 360.0f / 10000.0f);
			}
		}
		else {
			if (legAngle > 0.3f) {
				isLegReturn = !isLegReturn;
			}
			else {
				legAngle += glm::radians(t*3 * 360.0f / 10000.0f);
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
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Project01_20172979_이효중");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
