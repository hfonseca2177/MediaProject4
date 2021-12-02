
#pragma once
#include<GLFW/glfw3.h>



struct vColor
{
	//custom struct color
	GLfloat r, g, b, a;
	//constructors
	vColor();
	vColor(GLfloat r, GLfloat g, GLfloat b);
	vColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

	//functions
	void setColor(GLfloat r, GLfloat g, GLfloat b);
	void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);


};


