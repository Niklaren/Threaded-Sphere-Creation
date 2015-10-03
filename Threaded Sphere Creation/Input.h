#pragma once

class Input
{
private:
	
public:
bool keys[256];
bool left, right, middle;
bool MouseMoved;
float MouseX, MouseY;
float oldMouseX, oldMouseY;
float XMoved, YMoved;
float distanceMoved;
char distance[20];

Input();
void calculatemove();
};