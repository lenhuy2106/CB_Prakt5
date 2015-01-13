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
float light_pos[4] = {0.5f,0.1f,-5.0f,1.0f} ;
/// Lichtfarben
float light_ambient[4] = {0.0, 0.0, 0.0, 1.0}; 
float light_diffuse[4] = {0.9f,0.0f,0.5f,1.0f} ;
float light_specular[4] = {1.0f,1.0f,1.0f,1.0f} ;
//Materialeigenschaften
float mat_emissive[4] = {0.0, 0.0, 0.0, 1.0};
float mat_ambient[4]  = {0.0, 0.0, 0.0, 1.0}; 
float mat_diffuse[4]    = {1.0, 1.0, 1.0, 1.0};
float mat_specular[4]   = {1.0, 1.0, 1.0, 1.0};
float specular_power = 10 ;
// Rotationsgroessen
float rotation[] = {0, 0,0,0};
//GUI
TwBar *bar;
unsigned int tesselation = 1;
float scaling = 1.0f;
// Definition der Kreiszahl
#define GL_PI 3.1415f
// Zylinder
GLBatch rohr;
GLBatch kreis[2];
// TEST
GLShaderManager shaderManager;

void CreateCircle(int id, int xOff, int yOff, int zOff, int radius);
void CreatePipe(int xOff, int yOff, int zOff, int length, int radius);
void CreateCylinder(int xOff, int yOff, int zOff, int length, int radius);

void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'"); 
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F,&rotation,"");
	TwAddVarRW(bar,"Light Position", TW_TYPE_DIR3F,&light_pos,"group=Light axisx=-x axisy=-y axisz=-z");
	//Hier weitere GUI Variablen anlegen. Für Farbe z.B. den Typ TW_TYPE_COLOR4F benutzen
}
void CreateGeometry()
{
	/*
	//Dreieck
	geometryBatch.Begin(GL_TRIANGLES,3);
	
	geometryBatch.Normal3f(0,0,1);
	geometryBatch.Vertex3f(-1,-1,0);
	
	geometryBatch.Normal3f(0,0,1);
	geometryBatch.Vertex3f(0,1,0);
	
	geometryBatch.Normal3f(0,0,1);
	geometryBatch.Vertex3f(1,-1,0);
	
	geometryBatch.End();
	*/

	CreateCylinder(0, 0, 0, 10, 5);
	
	//Shader Programme laden. Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	shaders = gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_NORMAL, "vNormal");
	
	gltCheckErrors(shaders);
}

void CreateCylinder(int xOff, int yOff, int zOff, int length, int radius){
	CreateCircle(0, 0, 0, length / 2, radius);
	CreateCircle(1, 0, 0, -length / 2, radius);
	CreatePipe(0, 0, 0, length, radius);
}

// Malt Circle mit ID, falls zOff > 0 Front
void CreateCircle(int id, int xOff, int yOff, int zOff, int radius) {

	float size = 2 + 0.5 * scaling;

	int minTriangles = 4;
	float actualTriangles = tesselation * minTriangles;
	int numberOfVertices = actualTriangles * 3;

	// Erzeuge einen weiteren Triangle_Fan um den kreis zu bedecken
	// Manuel verändern... numberOfVertices
	M3DVector3f* kreisVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* kreisColors = new M3DVector4f[numberOfVertices]();

	// Das Zentrum des Triangle_Fans ist im Ursprung
	// DEP: m3dLoadVector3(kreisVertices[0], xOff, yOff, zOff*size);
	m3dLoadVector4(kreisColors[0], 1, 0, 0, 1);
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

		// Alterniere die Farbe zwischen Rot und Gruen
		/* DEP
		if ((iPivot % 2) == 0) {
			m3dLoadVector4(kreisColors[i], 1, 0.8, 0.2, 1);

		} else {
			m3dLoadVector4(kreisColors[i], 0, 0.8, 0, 1);
		}
		*/

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		// Spezifiziere den naechsten Vertex des Triangle_Fans
		m3dLoadVector3(kreisVertices[i], xOff, yOff, (zOff * size));
		m3dLoadVector3(kreisVertices[i + 1], xOff + x, yOff + y, (zOff * size));
		m3dLoadVector3(kreisVertices[i + 2], xOff + x2, yOff + y2, (zOff * size));
	}

	// calculate normals for every vertices
	// M3DVector3f* vNorms = calculateNormals(numberOfVertices, kreisVertices);
	// TODO: calculate

	kreis[id].Begin(GL_TRIANGLES, numberOfVertices);
	kreis[id].CopyVertexData3f(kreisVertices);
	kreis[id].CopyColorData4f(kreisColors);
	// kreis[id].CopyNormalDataf(vNorms);
	kreis[id].End();
}


// calculate normals
// param: GL_TRIANGLE - vertices
M3DVector3f* getAllSurfaceNormals(int numberOfVertices, M3DVector3f* vertices) {
	
	M3DVector3f* vNormals = new M3DVector3f[numberOfVertices/3]();

	/*
	M3DVector3f u;
	u[0] = vertices[i + 1][0] - vertices[i][0];
	u[1] = vertices[i + 1][1] - vertices[i][1];
	u[2] = vertices[i + 1][2] - vertices[i][2];

	M3DVector3f v;
	v[0] = vertices[i + 2][0] - vertices[i][0];
	v[1] = vertices[i + 2][1] - vertices[i][1];
	v[2] = vertices[i + 2][2] - vertices[i][2];

	vNormals[i][0] = (u[1] * v[2]) - (u[2] * v[1]);
	vNormals[i][1] = (u[2] * v[0]) - (u[0] * v[2]);
	vNormals[i][2] = (u[0] * v[1]) - (u[1] * v[0]);
	*/

	for (int i = 0; i < numberOfVertices / 3; i++) {
		M3DVector3f* normal = getTriangleNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
		m3dLoadVector3(vNormals[i], *normal[0], *normal[1], *normal[2]);
	}

	return vNormals;
}

/*Calculates and normalize normal of a triangle surface*/
M3DVector3f* getTriangleNormal(M3DVector3f v0, M3DVector3f v1, M3DVector3f v2) {
	M3DVector3f* normal;
	M3DVector3f u;
	u[0] = v1[0] - v0[0];
	u[1] = v1[1] - v0[1];
	u[2] = v1[2] - v0[2];
	M3DVector3f v;
	v[0] = v2[0] - v0[0];
	v[1] = v2[1] - v0[1];
	v[2] = v2[2] - v0[2];
	*normal[0] = (u[1] * v[2]) - (u[2] * v[1]);
	*normal[1] = (u[2] * v[0]) - (u[0] * v[2]);
	*normal[2] = (u[0] * v[1]) - (u[1] * v[0]);
	return normal;
}

/*Normalize vector*/
void normalizeVector(M3DVector3f *vector) {
	float length = sqrt((*vector[0] * *vector[0]) + (*vector[1] * *vector[1]) + (*vector[2] * *vector[2]));
	*vector[0] /= length;
	*vector[1] /= length;
	*vector[2] /= length;
}

// Malt Pipe mit Länge length.
void CreatePipe(int xOff, int yOff, int zOff, int length, int radius) {

	float size = 2 + 0.5 * scaling;

	int minTriangles = 8;
	int actualTriangles = tesselation * minTriangles;
	int numberOfVertices = actualTriangles + 2;
	// Erzeuge einen Triangle_Strip um den Mantel zu erzeugen
	// Manuel verändern... numberOfVertices
	M3DVector3f* pipeVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* pipeColors = new M3DVector4f[numberOfVertices]();

	int i = 0;
	for (int j = 0; j <= (actualTriangles) / 2; j++)
	{
		float angle = (2.0f*GL_PI) - j * (2.0f*GL_PI / (actualTriangles / 2.0f));

		// Berechne x und y Positionen des naechsten Vertex
		float x = radius*size*cos(angle);
		float y = radius*size*sin(angle);

		// Alterniere die Farbe zwischen Rot und Gruen
		m3dLoadVector4(pipeColors[i], 1, 0, 1, 1);
		m3dLoadVector4(pipeColors[i + 1], 1, 1, 0, 1);


		m3dLoadVector3(pipeVertices[i], xOff + x, yOff + y, zOff - (length / 2)*size);
		m3dLoadVector3(pipeVertices[i + 1], xOff + x, yOff + y, zOff + (length / 2)*size);
		i += 2;
	}

	rohr.Begin(GL_TRIANGLE_STRIP, numberOfVertices);
	rohr.CopyVertexData3f(pipeVertices);
	rohr.CopyColorData4f(pipeColors);
	rohr.End();
}


// Aufruf draw scene
void RenderScene(void)
{
	// Clearbefehle für den color buffer und den depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	modelViewMatrix.LoadIdentity();
	// initial viewpoint distance
	modelViewMatrix.Translate(0,0,-50);
	// Speichere den matrix state und führe die Rotation durch
	modelViewMatrix.PushMatrix();

	M3DMatrix44f rot;
	m3dQuatToRotationMatrix(rot,rotation);
	modelViewMatrix.MultMatrix(rot);
	

	//setze den Shader für das Rendern
	glUseProgram(shaders);
	// Matrizen an den Shader übergeben
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvMatrix"),  1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(glGetUniformLocation(shaders, "normalMatrix"),  1, GL_FALSE, transformPipeline.GetNormalMatrix(true));
	// Lichteigenschaften übergeben
	glUniform4fv(glGetUniformLocation(shaders, "light_pos_vs"),1,light_pos);
	glUniform4fv(glGetUniformLocation(shaders, "light_ambient"),1,light_ambient);
	glUniform4fv(glGetUniformLocation(shaders, "light_diffuse"),1,light_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "light_specular"),1,light_specular);
	glUniform1f(glGetUniformLocation(shaders, "spec_power"),specular_power);
	//Materialeigenschaften übergeben
	glUniform4fv(glGetUniformLocation(shaders, "mat_emissive"),1,mat_emissive);
	glUniform4fv(glGetUniformLocation(shaders, "mat_ambient"),1,mat_ambient);
	glUniform4fv(glGetUniformLocation(shaders, "mat_diffuse"),1,mat_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "mat_specular"),1,mat_specular);

	//TEST
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
	//---------------------------------

	//Zeichne Model
	geometryBatch.Draw();
	rohr.Draw();
	kreis[0].Draw();
	kreis[1].Draw();

	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
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
	glClearColor( 0.12f,0.35f,0.674f,0.0f ) ;

	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
	glFrontFace(GL_CCW);

	//TEST
	//initialisiert die standard shader
	shaderManager.InitializeStockShaders();
	//-------------------------------------

	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);
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
	if(h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();

	viewFrustum.SetPerspective(45,w/(float)h,1,100);
	
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
	glutInitWindowSize(800,600);
	glutCreateWindow("A3 Normalenvektoren");
	atexit(ShutDownRC);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		//Veralteter Treiber etc.
		std::cerr <<"Error: "<< glewGetErrorString(err) << "\n";
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
