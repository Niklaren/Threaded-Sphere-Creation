#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <windows.h>
#include <fstream>
#include <conio.h>
#include <crtdbg.h>
#include <string>
#include <vector>
#include "guicon.h"
#include "Camera.h"
#include "Input.h"
#include "defines.h"
#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>

using namespace std;

//#define SPHERE_RADIUS	3
//#define CONVERT_TO_RAD	0.0174532925
#define PI 3.14159265368979323846

struct make_args
{
	int first_section, last_section;
};

bool make_sphere();
void build_sphere(make_args);
void translate_sphere(double x_, double y_, double z_, make_args makesphere_args);
void draw_sphere(void);

boost::thread_group buildgroup;
boost::thread_group movegroup;
boost::thread make_thread;
boost::mutex section_mutex[1000];

vector<bool> v_sections_built;
vector<bool> v_sections_moved;

bool draw_ok;
bool build_complete;

struct vertex
{
	double x, y, z;
};

int lon_sections;
int lat_sections;
vector< vector<vertex> > v_vertices;

#define COLOUR_DEPTH 16	//Colour depth

HWND        ghwnd;
HDC			ghdc;
HGLRC		ghrc;
HINSTANCE	ghInstance;
RECT		gRect;
RECT		screenRect;

Input input;
Camera camera;

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
void DrawScene();

void task_division(int N_threads, make_args makesphere_args[])
// calculates the first and last section a given thread must calculate
{
	int remainder = ((lon_sections+1) % N_threads);
	int tasks_per_thread = ((lon_sections + 1) / N_threads);

	int initial_section(0);
	for(int i = 0; i<N_threads; i++)
	{
		makesphere_args[i].first_section = initial_section;
		makesphere_args[i].last_section = initial_section + (tasks_per_thread-1);

		if(remainder>0) //split the remainder between threads until remainder is 0
		{
			makesphere_args[i].last_section++;
			remainder--;
		}

		initial_section = makesphere_args[i].last_section +1;
	}
}

void build_sphere(make_args makesphere_args)
// Code to fill the x, y & z values of the vertex array
{
	double latAngle(0); double lonAngle(0);
	
	for(int lon_section=makesphere_args.first_section; lon_section < makesphere_args.last_section+1; lon_section++)
	{
		section_mutex[lon_section].lock();
		lonAngle = ((2*PI) / lon_sections)*lon_section;
		for (int lat_section = 0; lat_section <= lat_sections; ++lat_section)
		{
			latAngle = ((PI) / lat_sections)*lat_section;
			v_vertices[lat_section][lon_section].x = sin(lonAngle)*sin(latAngle);
			v_vertices[lat_section][lon_section].y = cos(latAngle);
			v_vertices[lat_section][lon_section].z = cos(lonAngle)*sin(latAngle);
		}
		v_sections_built[lon_section]=true;
		Sleep(5);
		section_mutex[lon_section].unlock();
	}
}

bool check_remaining(make_args makesphere_args)
// check if there are remaining sections to be moved
{
	for (int lon_section = makesphere_args.first_section; lon_section < makesphere_args.last_section + 1; lon_section++)
	{
		if (v_sections_moved[lon_section] == false)
			return true;
	}
	return false;
}

void translate_sphere(double x_, double y_, double z_, make_args makesphere_args)
// moves every section of the sphere the given distance
{
	int lon_section(makesphere_args.last_section);
	while (check_remaining(makesphere_args) == true)
	{
		//cout << "trying " << lon_section << endl;
		//cout << v_sections_built[lon_section]<<v_sections_moved[lon_section]<<endl;
		if(section_mutex[lon_section].try_lock())
		{
			if((v_sections_built[lon_section]==true) && (v_sections_moved[lon_section]==false))
			{
				for (int lat_section = 0; lat_section<=lat_sections; ++lat_section)
				{
					v_vertices[lat_section][lon_section].x += x_;
					v_vertices[lat_section][lon_section].y += y_;
					v_vertices[lat_section][lon_section].z += z_;
				}
				v_sections_moved[lon_section]=true;
				//cout<<"section " << lon_section << " completed" << endl;
			}
			section_mutex[lon_section].unlock();
		}
		lon_section--;
		if (lon_section<makesphere_args.first_section)
			lon_section = makesphere_args.last_section;
		Sleep(5);
	}
	//cout << "translation complete" << endl;
	//build_complete = true;
}

void write_sphere_coords()
// outputs the entire vertex array
{
	ofstream myfile;
	myfile.open("sphere coords.txt");
	//for (unsigned lon_sec(0); lon_sec < v_vertices.size(); lon_sec++){
	//	for (unsigned lat_sec(0); lat_sec < v_vertices[0].size(); lat_sec++){
	for (int lon_sec = 0; lon_sec<(lon_sections); lon_sec++){
		for (int lat_sec = 0; lat_sec<(lat_sections); lat_sec++){
			myfile << "lon: " << lon_sec << " lat: " << lat_sec;
			myfile << " x: " << v_vertices[lat_sec][lon_sec].x;
			myfile << " y: " << v_vertices[lat_sec][lon_sec].y;
			myfile << " z: " << v_vertices[lat_sec][lon_sec].z;
			myfile << endl;
		}
	}
	myfile.close();
}

void draw_sphere()
// uses vertex array to render the sphere
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen And The Depth Buffer
	glLoadIdentity();// load Identity Matrix

	//set camera looking down the -z axis,  6 units away from the center
	gluLookAt(camera.x, camera.y, camera.z,     camera.forwardx, camera.forwardy, camera.forwardz,     camera.upx, camera.upy, camera.upz); //Where we are, What we look at, and which way is up
	
	glPushMatrix();
	glBegin(GL_QUADS);
	for (int lon = 0; lon < (lon_sections); lon++) //all the way round
	{
		for (int lat = 0; lat < (lat_sections /*/ 2*/); lat++) //top to bottom
		{
			glVertex3f(v_vertices[lat][lon].x, v_vertices[lat][lon].y, v_vertices[lat][lon].z);
			glVertex3f(v_vertices[lat + 1][lon].x, v_vertices[lat + 1][lon].y, v_vertices[lat + 1][lon].z);
			glVertex3f(v_vertices[lat + 1][lon + 1].x, v_vertices[lat + 1][lon + 1].y, v_vertices[lat + 1][lon + 1].z);
			glVertex3f(v_vertices[lat][lon + 1].x, v_vertices[lat][lon + 1].y, v_vertices[lat][lon + 1].z);
		}
	}
	glEnd();
	glPopMatrix();
	

	SwapBuffers(ghdc);// Swap the frame buffers.
}

bool make_sphere()
{
	vertex init_value; init_value.x = 0; init_value.y = 0; init_value.z = 0;

	do{
		std::cout << "how many longditudal sections?" << std::endl;
		std::cin >> lon_sections;
		if (lon_sections>1000 || lon_sections < 4)
			std::cout << "please between 4 & 1000" << std::endl;
	} while (lon_sections > 1000 || lon_sections < 4);

	do{
		std::cout << "how many latitudal sections?" << std::endl;
		std::cin >> lat_sections;
		if (lat_sections>1000 || lat_sections < 4)
			std::cout << "please between 4 & 1000" << std::endl;
	} while (lat_sections>1000 || lat_sections < 4);

	v_vertices.resize(lat_sections + 1, vector<vertex>(lon_sections + 1, init_value));
	v_sections_built.resize(lon_sections + 1, false);
	v_sections_moved.resize(lon_sections + 1, false);
	draw_ok = true;

	int build_threads;
	int move_threads;

	do{
		std::cout << "how many build threads to run?" << std::endl;
		std::cin >> build_threads;
		if (build_threads > lon_sections || build_threads>100 || build_threads < 1)
			std::cout << "choose a number less than the number of longditudal sections between 1 & 100" << std::endl;
	} while (build_threads > lon_sections || build_threads>100 || build_threads < 1);
	do{
		std::cout << "how many move threads to run?" << std::endl;
		std::cin >> move_threads;
		if (build_threads > lon_sections || build_threads>100 || build_threads < 1)
			std::cout << "choose a number less than the number of longditudal sections between 1 & 100" << std::endl;
	} while (move_threads > lon_sections || move_threads>100 || move_threads < 1);

	make_args buildsphere_args[100];// declare an array size 100, since we know there can be a maximum of 100 threads
	task_division(build_threads, buildsphere_args); //calculate the number of sections each thread will have to build
	make_args movesphere_args[100];
	task_division(move_threads, movesphere_args);

	double x_, y_, z_;

	std::cout << "translate x by?" << std::endl;
	std::cin >> x_;
	std::cout << "translate y by?" << std::endl;
	std::cin >> y_;
	std::cout << "translate z by?" << std::endl;
	std::cin >> z_;


	boost::timer::cpu_timer t1;

	for (int i(0); i<build_threads; i++)
		buildgroup.add_thread(new boost::thread(build_sphere, buildsphere_args[i]));

	boost::timer::cpu_timer t2;

	for (int i(0); i<move_threads; i++)
		movegroup.add_thread(new boost::thread(translate_sphere, x_, y_, z_, movesphere_args[i]));

	buildgroup.join_all(); //wait for all build threads to complete for timing purposes
	t1.stop();
	movegroup.join_all(); //wait for all build threads to complete for timing purposes
	t2.stop();

	build_complete = true;

	std::cout << "build time: " << t1.format() << '\n';
	std::cout << "move time: " << t2.format() << '\n';

	return true;
}

// The following functions are not pertinent to the threading portion of the task
// With the exception of the last WinMain, aka the main entry point for the program

HWND CreateOurWindow(LPSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance)
{
	HWND hwnd;

	WNDCLASS wcex;

	memset(&wcex, 0, sizeof(WNDCLASS));
	wcex.style			= CS_HREDRAW | CS_VREDRAW;		
	wcex.lpfnWndProc	= WndProc;		
	wcex.hInstance		= hInstance;						
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);; 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);		
	wcex.hbrBackground	= (HBRUSH) (COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;	
	wcex.lpszClassName	= "WindowClass";	

	
	RegisterClass(&wcex);// Register the class

	dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	ghInstance	= hInstance;// Assign our global hInstance to the window's hInstance

	//Set the Client area of the window to be our resolution.
	RECT glwindow;
	glwindow.left		= 0;		
	glwindow.right		= width;	
	glwindow.top		= 0;		
	glwindow.bottom		= height;	

	AdjustWindowRect( &glwindow, dwStyle, false);

	//Create the window
	hwnd = CreateWindow(	"WindowClass", 
							strWindowName, 
							dwStyle, 
							0, 
							0,
							glwindow.right  - glwindow.left,
							glwindow.bottom - glwindow.top, 
							NULL,
							NULL,
							hInstance,
							NULL
							);

	if(!hwnd) return NULL;// If we could get a handle, return NULL

	ShowWindow(hwnd, SW_SHOWNORMAL);	
	UpdateWindow(hwnd);					
	SetFocus(hwnd);						

	return hwnd;
}

bool SetPixelFormat(HDC hdc) 
{ 
    PIXELFORMATDESCRIPTOR pfd = {0}; 
    int pixelformat; 
 
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);	// Set the size of the structure
    pfd.nVersion = 1;							// Always set this to 1
	// Pass in the appropriate OpenGL flags
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
    pfd.dwLayerMask = PFD_MAIN_PLANE;			// standard mask (this is ignored anyway)
    pfd.iPixelType = PFD_TYPE_RGBA;				// RGB and Alpha pixel type
    pfd.cColorBits = COLOUR_DEPTH;				// Here we use our #define for the color bits
    pfd.cDepthBits = COLOUR_DEPTH;				// Ignored for RBA
    pfd.cAccumBits = 0;							// nothing for accumulation
    pfd.cStencilBits = 0;						// nothing for stencil
 
	//Gets a best match on the pixel format as passed in from device
    if ( (pixelformat = ChoosePixelFormat(hdc, &pfd)) == false ) 
    { 
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK); 
        return false; 
    } 
 
	//sets the pixel format if its ok. 
    if (SetPixelFormat(hdc, pixelformat, &pfd) == false) 
    { 
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK); 
        return false; 
    } 
 
    return true;
}

void ResizeGLWindow(int width, int height)// Initialize The GL Window
{
	if (height==0)// Prevent A Divide By Zero error
	{
		height=1;// Make the Height Equal One
	}

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//calculate aspect ratio
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height, 1 ,150.0f);

	glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
	glLoadIdentity();// Reset The Modelview Matrix
}

void InitializeOpenGL(int width, int height) 
{  
    ghdc = GetDC(ghwnd);//  sets  global HDC

    if (!SetPixelFormat(ghdc))//  sets  pixel format
        PostQuitMessage (0);


    ghrc = wglCreateContext(ghdc);	//  creates  rendering context from  hdc
    wglMakeCurrent(ghdc, ghrc);		//	Use this HRC.

	ResizeGLWindow(width, height);	// Setup the Screen
}

void Init(HWND hwnd)
{
	ghwnd = hwnd;
	GetClientRect(ghwnd, &gRect);	//get rect into our handy global rect
	InitializeOpenGL(gRect.right, gRect.bottom);// initialise openGL

	//OpenGL settings
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Also, do any other setting ov variables here for your app if you wish. 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Repeat texture with u/v 's over 1.0

}	

void keyboard_update()
{
	if(input.keys['Z'])
			{
				camera.MoveY(0.01f);
				camera.Update();
			}
			if(input.keys['X'])
			{
				camera.MoveY(-0.01f);
				camera.Update();
			}
			if(input.keys['A'])
			{
				camera.MoveLeft();
				camera.Update();
			}
			if(input.keys['D'])
			{
				camera.MoveRight();
				camera.Update();
			}
			if(input.keys['W'])
			{
				camera.MoveForward();
				camera.Update();
			}
			if(input.keys['S'])
			{
				camera.MoveBackward();
				camera.Update();
			}
			if(input.keys['Q'])
			{
				camera.LeanLeft();
				camera.Update();
			}
			else if(input.keys['E'])
			{
				camera.LeanRight();
				camera.Update();
			}
			else
			{
				camera.LeanCentre();
				camera.Update();
			}
			if(input.MouseMoved)
			{
				//camera.RotateX(input.XMoved/ 50);
				//camera.RotateY(input.YMoved/-50);
				//SetCursorPos(CENTRE_X,CENTRE_Y);
				camera.Update();
				input.MouseMoved=false;
			}
			if(input.keys[VK_SPACE])
			{
				camera.Reset();
				camera.Update();
			}
}

void Cleanup()
{
	if (ghrc)
	{
		wglMakeCurrent(NULL, NULL);	// free rendering memory
		wglDeleteContext(ghrc);		// Delete our OpenGL Rendering Context
	}

	if (ghdc) 
	ReleaseDC(ghwnd, ghdc);			// Release our HDC from memory

	UnregisterClass("FirstWindowClass", ghInstance);// Free the window class

	PostQuitMessage (0);		// Post a QUIT message to the window
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)											
    {														
		case WM_CREATE:	
			break;

		case WM_SIZE:
			//resize the open gl window when the window is resized
			ResizeGLWindow(LOWORD(lParam),HIWORD(lParam));
			GetClientRect(hwnd, &gRect);
			break;	

		case WM_KEYDOWN:
			input.keys[wParam]=true;
			break;

		case WM_KEYUP:
			input.keys[wParam]=false;
			break;

		case WM_MOUSEMOVE:
			input.MouseX = LOWORD (lParam);
			input.MouseY = HIWORD (lParam);
			input.calculatemove();
			input.MouseMoved = true;
			break;

		case WM_PAINT:
			
	
		    break;		

		case WM_DESTROY:	
			
			PostQuitMessage(0);	
								
			break;				
	}													

	return DefWindowProc (hwnd, message, wParam, lParam);		
															
}

// Main Function
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)			
{	
	HWND		hwnd;
    MSG         msg;	

	//initialise and create window
	hwnd = CreateOurWindow("Threaded Sphere Creation", S_WIDTH, S_HEIGHT, 0, false, hInstance);	
	if(hwnd == NULL) return true;

	//initialise opengl and other settings
	Init(hwnd);

	#ifdef _DEBUG
	RedirectIOToConsole();
	#endif

	draw_ok = false;
	build_complete = false;

	boost::thread make_thread(make_sphere);

	while (true)					
    {							
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
		    if (msg.message==WM_QUIT)
				break;
			TranslateMessage (&msg);							
			DispatchMessage (&msg);
		}
		//else if(make_thread.try_join_for(boost::chrono::milliseconds(1)))
		else if (build_complete)
		// if translate thread has finished
		{
			draw_sphere();	// draw one last time
			boost::thread write_thread(write_sphere_coords); // output position of every vertex
			write_thread.join();	// wait for the write thread to finish
			cin.ignore();
			cin.get();
			return msg.wParam ;		//then exit
		}
		else
		{		
			if (draw_ok){
				keyboard_update();
				draw_sphere();
			}
		}
    }


	return msg.wParam ;										
}



