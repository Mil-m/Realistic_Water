#include "include/GL/glew.h"
#include "stdlib.h"
#include "include/GL/glut.h"
#include "include/GL/glext.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <gl\gl.h>							// Заголовочные файлы для библиотеки OpenGL32
#include <gl\glu.h>							// Заголовочные файлы для библиотеки GLu32
#include "include/GL/glaux.h"				// Заголовочные файлы для библиотеки GLaux
#pragma warning(disable: 4996)
LARGE_INTEGER time_start1, time_end1, frequent1;
LARGE_INTEGER time_start2, time_end2, frequent2;

const double PI = 3.14159265;
double Width, Height;
double x_angle, y_angle, z_angle;
double scale = 0.03;
//-----------------------------------arrays
int count_cnst = 40;
int lvl_cnst = 40;
int count = 40;
int lvl = 40;
int count_land = 40;
int lvl_land = 40;
int div_water = 40;
int div_land = 40;
int the_level = 0;
struct elem **water;
struct elem **landscape;
struct elem **environment;
BYTE *map_1;
BYTE *map_2;
BYTE *map_3;
BYTE *map_4;
BYTE *map_5;
int box_height = 1;
int object = 0;
int object_x = 30;
int object_y = 18;
//-----------------------------------wave
int key_for_wave = 0;
int key_for_big_wave = 0;
int dynamic = 1;
int dynamic_wave = 0;
double max_water = 0;
//-----------------------------------light
int use_light = 0;
//-----------------------------------texture
int constant_tex = 0;
int use_text = 0;

struct elem {
    double elemx;
	double elemy;
    double elemz;
};

struct elem vector(struct elem c1, struct elem c2) {
	struct elem v;
	v.elemx = c2.elemx - c1.elemx;
	v.elemy = c2.elemy - c1.elemy;
	v.elemz = c2.elemz - c1.elemz;
	return (v);
}

struct elem normal(struct elem a, struct elem b) {
	struct elem n;
	n.elemx = a.elemz*b.elemy - a.elemy*b.elemz;
	n.elemy = a.elemx*b.elemz - a.elemz*b.elemx;
	n.elemz = -(a.elemy*b.elemx - a.elemx*b.elemy);
	return (n);
}

int min(int a, int b) {
	if (a <= b) {
		return a;
	} else {
		return b;
	}
}

void free_arr(struct elem **a, int lvl) {
	for(int i = 0; i <= lvl; i ++) {
		if (a[i] != NULL) {
			free(a[i]);
		}
    }
    free(a);
}

int Height_map(BYTE *map, int X, int Y, int MAP_SIZE) {
	int x = X % MAP_SIZE;
    int y = Y % MAP_SIZE;
    if(!map) {
		return 0;
    }
    return map[x + (y * MAP_SIZE)];
}

void Load_Height_map(BYTE *map, char *s, int nSize) {
	FILE *pFile = NULL;
	pFile = fopen(s, "rb");
	if (pFile == NULL) {
		printf("%s", "cant't open raw file");
		return;
	}
	// Каждый раз читаем по одному байту, размер = ширина * высота
	fread(map, sizeof(BYTE), nSize, pFile);
	int result = ferror(pFile);
	if (result) {
		printf("%s", "cant't open file");
	}
	fclose(pFile);
}

void init_l() {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	//свойства материала
	float front_color[] = {0.5, 0.5, 0.5};
	float back_color[] = {0.5, 0.5, 0.5};
	float front_back_color[] = {0.5, 0.5, 0.5};
	glMaterialfv(GL_FRONT, GL_AMBIENT, front_color);
	glMaterialfv(GL_BACK, GL_DIFFUSE, back_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, front_back_color);
	float light0_ambient[4] = {0.5, 0.5, 0.5, 1};
	float light0_diffuse[]  = {0.5, 0.5, 0.5};
	float light0_specular[]  = {1, 1, 1};
	//float light0_direction[] = {-0.5, -0.5, 1, 1};
	//float light0_direction[] = {45, 45, 45, 0};
	float light0_direction[] = {0, 0, 1, 0};
	glEnable(GL_LIGHT0);									//разрешаем использовать light0
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);		//фоновое излучение
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);		//диффузное
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);		//зеркальное
	glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);	//устанавливаем направление источника света указанным ранее
}

GLuint picture[3];
FILE* img;
unsigned char *data;
void Texture(int k1, int k2, int k3, int k4) {
	FILE* img;
	unsigned char *data;

	if (k1) {
	img = fopen("land.bmp","r");
	data = new unsigned char [640056];
	fread(data, 640056, 1, img);
	fclose(img);
	glGenTextures(1, &picture[0]);
	glActiveTextureARB (GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, picture[0]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 400, 400, GL_RGBA, GL_UNSIGNED_BYTE, data + 54);
	//текстура окружающей среды
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	delete []data;
	}

	if (k2) {
    img = fopen("water.bmp","r");
	//data = new unsigned char [201040];
	data = new unsigned char [426008];
	//читает из img, 1 объект, длиной 786486, сохраняет в data
	fread(data, 426008, 1, img);
	fclose(img);
	//создать 1 идентификатор текстуры, &picture - указатель на массив unsigned int!
	glGenTextures(2, &picture[1]);
	//двумерное изображение, идентификатор
	glActiveTextureARB (GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, picture[1]);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 259, 194, GL_RGBA, GL_UNSIGNED_BYTE, data + 54);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 493, 216, GL_RGBA, GL_UNSIGNED_BYTE, data + 54);
	//текстура окружающей среды
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	delete []data;
	}

	if (k3) {
    img = fopen("box.bmp","r");
	data = new unsigned char [161324];
	fread(data, 161324, 1, img);
	fclose(img);
	glGenTextures(3, &picture[2]);
	glActiveTextureARB (GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, picture[2]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 267, 151, GL_RGBA, GL_UNSIGNED_BYTE, data + 54);
	//текстура окружающей среды
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	delete []data;
	}

	if (k4) {
    img = fopen("venice.bmp","r");
	data = new unsigned char [467572];
	fread(data, 467572, 1, img);
	fclose(img);
	glGenTextures(4, &picture[3]);
	glActiveTextureARB (GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, picture[3]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 550, 252, GL_RGB, GL_UNSIGNED_BYTE, data + 54);
	//текстура окружающей среды
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	delete []data;
	}

}

void Draw_landscape() {
	if (use_text) {
		//Texture(1, 0, 0, 0);
		glBindTexture(GL_TEXTURE_2D, picture[0]);
	}
	double tex1_land = 0;
	double tex2_land = 1.0/40.0;
	double offset = 1.0/40.0;

	glFrontFace(GL_CCW);
	for (int y = 0; y < lvl_land; y++) {
		for (int x = 0; x < count_land; x ++) {
			struct elem p2;
			p2.elemx = landscape[y][x].elemx;
			p2.elemy = landscape[y][x].elemy;
			p2.elemz = landscape[y][x].elemz;
			struct elem p3;
			p3.elemx = landscape[y+1][x].elemx;
			p3.elemy = landscape[y+1][x].elemy;
			p3.elemz = landscape[y+1][x].elemz;
			struct elem p4;
			p4.elemx = landscape[y+1][x+1].elemx;
			p4.elemy = landscape[y+1][x+1].elemy;
			p4.elemz = landscape[y+1][x+1].elemz;
			struct elem p1;
			p1.elemx = landscape[y][x+1].elemx;
			p1.elemy = landscape[y][x+1].elemy;
			p1.elemz = landscape[y][x+1].elemz;
			struct elem vect1 = vector(p1, p2);
			struct elem vect2 = vector(p1, p4);
			struct elem nl = normal(vect1, vect2);
			glBegin(GL_TRIANGLES);
			glNormal3f(nl.elemx, nl.elemy, nl.elemz);
			if (use_text) {
				glTexCoord2d(tex1_land, tex2_land);
			}
			glVertex3d(p1.elemx, p1.elemy, p1.elemz);
			if (use_text) {
				glTexCoord2d(tex1_land, (tex2_land - offset));
			}
			glVertex3d(p2.elemx, p2.elemy, p2.elemz);
			if (use_text) {
				glTexCoord2d((tex1_land + offset), (tex2_land - offset));
			}
			glVertex3d(p3.elemx, p3.elemy, p3.elemz);
			glEnd();
			glBegin(GL_TRIANGLES);
			glNormal3f(nl.elemx, nl.elemy, nl.elemz);
			if (use_text) {
				glTexCoord2d(tex1_land, tex2_land);
			}
			glVertex3d(p1.elemx, p1.elemy, p1.elemz);
			if (use_text) {
				glTexCoord2d((tex1_land + offset), (tex2_land - offset));
			}
			glVertex3d(p3.elemx, p3.elemy, p3.elemz);
			if (use_text) {
				glTexCoord2d((tex1_land + offset), tex2_land);
			}
			glVertex3d(p4.elemx, p4.elemy, p4.elemz);
			glEnd();
			tex2_land += offset;
		}
		tex1_land += offset;
	}

	/*landscape[0][count_land].elemx;	//слева
	landscape[lvl_land][0].elemx;		//справа
	landscape[lvl_land][count_land].elemx;	//сзади
	landscape[0][0].elemx;				//спереди*/
	
	if (use_text) {
		//Texture(0, 0, 0, 1);
		glBindTexture(GL_TEXTURE_2D, picture[3]);
	}
	struct elem p1;
	p1.elemx = landscape[0][count_land].elemx;	//слева
	p1.elemy = landscape[0][count_land].elemy;
	p1.elemz = landscape[0][count_land].elemz;
	struct elem p2;
	p2.elemx = landscape[0][count_land].elemx;
	p2.elemy = landscape[0][count_land].elemy;
	p2.elemz = landscape[0][count_land].elemz-40;
	struct elem p3;
	p3.elemx = landscape[lvl_land][count_land].elemx;
	p3.elemy = landscape[lvl_land][count_land].elemy;
	p3.elemz = landscape[lvl_land][count_land].elemz-40;
	struct elem p4;
	p4.elemx = landscape[lvl_land][count_land].elemx;
	p4.elemy = landscape[lvl_land][count_land].elemy;
	p4.elemz = landscape[lvl_land][count_land].elemz;
	struct elem vect1 = vector(p1, p2);
	struct elem vect2 = vector(p1, p4);
	glBegin(GL_QUADS);
	struct elem nl = normal(vect1, vect2);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(p1.elemx, p1.elemy, p1.elemz);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(p2.elemx, p2.elemy, p2.elemz);
	if (use_text) {
		glTexCoord2d(0.5, 1);
	}
	glVertex3d(p3.elemx, p3.elemy, p3.elemz);
	if (use_text) {
		glTexCoord2d(0.5, 0);
	}
	glVertex3d(p4.elemx, p4.elemy, p4.elemz);
	glEnd();

	p1.elemx = landscape[lvl_land][count_land].elemx;	//сзади
	p1.elemy = landscape[lvl_land][count_land].elemy;
	p1.elemz = landscape[lvl_land][count_land].elemz;
	p2.elemx = landscape[lvl_land][count_land].elemx;
	p2.elemy = landscape[lvl_land][count_land].elemy;
	p2.elemz = landscape[lvl_land][count_land].elemz-40;
	p3.elemx = landscape[lvl_land][0].elemx;
	p3.elemy = landscape[lvl_land][0].elemy;
	p3.elemz = landscape[lvl_land][0].elemz-40;
	p4.elemx = landscape[lvl_land][0].elemx;
	p4.elemy = landscape[lvl_land][0].elemy;
	p4.elemz = landscape[lvl_land][0].elemz;
	vect1 = vector(p1, p2);
	vect2 = vector(p1, p4);
	glBegin(GL_QUADS);
	nl = normal(vect1, vect2);
	if (use_text) {
		glTexCoord2d(0.5, 0);
	}
	glVertex3d(p1.elemx, p1.elemy, p1.elemz);
	if (use_text) {
		glTexCoord2d(0.5, 1);
	}
	glVertex3d(p2.elemx, p2.elemy, p2.elemz);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(p3.elemx, p3.elemy, p3.elemz);
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(p4.elemx, p4.elemy, p4.elemz);
	glEnd();

}

int flag_flag = 0;
void Draw_water() {
	if (use_text) {
		//Texture(0, 1, 0, 0);
		glBindTexture(GL_TEXTURE_2D, picture[1]);
	}
	double tex1 = 0;
	double tex2 = 1.0/40.0;
	double offset = 1.0/40.0;

	//glTranslated(1, 1, 0);
    glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW);
	for (int y = 0; y < lvl; y++) {
		for (int x = 0; x < count; x ++) {
			struct elem p2;
			p2.elemx = water[y][x].elemx;
			p2.elemy = water[y][x].elemy;
			p2.elemz = water[y][x].elemz;
			struct elem p3;
			p3.elemx = water[y+1][x].elemx;
			p3.elemy = water[y+1][x].elemy;
			p3.elemz = water[y+1][x].elemz;
			struct elem p4;
			p4.elemx = water[y+1][x+1].elemx;
			p4.elemy = water[y+1][x+1].elemy;
			p4.elemz = water[y+1][x+1].elemz;
			struct elem p1;
			p1.elemx = water[y][x+1].elemx;
			p1.elemy = water[y][x+1].elemy;
			p1.elemz = water[y][x+1].elemz;
			struct elem vect1 = vector(p1, p2);
			struct elem vect2 = vector(p1, p4);
			struct elem nl = normal(vect1, vect2);
			glBegin(GL_QUADS);
			glNormal3f(nl.elemx, nl.elemy, nl.elemz);
			if (use_text) {
				glTexCoord2d(tex1, tex2);
			}
			glVertex3d(p1.elemx, p1.elemy, p1.elemz);
			if (use_text) {
				glTexCoord2d(tex1, (tex2 - offset));
			}
			glVertex3d(p2.elemx, p2.elemy, p2.elemz);
			if (use_text) {
				glTexCoord2d((tex1 + offset), (tex2 - offset));
			}
			glVertex3d(p3.elemx, p3.elemy, p3.elemz);
			if (use_text) {
				glTexCoord2d((tex1 + offset), tex2);
			}
			glVertex3d(p4.elemx, p4.elemy, p4.elemz);
			glEnd();
			tex2 += offset;
		}
		tex1 += offset;
	}
	//glTranslated(-1, -1, 0);
}

void Draw_object() {
	if (use_text) {
		//Texture(0, 0, 1, 0);
		glBindTexture(GL_TEXTURE_2D, picture[2]);
	}
	/*GLUquadricObj *quadObj; 
	quadObj = gluNewQuadric(); 
	gluQuadricTexture(quadObj, GL_TRUE);
	gluQuadricDrawStyle(quadObj, GLU_FILL); */
	glPushMatrix();
	glTranslated(object_x, object_y, -box_height);
	//glTranslated(0.5+object_x, 0.5+object_y, -0.15+box_height);
	//gluSphere(quadObj, 2, 20, 20);
	glBegin(GL_QUADS);	//спереди
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(0, 5, 3);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(0, 0, 3);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(0, 0, -1);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(0, 5, -1);
	glEnd();
	glBegin(GL_QUADS);	//справа
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(0, 0, 3);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(5, 0, 3);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(5, 0, -1);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(0, 0, -1);
	glEnd();
	glBegin(GL_QUADS);	//низ
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(5, 5, 3);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(0, 5, 3);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(0, 0, 3);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(5, 0, 3);
	glEnd();
	glBegin(GL_QUADS);	//верх
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(5, 5, -1);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(0, 5, -1);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(0, 0, -1);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(5, 0, -1);
	glEnd();
	glBegin(GL_QUADS);	//сзади
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(5, 0, 3);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(5, 5, 3);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(5, 5, -1);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(5, 0, -1);
	glEnd();
	glBegin(GL_QUADS);	//слева
	if (use_text) {
		glTexCoord2d(1, 0);
	}
	glVertex3d(0, 0, 3);
	if (use_text) {
		glTexCoord2d(0, 0);
	}
	glVertex3d(0, 5, 3);
	if (use_text) {
		glTexCoord2d(0, 1);
	}
	glVertex3d(0, 5, -1);
	if (use_text) {
		glTexCoord2d(1, 1);
	}
	glVertex3d(0, 0, -1);
	glEnd();
    glPopMatrix();
	//gluDeleteQuadric(quadObj);
}

void Reflection() {
	double eqr[] = {0, 0, 1, 0};	// Уравнение плоскости отсечения для отсечения отраженных объектов
	glColorMask(0, 0, 0, 0);	// Установить маску цвета => ничего не выводится на экран в данный момент
	glEnable(GL_STENCIL_TEST);	// Использовать буфер шаблона
	glStencilFunc(GL_ALWAYS, 1, 1);	// Всегда проходит, 1 битовая плоскость, маска = 1; 1, где рисуется хоть какой-нибудь полигон
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// Тест шаблона не прошёл => оставляем состояние буфера; тест шаблона успешен, тест глубины - нет; Тест пройден успешно => заменяем значение в буфере шаблона
	glDisable(GL_DEPTH_TEST);	// Отключить проверку глубины
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	Draw_water();	// Рисование отражающей поверхности (только в буфере шаблона)
	glColorMask(1, 1, 1, 1);	// Маска цвета = TRUE, TRUE, TRUE, TRUE
	glStencilFunc(GL_EQUAL, 1, 1);	// Рисуем по шаблону (где шаблон=1) - то есть в том месте, где была нарисована отражающая поверхность
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// Не изменять буфер шаблона
	glEnable(GL_CLIP_PLANE0);	// Включить плоскость отсечения для удаления артефактов(когда объект (шар) пересекает пол (воду))
	glClipPlane(GL_CLIP_PLANE0, eqr);	// Уравнение для отраженных объектов
	
	glPushMatrix();	// Сохранить матрицу в стеке
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
	glEnable(GL_BLEND);
	glScalef(1.0f, 1.0f, -1.0f);	// Перевернуть ось (относительно оси z)
	double eqr3[] = {0, 0, -1, 0};
	glClipPlane(GL_CLIP_PLANE0, eqr3);
	Draw_landscape();
	
	if (object != 1) {
		double eqr1[] = {0, 0, -1, -max_water};
		glClipPlane(GL_CLIP_PLANE0, eqr1);
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
		Draw_object();
	} else {
		glScalef(1.0f, 1.0f, -1.0f);
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
		Draw_object();
		glScalef(1.0f, 1.0f, -1.0f);
	}
	glDisable(GL_BLEND);
	glPopMatrix();	// Восстановить матрицу

	glEnable(GL_DEPTH_TEST);	// Включить проверку глубины
	glDisable(GL_CLIP_PLANE0);	// Отключить плоскость отсечения
	glDisable(GL_STENCIL_TEST);	// Отключение проверки шаблона
}

void draw_scene() {
	// Очистка экрана, буфера глубины и буфера шаблона
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	if (use_light == 1) {
		init_l();
	}
	if (constant_tex == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (box_height >= 1) {
		double eqr2[] = {0, 0, 1, 0};
		glClipPlane(GL_CLIP_PLANE0, eqr2);
		glEnable(GL_CLIP_PLANE0);
		Draw_landscape();
		glDisable(GL_CLIP_PLANE0);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		Draw_water();
	QueryPerformanceCounter(&time_start2);
	QueryPerformanceFrequency(&frequent2);
		Reflection();
	QueryPerformanceCounter(&time_end2);
	printf("reflection1=%f\n", ((double)(time_end2.QuadPart) - (double)(time_start2.QuadPart))/(double)(frequent2.QuadPart));
		double eqr1[] = {0, 0, -1, 0};
		glClipPlane(GL_CLIP_PLANE0, eqr1);
		glEnable(GL_CLIP_PLANE0);
		Draw_landscape();
		glDisable(GL_CLIP_PLANE0);
		Draw_object();
	} else {
		object = 1;
		double eqr2[] = {0, 0, 1, 0};
		glClipPlane(GL_CLIP_PLANE0, eqr2);
		glEnable(GL_CLIP_PLANE0);
		Draw_landscape();
		glDisable(GL_CLIP_PLANE0);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		Draw_water();
	QueryPerformanceCounter(&time_start2);
	QueryPerformanceFrequency(&frequent2);
		Reflection();
	QueryPerformanceCounter(&time_end2);
	printf("reflection2=%f\n", ((double)(time_end2.QuadPart) - (double)(time_start2.QuadPart))/(double)(frequent2.QuadPart));
		double eqr1[] = {0, 0, -1, 0};
		glClipPlane(GL_CLIP_PLANE0, eqr1);
		glEnable(GL_CLIP_PLANE0);
		Draw_landscape();
		glDisable(GL_CLIP_PLANE0);
		object = 0;
		Draw_object();
	}
}

void count_landscape(int ll, int cc) {
	lvl_land = ll;
	count_land = cc;
	int i, j;
	if (landscape != NULL) {
		free_arr(landscape, lvl_land);
	}
	landscape = (struct elem**)malloc((lvl_land+1)*sizeof(struct elem*));
	for(i = 0; i < (lvl_land+1); i ++) {
		landscape[i] = (struct elem*)malloc((count_land+1)*sizeof(struct elem));
	}
	for(i = 0; i <= lvl_land; i ++) {
		for(j = 0; j <= count_land; j ++) {
			landscape[i][j].elemx = i;
			landscape[i][j].elemy = j;
			landscape[i][j].elemz = 7;
		}
	}
	for(i = 0; i <= lvl_land; i ++) {
		for(j = 0; j <= count_land; j ++) {
			landscape[i][j].elemz -= ((Height_map(map_5, i, j, count_land)))/6.9;
		}
	}
}

void count_water(int ll, int cc) {
	lvl = ll;
	count = cc;
	int i, j;

	if (!dynamic_wave) {
		max_water = 0;
	}

	if (water != NULL) {
		free_arr(water, lvl);
	}
	water = (struct elem**)malloc((lvl+1)*sizeof(struct elem*));
	for(i = 0; i < (lvl+1); i ++) {
		water[i] = (struct elem*)malloc((count+1)*sizeof(struct elem));
	}
	for(i = 0; i <= lvl; i ++) {
		for(j = 0; j <= count; j ++) {
			water[i][j].elemx = i;
			water[i][j].elemy = j;
			water[i][j].elemz = 0;
		}
	}

	if (dynamic_wave == 0) {
		for (int y = 0; y <= lvl; y++) {
			for (int x = 0; x <= count; x ++) {
				if (key_for_wave) {
					int random_p = rand() % 100;
					if (random_p % 2 == 0) {
						water[y][x].elemz += sin(Height_map(map_1, x, y, count))/4;
					} else if (random_p % 3 == 0) {
						water[y][x].elemz += sin(Height_map(map_2, x, y, count))/4;
					} else if (random_p % 5 == 0) {
						water[y][x].elemz += sin(Height_map(map_3, x, y, count))/4;
					} else {
						water[y][x].elemz += sin(Height_map(map_4, x, y, count))/4;
					}
					if (water[y][x].elemz > max_water) {
						max_water = water[y][x].elemz;
					}
				}
			}
		}
	} else {
		for (int y = 0; y <= lvl; y++) {
			for (int x = 0; x <= count; x ++) {
				water[y][x].elemx = x;
				water[y][x].elemy = y;
				water[y][x].elemz = 0;
			}
		}
		dynamic += 2;
		for (int y = 0; y <= lvl; y++) {
			for (int x = 0; x <= count; x ++) {
                water[y][x].elemz += (float(sin(((((dynamic+x)/5.0f)*40.0f)/360.0f)*3.141592654*2.0f)));
				if (water[y][x].elemz > max_water) {
					max_water = water[y][x].elemz;
				}
			}
		}
	}
}

void Redraw() {
	//if (use_light == 1) {
	//	init_l();
	//}
	count_landscape(lvl_land, count_land);
	count_water(lvl, count);
QueryPerformanceCounter(&time_start1);
QueryPerformanceFrequency(&frequent1);
	draw_scene();
QueryPerformanceCounter(&time_end1);
printf("scene=%f\n", ((double)(time_end1.QuadPart) - (double)(time_start1.QuadPart))/(double)(frequent1.QuadPart));
	glutSwapBuffers();
	glFlush();
}

void Timer(int val) {
	if (key_for_wave) {
		Redraw();
	}
	if (key_for_big_wave) {
		dynamic_wave = 1;
		Redraw();
		dynamic_wave = 0;
	}
	glutTimerFunc(350, Timer, 0);	//время в миллисекундах, ф-ия, идентификатор таймера
}

void Display() {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);	//МАТРИЦА ПРОЕКЦИИ
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);		//выбор матрицы преобразований для изменения - МАТРИЦА МОДЕЛИ (4*4)
	glLoadIdentity();				//делаем матрицу единичной (матрица 4*4)

	glRotated(x_angle, 1, 0, 0);		//угол поворота, x, y, z
	glRotated(y_angle, 0, 1, 0);
	glRotated(z_angle, 0, 0, 1);

	if (use_text) {
		Texture(1, 1, 1, 1);
	}

	map_1 = (BYTE*)malloc((lvl*count)*sizeof(BYTE));
	map_2 = (BYTE*)malloc((lvl*count)*sizeof(BYTE));
	map_3 = (BYTE*)malloc((lvl*count)*sizeof(BYTE));
	map_4 = (BYTE*)malloc((lvl*count)*sizeof(BYTE));
	Load_Height_map(map_1, "water_1.raw", lvl*count);
	Load_Height_map(map_2, "water_2.raw", lvl*count);
	Load_Height_map(map_3, "water_3.raw", lvl*count);
	Load_Height_map(map_4, "water_4.raw", lvl*count);

	map_5 = (BYTE*)malloc((lvl_land*count_land)*sizeof(BYTE));
	Load_Height_map(map_5, "land.raw", lvl_land*count_land);

//-----------------------------------------------------------------------------------------------

	glRotated(60, 1, 0, 0);
	glRotated(50, 0, 0, 1);
	//glTranslated(-1.2, -1, -0.3);
	glTranslated(-1, -0.9, 0);

	GLfloat ModelView[16];
	GLfloat XYZCameraCoordinates[3];
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	XYZCameraCoordinates[0] = ModelView[0] * ModelView[12] + ModelView[1] * ModelView[13] + ModelView[2] *  ModelView[14];
	XYZCameraCoordinates[1] = ModelView[4] * ModelView[12] + ModelView[5] * ModelView[13] + ModelView[6] *  ModelView[14];
	XYZCameraCoordinates[2] = ModelView[8] * ModelView[12] + ModelView[9] * ModelView[13] + ModelView[10] * ModelView[14];

	/*XYZCameraCoordinates[0] = ModelView[12];
	XYZCameraCoordinates[1] = ModelView[13];
	XYZCameraCoordinates[2] = ModelView[14];*/

	if (use_light == 1) {
		init_l();
	}
	glScaled(scale, scale, scale);
	//glViewport(Width/10.0, Height/10.0, 900, 900);
	count_landscape(lvl_land, count_land);
	count_water(lvl, count);
	draw_scene();
	glDisable(GL_LIGHT0);
	glutSwapBuffers();
	glFlush();
}

void Fog() {
	GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR }; // Хранит три типа тумана
	GLuint fogfilter = 2;
	GLfloat fogColor[4]= {0.55f, 0.55f, 0.55f, 1.0f}; // Цвет тумана
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, fogMode[fogfilter]);	// Выбираем тип тумана
	glFogfv(GL_FOG_COLOR, fogColor);			// Устанавливаем цвет тумана
	glFogf(GL_FOG_DENSITY, 2.0f);				// Насколько густым будет туман
	glHint(GL_FOG_HINT, GL_DONT_CARE);			// Вспомогательная установка тумана
	glFogf(GL_FOG_START, 0.9f);					// Глубина, с которой начинается туман
	glFogf(GL_FOG_END, -0.2f);
}

void Initialize() {
	glLineWidth(0.5);						//ширина линии
	glPointSize(1.0);						//точки
	glClearColor(0.9f, 0.9f, 0.9f, 0.9f);	//цвет фона - RGB, alpha
	glColor3f(0.1f, 0.1f, 0.1f);			//текущий цвет - RGB
	count_water(lvl, count);
	glewInit();								//ДЛЯ МУЛЬТИТЕКСТУРИРОВАНИЯ!
	//Fog();
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);   
	Width = width;
	Height= height;
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void Save() {
	FILE *file;
	file = fopen("savetxt.txt", "w");	//w => только для записи, r => только для чтения
	//указатель на файл, формат записи (%d - целочисл., %f - вещественные), переменные для записи
	//fprintf(file, "%d\t%d\t%f\n", err, jump, v0);
	for (int y = 0; y <= lvl; y++) {
		for (int x = 0; x <= count; x ++) {
			fprintf(file, "%f\t%f\t%f\n", water[y][x].elemx, water[y][x].elemy, water[y][x].elemz);
		}
	}
	fclose(file);
}

void Load() {
	FILE *file;
	file = fopen("savetxt.txt", "r");
	//fscanf(file, "%d%d%f", &err, &jump, &v0);
	for (int y = 0; y <= lvl; y++) {
		for (int x = 0; x <= count; x ++) {
			fscanf(file, "%f%f%f", &water[y][x].elemx, &water[y][x].elemy, &water[y][x].elemz);
		}
	}
	fclose(file);
}

void keyboard(unsigned char key,  int x, int y) {
	if (key == 'a') {
 		y_angle --;
		Redraw();
	}
	if (key == 'd') {
 		y_angle ++;
		Redraw();
	}
	if (key == 'w') {	
 		x_angle --;
		Redraw();
	}
	if (key == 's') {	
 		x_angle ++;
		Redraw();
	}
	if (key == 'q') {	
 		z_angle --;
		Redraw();
	}
	if (key == 'e') {	
 		z_angle ++;
		Redraw();
	}
	if (key == 'z') {	
 		scale += 0.001;
		Redraw();
	}
	if (key == 'x') {	
 		scale -= 0.001;
		if (scale < 0) {
			scale = 0;
		}
		Redraw();
	}
	if (key == 't') {	
		if (constant_tex == 0) {
			constant_tex = 1;
			Redraw();
		} else {
			constant_tex = 0;
		}
	}
	if (key == '-') {	
		if (use_light == 0) {
			use_light = 1;
		} else {
			use_light = 0;
		}
	}
	if (key == '1') {
		object_x ++;
	}
	if (key == '2') {
		object_x --;
		if (object_x < 0) {
			object_x = 0;
		}
	}
	if (key == '3') {
		object_y ++;
	}
	if (key == '4') {
		object_y --;
		if (object_y < 0) {
			object_y = 0;
		}
	}
	if (key == '[') {
		Save();
	}
	if (key == ']') {
		Load();
	}
	if (key == 'p') {
		if (use_text == 0) {
			use_text = 1;
		} else {
			glDisable(GL_TEXTURE_2D);
		}
	}
	if (key == 'k') {
		//тело не отражает
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	if (key == 'l') {
		//тело отражает
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	}
	if (key == ';') {
		//тело прозрачное
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
	if (key == 'g') {
		key_for_wave = 1;
		Redraw();
	}
	if (key == 'h') {
		key_for_wave = 0;
		Redraw();
	}
	if (key == ' ') {
		count_cnst = count;
		lvl_cnst = lvl;
		key_for_big_wave = 1;
		Redraw();
	}
	if (key == 'b') {
		key_for_big_wave = 0;
		Redraw();
	}
	if (key == 'n') {
		box_height ++;
	}
	if (key == 'm') {
		box_height --;
	}
	glutPostRedisplay();			//перерисовка текущего окна
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_STENCIL );
	glutInitWindowSize(900, 900);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("WATER");
	Initialize();
	glutDisplayFunc(Display);	//функция, рисующая в окне
	Display();
	glutIdleFunc(NULL);
	glutReshapeFunc(reshape);	//функция, отвечающая за изменение размеров окна
	glutKeyboardFunc(keyboard);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glutTimerFunc(10, Timer, 0);

	glutMainLoop();
	free(map_1);
	free(map_2);
	free(map_3);
	free(map_4);
	free(map_5);

	return 0;
}