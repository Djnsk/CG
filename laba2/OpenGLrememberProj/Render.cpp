#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}



class coordinates
{
public:
	double A[3] = { -8,1,0 };
	double B[3] = { -3,5,0 };
	double C[3] = { 0,2,0 };
	double D[3] = { 7,5,0 };
	double E[3] = { 2,0,0 };
	double F[3] = { 4,-6,0 };
	double G[3] = { 0,-5,0 };
	double H[3] = { -1,-1,0 };
	double O[3] = { 0,0,0 };
};
class figure : public coordinates
{
private:
	double angle;
	double Z;
	double A1[3];
	double B1[3];
	double C1[3];
	double D1[3];
	double E1[3];
	double F1[3];
	double G1[3];
	double H1[3];
	double O1[3];
	void base()
	{
		glBindTexture(GL_TEXTURE_2D, texId);
		glBegin(GL_POLYGON);
		glTexCoord3d(-10, -10, Z);
		glVertex3d(-10, -10, Z);
		glTexCoord3d(-10, 10, Z);
		glVertex3d(-10, 10, Z);
		glTexCoord3d(10, 10, Z);
		glVertex3d(10, 10, Z);
		glTexCoord3d(10, -10, Z);
		glVertex3d(10, -10, Z);
		glEnd();
		glBegin(GL_POLYGON);
		glColor3d(0.3, 0.3, 0.3);
		glNormal3d(0, 0, -1);
		glVertex3dv(B);
		glVertex3dv(C);
		glVertex3dv(O);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glVertex3dv(C);
		glVertex3dv(D);
		glVertex3dv(O);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glVertex3dv(D);
		glVertex3dv(E);
		glVertex3dv(O);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glVertex3dv(E);
		glVertex3dv(F);
		glVertex3dv(O);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glVertex3dv(G);
		glVertex3dv(H);
		glVertex3dv(O);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glVertex3dv(H);
		glVertex3dv(A);
		glVertex3dv(O);
		glEnd();
	}
	void base2()
	{
		glBegin(GL_POLYGON);
		glColor3d(0.3, 0.3, 0.3);
		glNormal3d(0, 0, 1);
		glVertex3dv(B1);
		glVertex3dv(C1);
		glVertex3dv(O1);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glVertex3dv(C1);
		glVertex3dv(D1);
		glVertex3dv(O1);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glVertex3dv(D1);
		glVertex3dv(E1);
		glVertex3dv(O1);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glVertex3dv(E1);
		glVertex3dv(F1);
		glVertex3dv(O1);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glVertex3dv(G1);
		glVertex3dv(H1);
		glVertex3dv(O1);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glVertex3dv(H1);
		glVertex3dv(A1);
		glVertex3dv(O1);
		glEnd();
	}
	double ang_calc(double x, double y)
	{
		double radius = sqrt(pow(x, 2) + pow(y, 2));
		double angle;
		if (x >= 0 && y >= 0)
			angle = (asin(abs(y) / radius) * 180) / PI;
		if (x < 0 && y >= 0)
			angle = 180 - (asin(abs(y) / radius) * 180) / PI;
		if (x < 0 && y < 0)
			angle = 180 + (asin(abs(y) / radius) * 180) / PI;
		if (x >= 0 && y < 0)
			angle = 360 - (asin(abs(y) / radius) * 180) / PI;
		return angle;
	}
	void base_calc()
	{
		double radius;
		double angle;

		radius = sqrt(pow(A[0], 2) + pow(A[1], 2));
		angle = ((this->angle + ang_calc(A[0], A[1])) * PI) / 180;
		A1[0] = radius * cos(angle); A1[1] = radius * sin(angle); A1[2] = Z;

		radius = sqrt(pow(B[0], 2) + pow(B[1], 2));
		angle = ((this->angle + ang_calc(B[0], B[1])) * PI) / 180;
		B1[0] = radius * cos(angle); B1[1] = radius * sin(angle); B1[2] = Z;

		radius = sqrt(pow(C[0], 2) + pow(C[1], 2));
		angle = ((this->angle + ang_calc(C[0], C[1])) * PI) / 180;
		C1[0] = radius * cos(angle); C1[1] = radius * sin(angle); C1[2] = Z;

		radius = sqrt(pow(D[0], 2) + pow(D[1], 2));
		angle = ((this->angle + ang_calc(D[0], D[1])) * PI) / 180;
		D1[0] = radius * cos(angle); D1[1] = radius * sin(angle); D1[2] = Z;

		radius = sqrt(pow(E[0], 2) + pow(E[1], 2));
		angle = ((this->angle + ang_calc(E[0], E[1])) * PI) / 180;
		E1[0] = radius * cos(angle); E1[1] = radius * sin(angle); E1[2] = Z;

		radius = sqrt(pow(F[0], 2) + pow(F[1], 2));
		angle = ((this->angle + ang_calc(F[0], F[1])) * PI) / 180;
		F1[0] = radius * cos(angle); F1[1] = radius * sin(angle); F1[2] = Z;

		radius = sqrt(pow(G[0], 2) + pow(G[1], 2));
		angle = ((this->angle + ang_calc(G[0], G[1])) * PI) / 180;
		G1[0] = radius * cos(angle); G1[1] = radius * sin(angle); G1[2] = Z;

		radius = sqrt(pow(H[0], 2) + pow(H[1], 2));
		angle = ((this->angle + ang_calc(H[0], H[1])) * PI) / 180;
		H1[0] = radius * cos(angle); H1[1] = radius * sin(angle); H1[2] = Z;

		O1[0] = 0; O1[1] = 0; O1[2] = Z;

	}
	void tube(double(&A)[3], double(&B)[3], double(&A1)[3], double(&B1)[3])
	{
		double x0 = (A[0] + B[0]) / 2;
		double y0 = (A[1] + B[1]) / 2;
		double x01 = (A1[0] + B1[0]) / 2;
		double y01 = (A1[1] + B1[1]) / 2;

		double radius = (sqrt(pow((A[0] - B[0]), 2) + pow((A[1] - B[1]), 2))) / 2;
		double radius1 = (sqrt(pow((A1[0] - B1[0]), 2) + pow((A1[1] - B1[1]), 2))) / 2;

		double angle_B = ang_calc(B[0] - x0, B[1] - y0);
		double angle_A = ang_calc(A[0] - x0, A[1] - y0);

		double angle;
		double step = 0.01;

		double T0[] = { A[0], A[1], A[2] };
		double T01[] = { A1[0], A1[1], A1[2] };

		double norm_x;
		double norm_y;
		double norm_z;

		for (double i = angle_B; i < angle_A; i += step)
		{
			angle = (i)*PI / 180;
			double T[] = { radius * cos(angle + step) + x0, radius * sin(angle + step) + y0, 0 };
			angle = (this->angle + i) * PI / 180;
			double T1[] = { radius * cos(angle + step) + x01, radius * sin(angle + step) + y01, Z };

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.3, 0.6);
			norm_x = T01[1] * T1[2] - T1[1] * T01[2]; norm_x /= abs(norm_x);
			norm_y = T1[0] * T01[2] - T01[0] * T1[2]; norm_y /= abs(norm_y);
			norm_z = T01[0] * T1[1] - T1[0] * T01[1]; norm_z /= abs(norm_z);
			glNormal3d(-norm_x, -norm_y, -norm_z);
			glVertex3dv(T0);
			glVertex3dv(T01);
			glVertex3dv(T1);
			glVertex3dv(T);
			glEnd();

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.3, 0.3);
			glNormal3d(0, 0, -1);
			glVertex3dv(T0);
			glVertex3dv(T);
			glVertex3dv(O);
			glEnd();

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.3, 0.3);
			glNormal3d(0, 0, 1);
			glVertex3dv(T01);
			glVertex3dv(T1);
			glVertex3dv(O1);
			glEnd();
			T0[0] = T[0]; T0[1] = T[1];
			T01[0] = T1[0]; T01[1] = T1[1];
		}
	}
	void tube2(double(&A)[3], double(&B)[3], double(&A1)[3], double(&B1)[3])
	{
		double x0 = (A[0] + B[0]) / 2;
		double y0 = (A[1] + B[1]) / 2;
		double x01 = (A1[0] + B1[0]) / 2;
		double y01 = (A1[1] + B1[1]) / 2;

		double radius = (sqrt(pow((A[0] - B[0]), 2) + pow((A[1] - B[1]), 2))) / 2;
		double radius1 = (sqrt(pow((A1[0] - B1[0]), 2) + pow((A1[1] - B1[1]), 2))) / 2;

		double angle_B = ang_calc(B[0] - x0, B[1] - y0);
		double angle_A = 360 - ang_calc(A[0] - x0, A[1] - y0);

		double angle;
		double step = 0.01;

		double T0[] = { A[0], A[1], A[2] };
		double T01[] = { A1[0], A1[1], A1[2] };

		double norm_x;
		double norm_y;
		double norm_z;

		for (double i = -angle_A; i < angle_B; i += step)
		{
			angle = (i)*PI / 180;
			double T[] = { radius * cos(angle + step) + x0, radius * sin(angle + step) + y0, 0 };
			angle = (this->angle + i) * PI / 180;
			double T1[] = { radius * cos(angle + step) + x01, radius * sin(angle + step) + y01, Z };

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.6, 0.3);
			norm_x = T01[1] * T1[2] - T1[1] * T01[2]; norm_x /= abs(norm_x);
			norm_y = T1[0] * T01[2] - T01[0] * T1[2]; norm_y /= abs(norm_y);
			norm_z = T01[0] * T1[1] - T1[0] * T01[1]; norm_z /= abs(norm_z);
			glNormal3d(norm_x, norm_y, norm_z);
			glVertex3dv(T0);
			glVertex3dv(T01);
			glVertex3dv(T1);
			glVertex3dv(T);
			glEnd();

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.3, 0.3);
			glNormal3d(0, 0, -1);
			glVertex3dv(T0);
			glVertex3dv(T);
			glVertex3dv(O);
			glEnd();

			glBegin(GL_POLYGON);
			glColor3d(0.3, 0.3, 0.3);
			glNormal3d(0, 0, 1);
			glVertex3dv(T01);
			glVertex3dv(T1);
			glVertex3dv(O1);
			glEnd();
			T0[0] = T[0]; T0[1] = T[1];
			T01[0] = T1[0]; T01[1] = T1[1];
		}
	}
	void wall()
	{
		double radius;
		double angle = (this->angle * PI) / 180;
		double tmp;

		double norm_x;
		double norm_y;
		double norm_z;

		glBegin(GL_POLYGON);
		glColor3d(0.6, 0.3, 0.3);
		norm_x = B1[1] * C1[2] - C1[1] * B1[2]; norm_x /= abs(norm_x);
		norm_y = C1[0] * B1[2] - B1[0] * C1[2]; norm_y /= abs(norm_y);
		norm_z = B1[0] * C1[1] - C1[0] * B1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(B);
		glVertex3dv(B1);
		glVertex3dv(C1);
		glVertex3dv(C);
		glEnd();
		glBegin(GL_POLYGON);
		norm_x = C1[1] * D1[2] - D1[1] * C1[2]; norm_x /= abs(norm_x);
		norm_y = D1[0] * C1[2] - C1[0] * D1[2]; norm_y /= abs(norm_y);
		norm_z = C1[0] * D1[1] - D1[0] * C1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(C);
		glVertex3dv(C1);
		glVertex3dv(D1);
		glVertex3dv(D);
		glEnd();
		glBegin(GL_POLYGON);
		norm_x = D1[1] * E1[2] - E1[1] * D1[2]; norm_x /= abs(norm_x);
		norm_y = E1[0] * D1[2] - D1[0] * E1[2]; norm_y /= abs(norm_y);
		norm_z = D1[0] * E1[1] - E1[0] * D1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(D);
		glVertex3dv(D1);
		glVertex3dv(E1);
		glVertex3dv(E);
		glEnd();
		glBegin(GL_POLYGON);
		norm_x = E1[1] * F1[2] - F1[1] * E1[2]; norm_x /= abs(norm_x);
		norm_y = F1[0] * E1[2] - E1[0] * F1[2]; norm_y /= abs(norm_y);
		norm_z = E1[0] * F1[1] - F1[0] * E1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(E);
		glVertex3dv(E1);
		glVertex3dv(F1);
		glVertex3dv(F);
		glEnd();
		glBegin(GL_POLYGON);
		norm_x = G1[1] * H1[2] - H1[1] * G1[2]; norm_x /= abs(norm_x);
		norm_y = H1[0] * G1[2] - G1[0] * H1[2]; norm_y /= abs(norm_y);
		norm_z = G1[0] * H1[1] - H1[0] * G1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(G);
		glVertex3dv(G1);
		glVertex3dv(H1);
		glVertex3dv(H);
		glEnd();
		glBegin(GL_POLYGON);
		norm_x = H1[1] * A1[2] - A1[1] * H1[2]; norm_x /= abs(norm_x);
		norm_y = A1[0] * H1[2] - H1[0] * A1[2]; norm_y /= abs(norm_y);
		norm_z = H1[0] * A1[1] - A1[0] * H1[1]; norm_z /= abs(norm_z);
		glNormal3d(norm_x, norm_y, norm_z);
		glVertex3dv(H);
		glVertex3dv(H1);
		glVertex3dv(A1);
		glVertex3dv(A);
		glEnd();
		tube(A, B, A1, B1);
		tube2(F, G, F1, G1);
	}
	void draw()
	{
		base_calc();

		base();
		base2();
		wall();
	}
public:
	figure(double f = 0, double Z = 5)
	{
		this->angle = f;
		this->Z = Z;
		draw();
	}
};

void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	figure(15, 5);

	//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glVertex3d(-4, -4, 0);
	glVertex3d(4, -4, 0);
	glVertex3d(0, 0, 4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glVertex3d(4, -4, 0);
	glVertex3d(4, 4, 0);
	glVertex3d(0, 0, 4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glVertex3d(4, 4, 0);
	glVertex3d(-4, 4, 0);
	glVertex3d(0, 0, 4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glVertex3d(-4, 4, 0);
	glVertex3d(-4, -4, 0);
	glVertex3d(0, 0, 4);
	glEnd();*/
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}