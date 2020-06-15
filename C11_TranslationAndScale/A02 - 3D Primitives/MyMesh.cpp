#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//Code modified from circle generation

	//Store points for center, the last point generated, and the peak
	vector3 center(0, -a_fHeight / 2, 0);
	vector3 peak(0, a_fHeight / 2, 0);
	vector3 lastPoint(a_fRadius, center.y, 0);

	float theta = 0;
	//Loop through the number of sides and generating a point based on the angle, and attaching them to the last point, the peak, and the center
	for (int i = 0; i < a_nSubdivisions; i++) {
		theta += ((2 * std::_Pi) / a_nSubdivisions);
		//2 / a_nSides radians for each slice
		float x = cos(theta) * a_fRadius;
		float z = sin(theta) * a_fRadius;
		vector3 newPoint(x, center.y, z);
		//Add tri to mesh
		AddTri(peak, newPoint, lastPoint);
		AddTri(center, lastPoint, newPoint);

		//Update the last point
		lastPoint = newPoint;
	}

	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//Code modified from circle generation

	//Store points for center and the last point generated, for both the top face and bottom face of the cylinder
	vector3 centerTop(0, a_fHeight / 2.0f, 0);
	vector3 lastPointTop(a_fRadius, a_fHeight / 2.0f, 0);
	vector3 centerBottom(0, -a_fHeight / 2.0f, 0);
	vector3 lastPointBottom(a_fRadius, -a_fHeight / 2.0f, 0);

	float theta = 0;
	//Loop through the number of sides and generating a point based on the angle, and attaching them to the last point, the peak, and the center
	for (int i = 0; i < a_nSubdivisions; i++) {
		theta += ((2 * std::_Pi) / a_nSubdivisions);
		float x = cos(theta) * a_fRadius;
		float z = sin(theta) * a_fRadius;
		vector3 newPointTop(x, centerTop.y, z);
		vector3 newPointBottom(x, centerBottom.y, z);

		//Add top and bottom tris to mesh
		AddTri(centerTop, newPointTop, lastPointTop);
		AddTri(centerBottom, lastPointBottom, newPointBottom);
		//Add side face
		AddQuad(lastPointTop, newPointTop, lastPointBottom, newPointBottom);

		//Update the last points
		lastPointTop = newPointTop;
		lastPointBottom = newPointBottom;
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//Code modified from cylinder generation

	//Store points for the last point generated for the top, bottom, inside, and outside of the tube
	vector3 lastPointTopOut(a_fOuterRadius, a_fHeight / 2.0f, 0);
	vector3 lastPointTopIn(a_fInnerRadius, a_fHeight / 2.0f, 0);
	vector3 lastPointBottomOut(a_fOuterRadius, -a_fHeight / 2.0f, 0);
	vector3 lastPointBottomIn(a_fInnerRadius, -a_fHeight / 2.0f, 0);

	float theta = 0;
	//Loop through the number of sides and generating a point based on the angle, and attaching them to the last point, the peak, and the center
	for (int i = 0; i < a_nSubdivisions; i++) {
		theta += ((2.0f * std::_Pi) / a_nSubdivisions);
		
		float outX = cos(theta) * a_fOuterRadius;
		float outZ = sin(theta) * a_fOuterRadius;
		vector3 newPointTopOut(outX, a_fHeight / 2.0f, outZ);
		vector3 newPointBottomOut(outX, -a_fHeight / 2.0f, outZ);

		float inX = cos(theta) * a_fInnerRadius;
		float inZ = sin(theta) * a_fInnerRadius;
		vector3 newPointTopIn(inX, a_fHeight / 2.0f, inZ);
		vector3 newPointBottomIn(inX, -a_fHeight / 2.0f, inZ);

		//Add outer face
		AddQuad(lastPointTopOut, newPointTopOut, lastPointBottomOut, newPointBottomOut);
		//Add inner face
		AddQuad(lastPointBottomIn, newPointBottomIn, lastPointTopIn, newPointTopIn);
		//Add top face
		AddQuad(lastPointTopIn, newPointTopIn, lastPointTopOut, newPointTopOut);
		//Add bottom face
		AddQuad(lastPointBottomOut, newPointBottomOut, lastPointBottomIn, newPointBottomIn);

		//Update last points
		lastPointTopOut = newPointTopOut;
		lastPointTopIn = newPointTopIn;
		lastPointBottomOut = newPointBottomOut;
		lastPointBottomIn = newPointBottomIn;
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	//Store the last point of the main segment of the torus (inside)
	vector3 lastPointA(a_fInnerRadius, 0, 0);
	//Store the last ring of points generated
	std::vector<vector3> lastRing;
	//Generate initial ring of points on the inside of the torus
	float theta = 0;
	for (int i = 0; i < a_nSubdivisionsA; i++) {
		theta += ((2.0f * std::_Pi) / a_nSubdivisionsA);
		float x = cos(theta) * a_fInnerRadius;
		float z = sin(theta) * a_fInnerRadius;
		lastRing.push_back(vector3(x, 0, z));
	}

	//Keep track of a "theta" between each layer to effectively "modulate" the height and radius of the points generated.
	//Offset by -90 degrees since this is in a different spot.
	float thetaA = -(std::_Pi / 2.0f);
	float thickness = a_fOuterRadius - a_fInnerRadius;
	//Loop through for each level of the second subdivisions (cross-section subdivisions) and generate rings
	for (int i = 0; i < a_nSubdivisionsB; i++) {
		thetaA += ((2.0f * std::_Pi) / a_nSubdivisionsB);
		float radius = (a_fInnerRadius + thickness) + (thickness * sin(thetaA));
		float y = (a_fOuterRadius - a_fInnerRadius) * cos(thetaA);

		std::vector<vector3> newRing;
		vector3 lastPoint(radius, y, 0);
		//Loop through main subdivisions and generate the new points for this layer
		theta = 0;
		for (int j = 0; j < a_nSubdivisionsA; j++) {
			theta += ((2.0f * std::_Pi) / a_nSubdivisionsA);
			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			vector3 newPoint(x, y, z);

			//Create a face
			AddQuad(lastRing.at((j - 1 < 0) ? (a_nSubdivisionsA - 1) : (j - 1)), lastRing.at(j), lastPoint, newPoint);
			lastPoint = newPoint;
			newRing.push_back(newPoint);
		}
		lastRing = newRing;

	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions < 4) {
		a_nSubdivisions = 4;
	}

	//a_nSubdivisions = 10;

	Release();
	Init();

	//Keep track of top of sphere
	vector3 top(0, a_fRadius, 0);
	//Keep track of the last ring of points generated
	std::vector<vector3> lastRing;
	//Similar to in torus, keep a "theta" to "modulate" radius and height of rings
	float thetaA = 0;

	//Begin one later and end one sooner since top and bottom don't require full rings
	for (int i = 1; i < a_nSubdivisions; i++) {
		//thetaA only needs half of a full circle since generation creates an entire ring, covering twice the cross-section
		thetaA += (((2.0f * std::_Pi) / a_nSubdivisions) / 2.0);
		float radius = a_fRadius * sin(thetaA);
		float y = a_fRadius * cos(thetaA);

		std::vector<vector3> newRing;
		vector3 lastPoint(radius, y, 0);

		float theta = 0;
		for (int j = 0; j < a_nSubdivisions; j++) {
			theta += ((2.0f * std::_Pi) / a_nSubdivisions);

			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			vector3 newPoint(x, y, z);
			if (i == 1) {
				//If i == 1 (first ring) make tris from new ring and top of sphere
				AddTri(top, newPoint, lastPoint);
			}
			else {
				//Otherwise, connect to last ring
				AddQuad(lastRing.at((j - 1 < 0) ? (a_nSubdivisions - 1) : (j - 1)), lastRing.at(j), lastPoint, newPoint);
			}
			lastPoint = newPoint;
			newRing.push_back(newPoint);
		}
		lastRing = newRing;
	}
	//Connect last ring to bottom of sphere
	for (int i = 0; i < a_nSubdivisions; i++) {
		AddTri(vector3(0, -a_fRadius, 0), lastRing.at((i - 1 < 0) ? (a_nSubdivisions - 1) : (i - 1)), lastRing.at(i));
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}