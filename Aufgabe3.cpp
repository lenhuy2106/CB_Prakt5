// Ausgangssoftware des 3. Praktikumsversuchs 
// zur Vorlesung Echtzeit-3D-Computergrahpik
// von Prof. Dr. Alfred Nischwitz
// Programm umgesetzt mit der GLTools Library
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif

#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <ImageLoader/ImageLoader.h>
#include <GLFrustum.h>
#include <math.h>
#include <math3d.h>
#include <GL/glut.h>
#include <AntTweakBar.h>

GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
GLBatch geometryBatch;
GLuint shaders;
/// View space light position
float light_pos[4] = { 0.5f, 0.1f, -5.0f, 1.0f };
/// Lichtfarben
float light_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
float light_diffuse[4] = { 1.f, 1.f, 1.f, 1.0f };
float light_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//Materialeigenschaften
float mat_emissive[4] = { 0.0, 0.0, 0.0, 1.0 };
float mat_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
float mat_diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
float mat_specular[4] = { 1.0, 1.0, 1.0, 1.0 };
float specular_power = 10;
// Rotationsgroessen
float rotation[] = { 0, 0, 0, 0 };
//GUI
TwBar *bar;
unsigned int tesselation = 2;
float scaling = 1.0f;
// Definition der Kreiszahl
#define GL_PI 3.1415f
// Zylinder
GLBatch rohr;
GLBatch kreis[2];
// Normals
GLBatch normals[3];
// TEST
GLShaderManager shaderManager;
M3DVector3f* surfaceNormalsCircle[2];
M3DVector3f* surfaceNormalsMantle;
M3DVector3f* vNormals[3];
M3DVector4f* vNormalsColors[3];
void CreateCircle(int id, int xOff, int yOff, int zOff, int radius);
void CreatePipe(int id, int xOff, int yOff, int zOff, int length, int radius);
void CreateCylinder(int xOff, int yOff, int zOff, int length, int radius);
void getAllSurfaceNormals(int numberOfVertices, M3DVector3f* vertices);
M3DVector3f* getTriangleNormal(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2);
void normalizeVector(M3DVector3f *vector);

const std::string TextureMapName = "../Texturen/e43_color_1280_720.bmp";

//Textur Id für die Texture-Map
GLuint TexId[1];



void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'");
	TwAddVarRW(bar, "Model Rotation", TW_TYPE_QUAT4F, &rotation, "");
	TwAddVarRW(bar, "Light Position", TW_TYPE_DIR3F, &light_pos, "group=Light axisx=-x axisy=-y axisz=-z");
	//Hier weitere GUI Variablen anlegen. F�r Farbe z.B. den Typ TW_TYPE_COLOR4F benutzen
}
void CreateGeometry()
{
	CreateCylinder(0, 0, 0, 10, 5);

	//Shader Programme laden. Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	shaders = gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 3,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal",
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoord");

	gltCheckErrors(shaders);
}

void CreateCylinder(int xOff, int yOff, int zOff, int length, int radius){
	CreateCircle(0, 0, 0, length / 2, radius);
	CreateCircle(1, 0, 0, -length / 2, radius);
	CreatePipe(2, 0, 0, 0, length, radius);
}

// Malt Circle mit ID, falls zOff > 0 Front
void CreateCircle(int id, int xOff, int yOff, int zOff, int radius) {
	float size = 2 + 0.5 * scaling;
	int minTriangles = 4;
	float actualTriangles = tesselation * minTriangles;
	int numberOfVertices = actualTriangles * 3;

	// Erzeuge einen weiteren Triangle_Fan um den kreis zu bedecken
	// Manuel ver�ndern... numberOfVertices
	M3DVector3f* kreisVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* kreisColors = new M3DVector4f[numberOfVertices]();

	// Das Zentrum des Triangle_Fans ist im Ursprung
	// DEP: m3dLoadVector3(kreisVertices[0], xOff, yOff, zOff*size);
	m3dLoadVector4(kreisColors[0], 1, 0.5, 0, 1);
	int iPivot = 1 * size;
	int i = 0;

	for (int j = 0; j < actualTriangles; j++) {
		i = j * 3;
		float angle;
		float angle2;
		if (zOff > 0) {
			angle = (2.0f*GL_PI) + j * (2.0f * GL_PI / actualTriangles);
			angle2 = (2.0f*GL_PI) + (j + 1) * (2.0f * GL_PI / actualTriangles);
		}

		else {
			angle = (2.0f*GL_PI) - j * (2.0f * GL_PI / actualTriangles);
			angle2 = (2.0f*GL_PI) - (j + 1) * (2.0f * GL_PI / actualTriangles);
		}

		// Berechne x und y Positionen des naechsten Vertex
		float x = radius*size*cos(angle);
		float y = radius*size*sin(angle);
		float x2 = radius*size*cos(angle2);
		float y2 = radius*size*sin(angle2);

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		// Spezifiziere den naechsten Vertex des Triangle_Fans
		m3dLoadVector3(kreisVertices[i], xOff, yOff, (zOff * size));
		m3dLoadVector3(kreisVertices[i + 1], xOff + x, yOff + y, (zOff * size));
		m3dLoadVector3(kreisVertices[i + 2], xOff + x2, yOff + y2, (zOff * size));
	}

	printf("CIRCLE: actual triangles: %d \n", actualTriangles);
	// calculate normals for every triangles
	// getAllSurfaceNormals(actualTriangles, kreisVertices);
	surfaceNormalsCircle[id] = new M3DVector3f[(int)actualTriangles]();
	for (int i = 0; i < actualTriangles; i++) {
		// M3DVector3f* normal = getTriangleNormal(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		int j = i * 3;
		M3DVector3f u;
		u[0] = kreisVertices[j + 1][0] - kreisVertices[j][0];
		u[1] = kreisVertices[j + 1][1] - kreisVertices[j][1];
		u[2] = kreisVertices[j + 1][2] - kreisVertices[j][2];
		M3DVector3f v;
		v[0] = kreisVertices[j + 2][0] - kreisVertices[j][0];
		v[1] = kreisVertices[j + 2][1] - kreisVertices[j][1];
		v[2] = kreisVertices[j + 2][2] - kreisVertices[j][2];
		float x = (u[1] * v[2]) - (u[2] * v[1]);
		float y = (u[2] * v[0]) - (u[0] * v[2]);
		float z = (u[0] * v[1]) - (u[1] * v[0]);
		//printf("x: %f, y: %f, z: %f \n", x, y, z);
		m3dLoadVector3(surfaceNormalsCircle[id][i], x, y, z);
		// normalize
		m3dNormalizeVector3(surfaceNormalsCircle[id][i]);
	}
	printf("\n");

	// ---------------------------------------------

	kreis[id].Begin(GL_TRIANGLES, numberOfVertices);
	kreis[id].CopyVertexData3f(kreisVertices);
	kreis[id].CopyColorData4f(kreisColors);
	kreis[id].End();

	// Draw normals
	vNormals[id] = new M3DVector3f[numberOfVertices * 2]();
	vNormalsColors[id] = new M3DVector4f[numberOfVertices * 2]();
	for (int i = 0; i < numberOfVertices; i++) {
		int j = i * 2;
		m3dLoadVector3(vNormals[id][j], kreisVertices[i][0], kreisVertices[i][1], kreisVertices[i][2]);
		m3dLoadVector3(vNormals[id][j + 1], surfaceNormalsCircle[id][0][0] + kreisVertices[i][0],
			surfaceNormalsCircle[id][0][1] + kreisVertices[i][1],
			surfaceNormalsCircle[id][0][2] + kreisVertices[i][2]);

		// colors
		m3dLoadVector4(vNormalsColors[id][j], 0.5, 0.5, 0.5, 1);
		m3dLoadVector4(vNormalsColors[id][j + 1], 0.5, 0.5, 0.5, 1);
	}

	// print
	for (int i = 0; i < numberOfVertices / 3; i++) {
		printf("%d: snc_x: %f snc_y: %f snc_z: %f \n", i, surfaceNormalsCircle[id][i][0], surfaceNormalsCircle[id][i][1], surfaceNormalsCircle[id][i][2]);
	}
	printf("\n");

	for (int i = 0; i < numberOfVertices; i++) {
		printf("%d: kv_x: %f kv_y: %f kv_z: %f \n", i, kreisVertices[i][0], kreisVertices[i][1], kreisVertices[i][2]);
	}
	printf("\n");

	normals[id].Begin(GL_LINES, numberOfVertices * 2);
	normals[id].CopyVertexData3f(vNormals[id]);
	// TODO: lol
	//kreis[id].CopyColorData4f(vNormalsColors[id]);
	normals[id].End();
}

/*DEP - calculate normals -- param: GL_TRIANGLE - vertices*/
void getAllSurfaceNormals(int nTriangles, M3DVector3f* vertices) {
	// surfaceNormalsCircle = new M3DVector3f[nTriangles]();
	for (int i = 0; i < nTriangles; i++) {
		// M3DVector3f* normal = getTriangleNormal(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		int j = i * 3;
		M3DVector3f u;
		u[0] = vertices[j + 1][0] - vertices[j][0];
		u[1] = vertices[j + 1][1] - vertices[j][1];
		u[2] = vertices[j + 1][2] - vertices[j][2];
		M3DVector3f v;
		v[0] = vertices[j + 2][0] - vertices[j][0];
		v[1] = vertices[j + 2][1] - vertices[j][1];
		v[2] = vertices[j + 2][2] - vertices[j][2];
		float x = (u[1] * v[2]) - (u[2] * v[1]);
		float y = (u[2] * v[0]) - (u[0] * v[2]);
		float z = (u[0] * v[1]) - (u[1] * v[0]);
		// printf("x: %f, y: %f, z: %f \n", x, y, z);
		// m3dLoadVector3(surfaceNormalsFirstCircle[i], x, y, z);
		// printf("snfc_x: %f snfc_y: %f snfc_z: %f \n", surfaceNormalsFirstCircle[i][0], surfaceNormalsFirstCircle[i][1], surfaceNormalsFirstCircle[i][2]);
	}
}

/*DEPRECATED: heap allocation bugs*/
/*Calculates and normalize normal of a triangle surface*/
M3DVector3f* getTriangleNormal(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2) {
	M3DVector3f* normal = new M3DVector3f[0]();
	M3DVector3f u;
	u[0] = v1[0] - v0[0];
	u[1] = v1[1] - v0[1];
	u[2] = v1[2] - v0[2];
	M3DVector3f v;
	v[0] = v2[0] - v0[0];
	v[1] = v2[1] - v0[1];
	v[2] = v2[2] - v0[2];
	float x = (u[1] * v[2]) - (u[2] * v[1]);
	float y = (u[2] * v[0]) - (u[0] * v[2]);
	float z = (u[0] * v[1]) - (u[1] * v[0]);
	m3dLoadVector3(*normal, x, y, z);
	normalizeVector(normal);
	return normal;
}

/*Normalize vector*/
void normalizeVector(M3DVector3f *vector) {
	float length = sqrt((*vector[0] * *vector[0]) + (*vector[1] * *vector[1]) + (*vector[2] * *vector[2]));
	*vector[0] /= length;
	*vector[1] /= length;
	*vector[2] /= length;
}

// Malt Pipe mit L�nge length.
void CreatePipe(int id, int xOff, int yOff, int zOff, int length, int radius) {
	float size = 2 + 0.5 * scaling;
	int minTriangles = 8;
	int actualTriangles = tesselation * minTriangles;
	int numberOfVertices = actualTriangles * 3;
	// Erzeuge einen Triangle_Strip um den Mantel zu erzeugen
	// Manuel ver�ndern... numberOfVertices
	M3DVector3f* pipeVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* pipeColors = new M3DVector4f[numberOfVertices]();
	M3DVector2f* pipeTexture = new M3DVector2f[numberOfVertices]();

	// create
	int i = 0;
	for (int j = 0; j < actualTriangles / 2; j++) {
		float angle = (2.0f*GL_PI) - j * (2.0f*GL_PI / (actualTriangles / 2.0f));
		float angle2 = (2.0f*GL_PI) - (j + 1) * (2.0f*GL_PI / (actualTriangles / 2.0f));

		float numberOfRectangles = actualTriangles / 2;
		float h = 1280 / numberOfRectangles;


		// Berechne x und y Positionen des naechsten Vertex
		float x = radius*size*cos(angle);
		float y = radius*size*sin(angle);
		float x2 = radius*size*cos(angle2);
		float y2 = radius*size*sin(angle2);

		// Alterniere die Farbe zwischen Rot und Gruen
		m3dLoadVector4(pipeColors[i], 1, 0, 1, 1);
		m3dLoadVector4(pipeColors[i + 1], 1, 1, 0, 1);
		m3dLoadVector4(pipeColors[i + 2], 1, 1, 0, 1);

		m3dLoadVector3(pipeVertices[i + 0], xOff + x, yOff + y, zOff - (length / 2)*size);
		m3dLoadVector3(pipeVertices[i + 1], xOff + x, yOff + y, zOff + (length / 2)*size);
		m3dLoadVector3(pipeVertices[i + 2], xOff + x2, yOff + y2, zOff + (length / 2)*size);

		m3dLoadVector3(pipeVertices[i + 3], xOff + x2, yOff + y2, zOff - (length / 2)*size);
		m3dLoadVector3(pipeVertices[i + 4], xOff + x, yOff + y, zOff - (length / 2)*size);
		m3dLoadVector3(pipeVertices[i + 5], xOff + x2, yOff + y2, zOff + (length / 2)*size);

		m3dLoadVector2(pipeTexture[i + 0], (j)*h, 720);
		m3dLoadVector2(pipeTexture[i + 1], (j)*h, 0);
		m3dLoadVector2(pipeTexture[i + 2], (j+1)*h, 0);

		m3dLoadVector2(pipeTexture[i + 3], (j + 1)*h, 720);
		m3dLoadVector2(pipeTexture[i + 4], (j)*h, 720);
		m3dLoadVector2(pipeTexture[i + 5], (j + 1)*h, 0);

		i += 6;
	}

	for (int test = 0; test < numberOfVertices; test++){
		std::cout << "X = " << pipeTexture[test][0] << ",  Y = " << pipeTexture[test][1] << "\n";
	}

	printf("MANTLE: actual triangles: %d \n", actualTriangles);

	// calculate normals for every triangles
	// getAllSurfaceNormals(actualTriangles, kreisVertices);
	surfaceNormalsMantle = new M3DVector3f[(int)actualTriangles]();
	for (int i = 0; i < actualTriangles; i++) {
		// M3DVector3f* normal = getTriangleNormal(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		int j = i * 3;
		M3DVector3f u;
		u[0] = pipeVertices[j + 1][0] - pipeVertices[j][0];
		u[1] = pipeVertices[j + 1][1] - pipeVertices[j][1];
		u[2] = pipeVertices[j + 1][2] - pipeVertices[j][2];
		M3DVector3f v;
		v[0] = pipeVertices[j + 2][0] - pipeVertices[j][0];
		v[1] = pipeVertices[j + 2][1] - pipeVertices[j][1];
		v[2] = pipeVertices[j + 2][2] - pipeVertices[j][2];
		float x = (u[1] * v[2]) - (u[2] * v[1]);
		float y = (u[2] * v[0]) - (u[0] * v[2]);
		float z = (u[0] * v[1]) - (u[1] * v[0]);
		// printf("x: %f, y: %f, z: %f \n", x, y, z);
		m3dLoadVector3(surfaceNormalsMantle[i], x, y, z);
		// normalize
		m3dNormalizeVector3(surfaceNormalsMantle[i]);
		printf("%d: snm_x: %f snm_y: %f snm_z: %f \n", i, surfaceNormalsMantle[i][0], surfaceNormalsMantle[i][1], surfaceNormalsMantle[i][2]);
	}

	printf("\n");

	// ------------------------------------------------------------

	M3DVector3f* lightNormalsMantle = new M3DVector3f[numberOfVertices * 2]();


	printf("\n");
	int nSurface = numberOfVertices / 3;
	for (int i = 1; i < nSurface; i += 2) {
		int j = i * 3;
		m3dLoadVector3(lightNormalsMantle[j - 1 % numberOfVertices], surfaceNormalsMantle[i][0] + surfaceNormalsMantle[i + 2 % nSurface][0] + surfaceNormalsCircle[0][0][0],
			surfaceNormalsMantle[i][1] + surfaceNormalsMantle[i + 2 % nSurface][1] + surfaceNormalsCircle[0][0][1],
			surfaceNormalsMantle[i][2] + surfaceNormalsMantle[i + 2 % nSurface][2] + surfaceNormalsCircle[0][0][2]);
		m3dLoadVector3(lightNormalsMantle[j + 2 % numberOfVertices], lightNormalsMantle[j - 1 % numberOfVertices][0],
			lightNormalsMantle[j - 1 % numberOfVertices][1],
			lightNormalsMantle[j - 1 % numberOfVertices][2]);
		m3dLoadVector3(lightNormalsMantle[j + 4 % numberOfVertices], lightNormalsMantle[j - 1 % numberOfVertices][0],
			lightNormalsMantle[j - 1 % numberOfVertices][1],
			lightNormalsMantle[j - 1 % numberOfVertices][2]);
		m3dLoadVector3(lightNormalsMantle[j + 0 % numberOfVertices], surfaceNormalsMantle[i][0] + surfaceNormalsMantle[i + 2 % nSurface][0] + surfaceNormalsCircle[1][0][0],
			surfaceNormalsMantle[i][1] + surfaceNormalsMantle[i + 2 % nSurface][1] + surfaceNormalsCircle[1][0][1],
			surfaceNormalsMantle[i][2] + surfaceNormalsMantle[i + 2 % nSurface][2] + surfaceNormalsCircle[1][0][2]);
		m3dLoadVector3(lightNormalsMantle[j + 3 % numberOfVertices], lightNormalsMantle[j + 0 % numberOfVertices][0],
			lightNormalsMantle[j + 0 % numberOfVertices][1],
			lightNormalsMantle[j + 0 % numberOfVertices][2]);
		m3dLoadVector3(lightNormalsMantle[j + 7 % numberOfVertices], lightNormalsMantle[j + 0 % numberOfVertices][0],
			lightNormalsMantle[j + 0 % numberOfVertices][1],
			lightNormalsMantle[j + 0 % numberOfVertices][2]);
	}

	// TODO: fix first/last
	m3dLoadVector3(lightNormalsMantle[0], surfaceNormalsMantle[nSurface - 1][0] + surfaceNormalsMantle[0][0] + surfaceNormalsCircle[1][0][0],
		surfaceNormalsMantle[nSurface - 1][1] + surfaceNormalsMantle[0][1] + surfaceNormalsCircle[1][0][1],
		surfaceNormalsMantle[nSurface - 1][2] + surfaceNormalsMantle[0][2] + surfaceNormalsCircle[1][0][2]);
	m3dLoadVector3(lightNormalsMantle[1], surfaceNormalsMantle[nSurface - 1][0] + surfaceNormalsMantle[0][0] + surfaceNormalsCircle[0][0][0],
		surfaceNormalsMantle[nSurface - 1][1] + surfaceNormalsMantle[0][1] + surfaceNormalsCircle[0][0][1],
		surfaceNormalsMantle[nSurface - 1][2] + surfaceNormalsMantle[0][2] + surfaceNormalsCircle[0][0][2]);

	// print
	for (int i = 0; i < numberOfVertices * 2; i++) {
		printf("%d: lnm_x: %f lnm_y: %f lnm_z: %f \n", i, lightNormalsMantle[i][0], lightNormalsMantle[i][1], lightNormalsMantle[i][2]);
	}


	// Draw light normals
	vNormals[id] = new M3DVector3f[numberOfVertices * 2]();
	vNormalsColors[id] = new M3DVector4f[numberOfVertices * 2]();

	for (int i = 0; i < numberOfVertices; i++) {
		int j = i * 2;
		m3dLoadVector3(vNormals[id][j], pipeVertices[i][0], pipeVertices[i][1], pipeVertices[i][2]);
		m3dLoadVector3(vNormals[id][j + 1], lightNormalsMantle[i][0] + pipeVertices[i][0],
			lightNormalsMantle[i][1] + pipeVertices[i][1],
			lightNormalsMantle[i][2] + pipeVertices[i][2]);

		// colors
		m3dLoadVector4(vNormalsColors[id][j], 0.5, 0.5, 0.5, 1);
		m3dLoadVector4(vNormalsColors[id][j + 1], 0.5, 0.5, 0.5, 1);
	}


	normals[id].Begin(GL_LINES, numberOfVertices * 2);
	normals[id].CopyVertexData3f(vNormals[id]);
	// TODO: lol
	normals[id].End();



	// define all light normals
	rohr.Begin(GL_TRIANGLES, numberOfVertices, 1);
	rohr.CopyVertexData3f(pipeVertices);
	rohr.CopyTexCoordData2f(pipeTexture, 0);
	rohr.CopyColorData4f(pipeColors);
	rohr.CopyNormalDataf(lightNormalsMantle);
	rohr.End();

	M3DVector3f* lightNormalsCircle1 = new M3DVector3f[numberOfVertices]();
	M3DVector3f* lightNormalsCircle2 = new M3DVector3f[numberOfVertices]();
	for (int i = 0; i < numberOfVertices; i++) {
		m3dLoadVector3(lightNormalsCircle1[i], surfaceNormalsCircle[0][0][0], surfaceNormalsCircle[0][0][1], surfaceNormalsCircle[0][0][2]);
		m3dLoadVector3(lightNormalsCircle2[i], surfaceNormalsCircle[1][0][0], surfaceNormalsCircle[1][0][1], surfaceNormalsCircle[1][0][2]);
	}

	kreis[0].Begin(GL_TRIANGLES, numberOfVertices);
	kreis[0].CopyNormalDataf(lightNormalsCircle1);
	kreis[0].End();

	kreis[1].Begin(GL_TRIANGLES, numberOfVertices);
	kreis[1].CopyNormalDataf(lightNormalsCircle2);
	kreis[1].End();

}


// Aufruf draw scene
void RenderScene(void)
{
	// TEST:
	glPolygonMode(GL_BACK, GL_LINE);

	// Clearbefehle f�r den color buffer und den depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	modelViewMatrix.LoadIdentity();
	// initial viewpoint distance
	modelViewMatrix.Translate(0, 0, -50);
	modelViewMatrix.PushMatrix();
	// Speichere den matrix state und f�hre die Rotation durch
	M3DMatrix44f rot;
	m3dQuatToRotationMatrix(rot, rotation);
	modelViewMatrix.MultMatrix(rot);

	glBindTexture(GL_TEXTURE_RECTANGLE, TexId[0]);
	//setze den Shader f�r das Rendern
	glUseProgram(shaders);
	// Matrizen an den Shader �bergeben
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(glGetUniformLocation(shaders, "normalMatrix"), 1, GL_FALSE, transformPipeline.GetNormalMatrix(true));
	// Lichteigenschaften �bergeben
	glUniform4fv(glGetUniformLocation(shaders, "light_pos_vs"), 1, light_pos);
	glUniform4fv(glGetUniformLocation(shaders, "light_ambient"), 1, light_ambient);
	glUniform4fv(glGetUniformLocation(shaders, "light_diffuse"), 1, light_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "light_specular"), 1, light_specular);
	glUniform1f(glGetUniformLocation(shaders, "spec_power"), specular_power);
	//Materialeigenschaften �bergeben
	glUniform4fv(glGetUniformLocation(shaders, "mat_emissive"), 1, mat_emissive);
	glUniform4fv(glGetUniformLocation(shaders, "mat_ambient"), 1, mat_ambient);
	glUniform4fv(glGetUniformLocation(shaders, "mat_diffuse"), 1, mat_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "mat_specular"), 1, mat_specular);

	//Zeichne Model
	//	geometryBatch.Draw();
	kreis[0].Draw();
	kreis[1].Draw();
	rohr.Draw();

	glLineWidth(1);
	normals[0].Draw();
	normals[1].Draw();
	normals[2].Draw();
	modelViewMatrix.PopMatrix();
	// Draw tweak bars
	TwDraw();
	gltCheckErrors(shaders);
	// Vertausche Front- und Backbuffer
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC()
{
	// Schwarzer Hintergrund
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans ben�tzen
	glFrontFace(GL_CCW);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);


	//Erzeuge eindeutige Namen (Nummern) für Texturen und lade sie
	glGenTextures(1, TexId);
	img::ImageLoader imgLoader;
	//Aktive Textur setzen
	glBindTexture(GL_TEXTURE_RECTANGLE, TexId[0]);
	int width, height, type, internalformat;
	bool jpg = false;
	type = GL_RGBA;
	internalformat = GL_BGR;

	unsigned char* data = imgLoader.LoadTextureFromFile(TextureMapName, &width, &height, jpg);
	//Textur hochladen, bei JPEG bildern muss GL_BGR verwendet werden
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, type, width, height,
		0, internalformat, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();
}

void ShutDownRC()
{
	glDeleteProgram(shaders);
	TwTerminate();
}

void ChangeSize(int w, int h)
{
	// Verhindere eine Division durch Null
	if (h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();

	viewFrustum.SetPerspective(45, w / (float)h, 1, 100);

	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();

	// Send the new window size to AntTweakBar
	TwWindowSize(w, h);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("A3 Normalenvektoren");
	atexit(ShutDownRC);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		//Veralteter Treiber etc.
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
		return 1;
	}
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	glutSpecialFunc((GLUTspecialfun)TwEventKeyboardGLUT);

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	TwInit(TW_OPENGL, NULL);
	SetupRC();
	glutMainLoop();
	ShutDownRC();
	return 0;
}