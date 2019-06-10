
/* mode 0 = wire frame, mode 1 = constant shading,
mode 3 = interpolative shading */
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

static float year = 0,day = 0;

typedef float point[4];

/* initial tetrahedron */

point v[]={{0.0, 0.0, 1.0}, {0.0, 0.942809, -0.33333},
		   {-0.816497, -0.471405, -0.333333}, {0.816497, -0.471405, -0.333333}};

int n=7;
int mode;

GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat position[] = { 0.0, 0.0, 0.0, 1.0 };


//�������������;
GLuint earthTex,moonTex;

#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����

// ����power_of_two�����ж�һ�������ǲ���2����������
int power_of_two(int n)
{
	if( n <= 0 )
		return 0;
	return (n & (n-1)) == 0;
}

/* ����load_texture
* ��ȡһ��BMP�ļ���Ϊ����
* ���ʧ�ܣ�����0������ɹ�������������
*/
GLuint load_texture(const char* file_name)
{	
	GLint lastTextureID;
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID=0, texture_ID = 0; 
	// ���ļ������ʧ�ܣ�����	
	FILE* pFile = fopen(file_name, "rb");
	if( pFile == 0 )
		return 0; 
	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�
	fseek(pFile, 0x0012, SEEK_SET);	
	fread(&width, 4, 1, pFile);	
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);
	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���
	{
		GLint line_bytes = width * 3;
		while( line_bytes % 4 != 0 )
			++line_bytes;
		total_bytes = line_bytes * height;
	} 	
	// �����������ֽ��������ڴ�	
	pixels = (GLubyte*)malloc(total_bytes);	
	if( pixels == 0 )
	{
		fclose(pFile);
		return 0;
	} 	
	// ��ȡ��������	
	if( fread(pixels, total_bytes, 1, pFile) <= 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	} 	
	// �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������	
	// ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����	
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if( !power_of_two(width) || !power_of_two(height) || width > max || height > max )
		{	
			const GLint new_width = 256;
			const GLint new_height = 256; 
			// �涨���ź��µĴ�СΪ�߳���������
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;
			// ����ÿ����Ҫ���ֽ��������ֽ���
			new_line_bytes = new_width * 3;
			while( new_line_bytes % 4 != 0 )
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;
			// �����ڴ�
			new_pixels = (GLubyte*)malloc(new_total_bytes);	
			if( new_pixels == 0 )
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}
			// ������������
			gluScaleImage(GL_RGB,width, height, GL_UNSIGNED_BYTE, pixels, new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);
			// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}
	// ����һ���µ�������
	glGenTextures(1, &texture_ID);
	if( texture_ID == 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}
	// ���µ������������������������	
	// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�
	lastTextureID=last_texture_ID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);	
	glBindTexture(GL_TEXTURE_2D, lastTextureID);   
	//�ָ�֮ǰ�������
	free(pixels);
	return texture_ID;
}

void triangle( point a, point b, point c)

	/* display one triangle using a line loop for wire frame, a single
	normal for constant shading, or three normals for interpolative shading */
{
	if (mode==0) glBegin(GL_LINE_LOOP);
	else glBegin(GL_POLYGON);
	if(mode==1) glNormal3fv(a);
	if(mode==2) glNormal3fv(a);
	glVertex3fv(a);
	if(mode==2) glNormal3fv(b);
	glVertex3fv(b);
	if(mode==2) glNormal3fv(c);
	glVertex3fv(c);
	glEnd();
}

void normal(point p)
{

	/* normalize a vector */

	double sqrt();
	float d =0.0;
	int i;
	for(i=0; i<3; i++) d+=p[i]*p[i];
	d=sqrt(d);
	if(d>0.0) for(i=0; i<3; i++) p[i]/=d;
}

void divide_triangle(point a, point b, point c, int m)
{
	/* triangle subdivision using vertex numbers
	righthand rule applied to create outward pointing faces */

	point v1, v2, v3;
	int j;
	if(m>0)
	{
		for(j=0; j<3; j++) v1[j]=a[j]+b[j];
		normal(v1);
		for(j=0; j<3; j++) v2[j]=a[j]+c[j];
		normal(v2);
		for(j=0; j<3; j++) v3[j]=b[j]+c[j];
		normal(v3);
		divide_triangle(a, v1, v2, m-1);
		divide_triangle(c, v2, v3, m-1);
		divide_triangle(b, v3, v1, m-1);
		divide_triangle(v1, v3, v2, m-1);
	}
	else(triangle(a,b,c)); /* draw triangle at end of recursion */
}

void tetrahedron( int m)
{

	/* Apply triangle subdivision to faces of tetrahedron */

	divide_triangle(v[0], v[1], v[2], m);
	divide_triangle(v[3], v[2], v[1], m);
	divide_triangle(v[0], v[3], v[1], m);
	divide_triangle(v[0], v[2], v[3], m);
}



void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,3.0,-3.0, 0.0,0.0,0.0, 0.0,0.0,1.0);

	
	mode = 0;
    glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1.0,1.0,0.0);
	glTranslatef(0.0,0.0,0.0);
	tetrahedron(n);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glPopMatrix();

	mode=1;
	glRotatef((GLfloat)year,0.0,1.0,0.0);
	glTranslatef(-2.0,0.0,4.0);
	glRotatef((GLfloat)day,0.0,0.0,1.0);
	glScalef(0.4,0.4,0.4);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,earthTex);
	glEnable(GL_TEXTURE_2D);	
	tetrahedron(n);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	mode=2;
	glRotatef((GLfloat)year*40.0,0.0,1.0,0.0);
	glTranslatef(1.5,0.0,0.0);
	glRotatef((GLfloat)day/30*360-day,0.0,1.0,0.0);	
	glScalef(0.1,0.1,0.1);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,moonTex);
	glEnable(GL_TEXTURE_2D);
	tetrahedron(n);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

void spinDisplay(void)
{
	year = year + 0.2;
	if ( year > 360.0)
	{
	year = year - 360.0;
	}
	day = day + 0.2;
	if ( day > 360.0)
	{
	day = day - 360.0;
	}
	glutPostRedisplay();
}

void myReshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-4.0, 4.0, -4.0 * (GLfloat) h / (GLfloat) w,
		4.0 * (GLfloat) h / (GLfloat) w, -10.0, 10.0);
	else
		glOrtho(-4.0 * (GLfloat) w / (GLfloat) h,
		4.0 * (GLfloat) w / (GLfloat) h, -4.0, 4.0, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	//display();
	glLoadIdentity();
}


void myinit()
{
	glClearColor(0.0, 0.1, 0.1, 0.0);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT&GL_DIFFUSE);


	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glGenTextures(1,&earthTex);
	glBindTexture(GL_TEXTURE_2D,earthTex);
	glGenTextures(1,&moonTex);
	glBindTexture(GL_TEXTURE_2D,moonTex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);

}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	}
}

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("sphere");
	myinit();
	glEnable(GL_TEXTURE_2D);
	earthTex = load_texture("earth.bmp");
	moonTex = load_texture("moon.bmp");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	glutIdleFunc(spinDisplay);
	glutMainLoop();
}
