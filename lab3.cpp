//
//
//
// program: lab3.cpp
// author:  Gordon Griesel
// date written: 2014
// modified for game design: 2018, 2020
//
// This program demonstrates the use of OpenGL and XWindows
// It also demonstrates spawning a child process.
//
//
// Assignment:
//
//  Choose one or more.
//
//  1. Make the metronome work
//
//  2. Make hand shadows show on the clock's numeral spots
//
//  3. Make clock work without glRotate, glTranslate
//     Use matrix.cpp, matrix.h
//
//  4. Use a bit-field to store state variables use_metronome, use_usleep.
//
//  5. Clocks should be synchronized when the child process begins.
//     If not synchronized, apply some kind of timing method to make
//     the clocks start together.
//
//  6. Scale all clock visual components when window is resized.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "matrix.h"
#include <sys/wait.h>

typedef float Flt;
#define SWAP(x, y) \
	(x) ^= (y);    \
	(y) ^= (x);    \
	(x) ^= (y)

// Global variables
class Global
{
public:
	int xres, yres;
	float bigHand;
	float littleHand;
	float secondHand;
	float handIncrement;
	int nRevolutions;
	int child;
	bool use_metronome;
	bool use_usleep;
	unsigned int uPause;
	Global()
	{
		xres = 200;
		yres = 300;
		bigHand = 90.0;
		littleHand = 90.0;
		secondHand = 90.0;
		handIncrement = 6.0;
		nRevolutions = 0;
		child = 0;
		use_metronome = false;
		use_usleep = false;
		uPause = false;
	}
} g;

class X11_wrapper
{
private:
	Display *dpy;
	Window win;

public:
	X11_wrapper()
	{
		GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
		// GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		XSetWindowAttributes swa;
		setupScreenRes(g.xres, g.yres);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL)
		{
			printf("\n\tcannot connect to X server\n\n");
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL)
		{
			printf("\n\tno appropriate visual found\n\n");
			exit(EXIT_FAILURE);
		}
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
						 StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
							vi->depth, InputOutput, vi->visual,
							CWColormap | CWEventMask, &swa);
		GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
		setTitle();
	}
	~X11_wrapper()
	{
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void reshapeWindow(int width, int height)
	{
		// window has been resized.
		setupScreenRes(width, height);
		//
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glOrtho(0, g.xres, 0, g.yres, -1, 1);
		setTitle();
	}
	void checkResize(XEvent *e)
	{
		// Respond to ConfigureNotify.
		// Event sent by the server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		// Has window size changed?
		if (xce.width != g.xres || xce.height != g.yres)
		{
			reshapeWindow(xce.width, xce.height);
		}
	}
	void setTitle(void)
	{
		// Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "Synchronize the clocks");
	}
	void setupScreenRes(const int w, const int h)
	{
		g.xres = w;
		g.yres = h;
	}
	bool getXPending()
	{
		return XPending(dpy);
	}
	XEvent getXNextEvent()
	{
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers()
	{
		glXSwapBuffers(dpy, win);
	}
} x11;

// Function prototypes
void initOpengl(void);
void checkMouse(XEvent *e);
int checkKeys(XEvent *e);
void physics(void);
void render(void);

// Setup timers-------------------------------------------------------
struct timespec res;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown = 0.0;
double timeSpan = 0.0;
const double physicsRate = 1.0 / 30.0;
const double oobillion = 1.0 / 1e9;
double timeDiff(struct timespec *start, struct timespec *end)
{
	// return the time difference in seconds
	return (double)(end->tv_sec - start->tv_sec) +
		   (double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source)
{
	memcpy(dest, source, sizeof(struct timespec));
}
//-------------------------------------------------------------------

int main(int argc, char *argv[])
{
	initOpengl();
	initialize_fonts();
	// Debugging execl in fork call....
	//printf("argc: %i ", argc);
	//for (int i = 0; i < argc; i++)
	//	printf("argv[%i] **%s**\t", i, argv[i]);
	//printf("\n");
	if (argc > 1)
	{
		printf("argc: %i argv[0] **%s**\n", argc, argv[0]);
		if (*argv[0] == '1')
		{
			// This happens when a user presses X in a
			// running openglClock process.
			g.child = 1;
			// g.handIncrement = 360.0 * physicsRate;
			g.handIncrement = 180.0 * physicsRate;
			g.use_metronome = true;
			g.nRevolutions = 0;
		}
	} else {
		logOpen();
	}
	//
	// Display clock resolution...
	if (clock_getres(CLOCK_REALTIME, &res) < 0)
	{
		printf("clock_getres error!\n");
	}
	// printf("tv_sec: %li\n", res.tv_sec);
	// printf("tv_nsec: %li\n", res.tv_nsec)

	//
	// Important to init timers before starting loop!
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	//
	int done = 0;
	while (!done)
	{
		while (x11.getXPending())
		{
			XEvent e = x11.getXNextEvent();
			x11.checkResize(&e);
			checkMouse(&e);
			done = checkKeys(&e);
		}
		//
		// Apply physics at a consistent rate
		//
		// 1. Get the time right now. (done for you)
		clock_gettime(CLOCK_REALTIME, &timeCurrent);
		//
		// 2. How long since we were here last?
		timeSpan = timeDiff(&timeStart, &timeCurrent);
		//
		// 3. Save the current time as our new starting time.
		timeCopy(&timeStart, &timeCurrent);
		//
		// 4. Add time-span to our countdown amount.
		physicsCountdown += timeSpan;
		//
		// 5. Has countdown gone beyond our physics rate?
		//       if yes,
		//           In a loop...
		//              Apply physics
		//              Reducing countdown by physics-rate.
		//              Break when countdown < physics-rate.
		//       if no,
		//           Apply no physics this frame.
		//
		if (g.use_metronome)
		{
			while (physicsCountdown >= physicsRate)
			{
				// 6. Apply physics
				physics();
				// 7. Reduce the countdown by our physics-rate
				physicsCountdown -= physicsRate;
			}
			// This call to physics is temporary.
			// Remove it when metronome is working.
			// physics();
		}
		else
		{
			physics();
		}
		//
		// Always render every frame.
		render();
		x11.swapBuffers();
		//
		if (g.use_usleep)
		{
			//-------------------------------------------------------
			// This is just for testing to mix-up the program speed
			// Sleep for an unknown amount of time...
			timeSpan = timeDiff(&timePause, &timeCurrent);
			// printf("timeSpan: %lf\n", timeSpan);
			if (timeSpan > 2.123)
			{
				g.uPause = rand() % 200000;
				// printf("pause: %u\n", uPause);
				timeCopy(&timePause, &timeCurrent);
			}
			usleep(g.uPause);
			//-------------------------------------------------------
		}
	}
	cleanup_fonts();
	logClose();
	return 0;
}

void initOpengl(void)
{
	// OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	// init matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// This sets 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	// Clear the screen
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	// Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
}

void checkMouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	//
	if (e->type == ButtonRelease)
	{
		return;
	}
	if (e->type == ButtonPress)
	{
		if (e->xbutton.button == 1)
		{
		}
		if (e->xbutton.button == 3)
		{
		}
	}
	if (savex != e->xbutton.x || savey != e->xbutton.y)
	{
		// mouse moved
		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
}

void initMetronome(void)
{
	g.use_metronome = true;
	g.bigHand    = 90.0;
	g.littleHand = 90.0;
 	g.secondHand = 90.0;
	// g.handIncrement = 360.0 * physicsRate;
	g.handIncrement = 180.0 * physicsRate;
	g.nRevolutions = 0;
	physicsCountdown = 0.0;
	timeSpan = 0.0;
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
}

void startAnotherClock(void)
{
	Log("%f\n", g.bigHand);
	Log("%f\n", g.littleHand);
	Log("%f\n", g.secondHand);
	
	pid_t pid;
	pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork");
		break;
	case 0:
		execl("./lab3", "1", "2", (char *)0);
		perror("exec");
		break;
		// The following code is not needed here.
		// You might use some of this logic in the main game loop to
		// monitor the status of the child process.
		/*
			default:
			{
				int status;
				printf("Child id: %i\n", pid);
				log(getpid());
				fflush(NULL);

				if (waitpid(pid, &status, 0) != -1) {
					printf("Child exited with status %i\n", status);
				} else {
					perror("waitpid");
				}

				break;
			}
		*/
	}
}

int checkKeys(XEvent *e)
{
	static int shift = 0;
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyRelease)
	{
		if (key == XK_Shift_L || key == XK_Shift_R)
			shift = 0;
		return 0;
	}
	if (key == XK_Shift_L || key == XK_Shift_R)
	{
		shift = 1;
		return 0;
	}
	(void)shift;
	switch (key)
	{
	case XK_m:
		g.use_metronome = !g.use_metronome;
		if (g.use_metronome)
			initMetronome();
		break;
	case XK_u:
		g.uPause = 0;
		g.use_usleep ^= 1;
		printf("%i\n", g.use_usleep);
		fflush(stdout);
		break;
	case XK_x:
		if (g.child)
			break;
		g.child = 1;
		startAnotherClock();
		initMetronome();
		break;
	case XK_equal:
		g.handIncrement += 1.0f;
		break;
	case XK_minus:
		g.handIncrement -= 1.0f;
		break;
	case XK_Escape:
		return 1;
	}
	return 0;
}

void clockAnimation(void)
{
	// Draw a clock face
	// Dimensions are proportional to the width of the window.
	//
	int i;
	float ang;
	float len, wid, len2, len4;
	// x,y is clock center
	float x = g.xres / 2.0;
	float y = g.yres / 2.0 - 40.0;
	//Vec xyz = {x, y, 0.0};

	//
	// Numeral spots
	for (i = 0; i < 12; i++)
	{
		glPushMatrix();
		// translate(xyz,);
		glTranslatef(x, y, 0.0);
		glRotatef((360.0 / 12.0) * (float)i, 0.0, 0.0, 1.0);
		glTranslatef(g.xres / 4.0 + 20.0, 0.0, 0.0);
		glColor3ub(250, 200, 200);
		glBegin(GL_QUADS);
		glVertex2i(-12, -12);
		glVertex2i(-12, 12);
		glVertex2i(12, 12);
		glVertex2i(12, -12);
		glEnd();
		glPopMatrix();
	}

	// Draw clock hands
	//
	// second hand shadow
	len = (float)(g.xres / 4);
	wid = 1.0;
	len2 = len / 2.0;
	len4 = len / 4.0;
	glPushMatrix();
	glTranslatef(x + 8, y - 8, 0.0);
	glRotatef(g.secondHand, 0.0, 0.0, 1.0);
	glColor3ub(230, 230, 230);
	glBegin(GL_QUADS);
	glVertex2i(-len2, -wid);
	glVertex2i(-len2, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glVertex2i(-len4, -wid * 4);
	glVertex2i(-len4, wid * 4);
	glVertex2i(-len2, wid * 4);
	glVertex2i(-len2, -wid * 4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 5.0);
	glVertex2i(len + 24.0, 0.0);
	glVertex2i(len, wid * 5.0);
	glEnd();
	glPopMatrix();
	//
	// big hand shadow
	len = (float)(g.xres / 6);
	wid = 4.0;
	glPushMatrix();
	glTranslatef(x + 6, y - 6, 0.0);
	glRotatef(g.bigHand, 0.0, 0.0, 1.0);
	glColor3ub(230, 230, 230);
	glBegin(GL_QUADS);
	glVertex2i(0, -wid);
	glVertex2i(0, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 2.0);
	glVertex2i(len + 20.0, 0.0);
	glVertex2i(len, wid * 2.0);
	glEnd();
	glPopMatrix();
	//
	// little hand shadow
	len = (float)(g.xres / 10);
	wid = 4.0;
	glPushMatrix();
	glTranslatef(x + 4, y - 4, 0.0);
	glRotatef(g.littleHand, 0.0, 0.0, 1.0);
	glColor3ub(230, 230, 230);
	glBegin(GL_QUADS);
	glVertex2i(0, -wid);
	glVertex2i(0, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 2.5);
	glVertex2i(len + 10.0, 0.0);
	glVertex2i(len, wid * 2.5);
	glEnd();
	glPopMatrix();
	//
	//
	// clock face
	//
	//
	// big hand
	glColor3ub(60, 60, 60);
	glPushMatrix();
	len = (float)(g.xres / 5);
	wid = 4.0;
	glTranslatef(x, y, 0.0);
	glRotatef(g.bigHand, 0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glVertex2i(0, -wid);
	glVertex2i(0, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 2.0);
	glVertex2i(len + 20.0, 0.0);
	glVertex2i(len, wid * 2.0);
	glEnd();
	glPopMatrix();
	//
	// little hand
	glColor3ub(60, 60, 60);
	glPushMatrix();
	len = (float)(g.xres / 8);
	wid = 4.0;
	glTranslatef(x, y, 0.0);
	glRotatef(g.littleHand, 0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glVertex2i(0, -wid);
	glVertex2i(0, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 2.5);
	glVertex2i(len + 10.0, 0.0);
	glVertex2i(len, wid * 2.5);
	glEnd();
	glPopMatrix();
	//
	// second hand
	glColor3ub(200, 60, 90);
	glPushMatrix();
	len = (float)(g.xres / 4);
	wid = 1.0;
	glTranslatef(x, y, 0.0);
	glRotatef(g.secondHand, 0.0, 0.0, 1.0);
	// glScalef(scale,1,1);
	glBegin(GL_QUADS);
	glVertex2i(-len2, -wid);
	glVertex2i(-len2, wid);
	glVertex2i(len, wid);
	glVertex2i(len, -wid);
	glVertex2i(-len4, -wid * 4);
	glVertex2i(-len4, wid * 4);
	glVertex2i(-len2, wid * 4);
	glVertex2i(-len2, -wid * 4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2i(len, -wid * 5.0);
	glVertex2i(len + 24.0, 0.0);
	glVertex2i(len, wid * 5.0);
	glEnd();
	glPopMatrix();
	//
	// dot at center of clock
	glColor3ub(250, 60, 90);
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glBegin(GL_TRIANGLE_FAN);
	for (i = 0; i < 12; i++)
	{
		ang = (float)i * (PI / 6.0);
		glVertex2f(cos(ang) * 10.0f, sin(ang) * 10.0f);
	}
	glEnd();
	glPopMatrix();
}

void physics(void)
{
	g.secondHand -= g.handIncrement;
	if (g.secondHand <= -360.0)
	{
		g.nRevolutions++;
		g.secondHand += 360.0;
	}
	g.bigHand -= g.handIncrement / 60.0;
	g.littleHand -= g.handIncrement / 60.0 / 12.0;
}

void render(void)
{
	Rect r;
	glClear(GL_COLOR_BUFFER_BIT);
	clockAnimation();
	r.left = 8;
	r.bot = g.yres - 30;
	r.center = 0;
	ggprint16(&r, 24, 0x00990000, "Metronome: %s",
			  (g.use_metronome) ? "ON" : "OFF");
	ggprint16(&r, 24, 0, "Step: %3.0lf", g.handIncrement);
	ggprint16(&r, 24, 0, "usleep(%u)", g.uPause);
}
