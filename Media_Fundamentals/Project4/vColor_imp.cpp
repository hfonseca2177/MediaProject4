
#include"vColor.h"

//GLfloat r, g, b, a;
//constructors
vColor::vColor() {
	//default color is blue-ish
	r = 0.023f;
	g = 0.196f;
	b = 0.192f;
	a = 1.0f;

}
vColor::vColor(GLfloat red, GLfloat green, GLfloat blue) {

	r = red;
	g = green;
	b = blue;
	a = 1.0f;
}
vColor::vColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	r = red;
	g = green;
	b = blue;
	a = alpha;
}

//functions

void vColor::setColor(GLfloat red, GLfloat green, GLfloat blue) {

	r = red;
	g = green;
	b = blue;
	a = 1.0f;
}
void vColor::setColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {

	r = red;
	g = green;
	b = blue;
	a = alpha;
}
