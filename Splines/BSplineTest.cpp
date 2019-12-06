//	----==== BSPLINETEST.CPP ====----
//
//	Author:			Jeff Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			6/04
//	Description:	Tests 3D B-Splines and Catmull-Rom Splines although it appears 2D
//	-------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN		// this keeps MFC (Microsoft Foundation Classes) from being included

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <time.h>
#include "utilitycode/keyboardmanager.h"
#include "utilitycode/mousemanager.h"
#include "mathcode/spline.h"
#include "mathcode/bezier.h"
#include "mathcode/vector2.h"
#include "mathcode/vector3.h"
#include "mathcode/vector4.h"
#include "utilitycode/glfont.h"


/*---------------
---- DEFINES ----
---------------*/

#define NUMPOINTS		10
#define ITERATIONS		30


/*-----------------
---- VARIABLES ----
-----------------*/

extern HDC		hDC;
extern GLFont	*font;

Vector3	p[NUMPOINTS];
bool	bezierOn = true;
bool	catmullRomOn = true;
bool	bSplineOn = true;
bool	bSplineSeriesOn = true;
bool	controlPointsOn = true;
bool	knotVectorOpen = true;
bool	drawLines = true;
int		bSplineK = 4;
float	tangentT = 0.0f;
int		tangentI = 0;


/*-----------------
---- FUNCTIONS ----
-----------------*/

void setupControlPoints(void)
{
	srand(time(NULL) + (rand() % 100));

	for (int c = 0; c < NUMPOINTS; c++) {
		p[c].x = (c * 2.0f) - 9;
		p[c].y = (rand() % 10) - 5.0f;
		p[c].z = 0;
	}
}


void drawCatmullRomSpline(void) {
	Vector3	currentP(0,0,0);

	// draw iterations for the spline
	glColor3f(1,0,0);

	// find the interpolation time step
	float tStep = 1.0f / (float)ITERATIONS;

	if (drawLines) glBegin(GL_LINE_STRIP);
	else glBegin(GL_POINTS);

		// draw NUMPOINTS - 3 segments
		for (int i = 0; i < NUMPOINTS-3; i++) {
	
			for (float t = 0; t < 1.0f; t += tStep) {

				currentP = CatmullRomSpline::calcPointOnSpline(t, p[i], p[1+i], p[2+i], p[3+i]);

				glVertex3fv(currentP.v);
			}

		}

	glEnd();
}


void drawBSpline(bool openKnot)
{
	Vector3	currentP(0,0,0);

	// draw iterations for the spline
	glColor3f(0,0,0);

	if (drawLines) glBegin(GL_LINE_STRIP);
	else glBegin(GL_POINTS);
		if (openKnot) {

			// find the interpolation time step
			float tStep = 1.0f / (float)(ITERATIONS * NUMPOINTS);
			
			for (float t = 0; t < 1.0f; t += tStep) {
				currentP = BSpline::calcPointOnBSpline(t, bSplineK, p, NUMPOINTS, BSpline::BSPLINE_TYPE_OPEN_NORMALIZED);
				glVertex3fv(currentP.v);
			}

		} else {

			// find the interpolation time step
			float tStep = 1.0f / (float)ITERATIONS;

			for (float t = (float)(bSplineK-1); t < NUMPOINTS; t += tStep) {
				currentP = BSpline::calcPointOnBSpline(t, bSplineK, p, NUMPOINTS, BSpline::BSPLINE_TYPE_PERIODIC_NOT_NORMALIZED);
				glVertex3fv(currentP.v);
			}

		}

	glEnd();
}


void drawBSplineSeries(void)
{	
	// find the interpolation time step
	float tStep = 1.0f / (float)ITERATIONS;

	for (int i = 0; i < NUMPOINTS-3; i++) {
		
		glColor3f(1,0,1);
		CubicBSpline::preCalcSpline(Vector4(p[i].y,p[i+1].y,p[i+2].y,p[i+3].y));
		
		if (drawLines) glBegin(GL_LINE_STRIP);
		else glBegin(GL_POINTS);

			float t = 0;
			for (int c = 0; c <= ITERATIONS; c++) {
				Vector3	currentP(0,0,0);

				currentP.x = (i*2) + (t*2) - 7;
				currentP.y = CubicBSpline::getPreCalcHeight(t);
				currentP.z = 0;
							
				glVertex3fv(currentP.v);

				t += tStep;
			}

		glEnd();

		// This section shows the tangent curves (slow and precalc)
		glLineStipple(3,21845);
		glEnable(GL_LINE_STIPPLE);
		glColor3f(0,1,1);
		CubicBSpline::preCalcTangent(Vector4(p[i].y,p[i+1].y,p[i+2].y,p[i+3].y));

		if (drawLines) glBegin(GL_LINE_STRIP);
		else glBegin(GL_POINTS);

			t = 0;
			for (int c = 0; c <= ITERATIONS; c++) {
				Vector3 tangentP(0,0,0);

				tangentP.x = (i*2) + (t*2) - 7;
				tangentP.y = CubicBSpline::getPreCalcTangent(t);
				tangentP.z = 0;
				
				glVertex3fv(tangentP.v);

				t += tStep;
			}

		glEnd();

/*		glColor3f(1,0,0);
		if (drawLines) glBegin(GL_LINE_STRIP);
		else glBegin(GL_POINTS);

			t = 0;
			for (int c = 0; c <= ITERATIONS; c++) {
				Vector3 tangentP(0,0,0);

				tangentP.x = (i*2) + (t*2) - 7;
				tangentP.y = CubicBSpline::getTangentOnCubicBSpline(t,p[i].y,p[i+1].y,p[i+2].y,p[i+3].y);
				tangentP.z = 0;
				
				glVertex3fv(tangentP.v);

				t += tStep;
			}

		glEnd();*/

		CubicBSpline::preCalcConcavity(Vector4(p[i].y,p[i+1].y,p[i+2].y,p[i+3].y));
		glColor3f(0.5f,0.5f,0);
		if (drawLines) glBegin(GL_LINE_STRIP);
		else glBegin(GL_POINTS);

			t = 0;
			for (int c = 0; c <= ITERATIONS; c++) {
				Vector3 concP(0,0,0);

				concP.x = (i*2) + (t*2) - 7;
				//concP.y = CubicBSpline::getConcavityOnCubicBSpline(t,p[i].y,p[i+1].y,p[i+2].y,p[i+3].y);
				concP.y = CubicBSpline::getPreCalcConcavity(t); 
				concP.z = 0;
				
				glVertex3fv(concP.v);

				t += tStep;
			}

		glEnd();
		
		glBegin(GL_LINES);
			glColor3f(0,1,0);
			glVertex3f(-7,0,0);
			glVertex3f(7,0,0);

			glColor3f(0.3f,0.8f,0.3f);
			for (int l = 1; l < 10; ++l) {				
				glVertex3f(-7,(float)l,0);
				glVertex3f(7,(float)l,0);
				glVertex3f(-7,-(float)l,0);
				glVertex3f(7,-(float)l,0);
			}

		glEnd();

		glDisable(GL_LINE_STIPPLE);
	}

	// show the tangent line
	glColor3f(0,0.5f,0);
	float t = tangentT;
	int ti = tangentI;
	CubicBSpline::preCalcSpline(Vector4(p[ti].y,p[ti+1].y,p[ti+2].y,p[ti+3].y));
	CubicBSpline::preCalcTangent(Vector4(p[ti].y,p[ti+1].y,p[ti+2].y,p[ti+3].y));
	glBegin(GL_LINES);
		glVertex3f(ti*2+t*2-7-2, CubicBSpline::getPreCalcHeight(t)-CubicBSpline::getPreCalcTangent(t), 0);
		glVertex3f(ti*2+t*2-7+2, CubicBSpline::getPreCalcHeight(t)+CubicBSpline::getPreCalcTangent(t), 0);
	glEnd();
}


void drawBezierCurves(void)
{
	Vector3	currentP(0,0,0);

	// draw iterations for the spline
	glColor3f(0,0.8f,0);

	// find the interpolation time step
	float tStep = 1.0f / (float)ITERATIONS;
	
	for (int i = 0; i < NUMPOINTS-3; i += 3) {
		if (drawLines) glBegin(GL_LINE_STRIP);
		else glBegin(GL_POINTS);
		
			for (float t = 0; t < 1.0f; t += tStep) {
				currentP = BezierCurve::calcPointOnCurve(t, p[0+i],p[1+i],p[2+i],p[3+i]);
				glVertex3f(currentP.x, currentP.y, currentP.z);
			}

		glEnd();
	}
}


void drawControlPoints(void)
{
	// draw the control points in blue
	glPointSize(8.0f);
	glColor3f(0,0,1);

	for (int c = 0; c < NUMPOINTS; c++) {		
		glBegin(GL_POINTS);
			glVertex3f(p[c].x, p[c].y, p[c].z);
		glEnd();
	}

	glPointSize(1.5f);
}


void renderScene(void)
{
	// Perform a page flip immediately prior to doing all rendering for best performance
	SwapBuffers(hDC);

	// Clear the back buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// handle keyboard input
	if (kb.buttonPressed('1')) bezierOn = !bezierOn;
	if (kb.buttonPressed('2')) catmullRomOn = !catmullRomOn;
	if (kb.buttonPressed('3')) bSplineOn = !bSplineOn;
	if (kb.buttonPressed('4')) knotVectorOpen = !knotVectorOpen;
	if (kb.buttonPressed('5')) bSplineSeriesOn = !bSplineSeriesOn;
	if (kb.buttonPressed('6')) controlPointsOn = !controlPointsOn;
	if (kb.buttonPressed('7')) drawLines = !drawLines;
	if (kb.buttonPressed(0xBB)) bSplineK == NUMPOINTS ? bSplineK : bSplineK++;
	if (kb.buttonPressed(0xBD)) bSplineK == 2 ? bSplineK : bSplineK--;
	if (kb.keyDown(VK_RIGHT)) tangentT += 0.02f;
	if (kb.keyDown(VK_LEFT)) tangentT -= 0.02f;
	if (tangentT < 0.0f) {
		if (tangentI > 0) {
			--tangentI;
			tangentT = 1.0f;
		} else {
			tangentT = 0.0f;
		}
	} 
	if (tangentT > 1.0f) {
		if (tangentI < NUMPOINTS-4) {
			++tangentI;
			tangentT = 0.0f;
		} else {
			tangentT = 1.0f;
		}
	}

	// handle mouse movement
	mouse.updateMousePosition();

	// reset the modelview matrix
	glLoadIdentity();

	// translate the view in the -z direction (straight back away from the screen)
	// so we can see the scene
	glTranslatef(0,-3,-10);

	// Draw the scene
	if (catmullRomOn) drawCatmullRomSpline();
	if (bSplineOn) drawBSpline(knotVectorOpen);
	if (bezierOn) drawBezierCurves();
	if (bSplineSeriesOn) drawBSplineSeries();
	if (controlPointsOn) drawControlPoints();

	// Write text
	glColor3f(0,0.8f,0);
	if (bezierOn)
		font->print(10,50, "<1> Bezier Curves ON");
	else
		font->print(10,50, "<1> Bezier Curves OFF");

	glColor3f(1,0,0);
	if (catmullRomOn)
		font->print(10,64, "<2> Catmull-Rom Spline ON");
	else
		font->print(10,64, "<2> Catmull-Rom Spline OFF");

	glColor3f(0,0,0);
	if (bSplineOn)
		font->print(10,78, "<3> Single B-Spline ON");
	else
		font->print(10,78, "<3> Single B-Spline OFF");

	if (knotVectorOpen)
		font->print(10,92, "<4> Single B-Spline Knot Vector OPEN");
	else
		font->print(10,92, "<4> Single B-Spline Knot Vector PERIODIC");

	font->print(10,106, "<-/+> Single B-Spline Order = %i", bSplineK);

	glColor3f(1,0,1);
	if (bSplineSeriesOn)
		font->print(10,120, "<5> Cubic B-Spline Series ON");
	else
		font->print(10,120, "<5> Cubic B-Spline Series OFF");

	glColor3f(0,0.5f,0);
	font->print(10,134, "<LEFT><RIGHT> Move Cubic B-Spline tangent");
	
	glColor3f(0,0,1);
	if (controlPointsOn)
		font->print(10,148, "<6> Control Points ON");
	else
		font->print(10,148, "<6> Control Points OFF");

	if (drawLines)
		font->print(10,162, "<7> Plot with LINES");
	else
		font->print(10,162, "<7> Plot with POINTS");

	font->print(10,176, "<ENTER> Recalculate Points");
}
