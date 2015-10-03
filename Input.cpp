#include "Input.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "defines.h"

Input::Input()
{
	for(int i=0;i<256;i++)
	{
		keys[i]=false;
	}

	distanceMoved =0;
	left, right, middle = false;
	MouseMoved = false;
	MouseX, MouseY, oldMouseX, oldMouseY = 0;
}

void Input::calculatemove()
{
	XMoved = (MouseX - (CENTRE_X-8));
	YMoved = (MouseY - (CENTRE_Y-30));
	//if(XMoved < 0){XMoved*=-1;}
	//if(YMoved < 0){YMoved*=-1;}
	//distanceMoved += sqrt((XMoved*XMoved)+(YMoved*YMoved));
}