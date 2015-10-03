# Threaded-Sphere-Creation
Threading Project

A coursework project looking at threading and how it can be used to optimise efficiency, organise tasks and speed up program execution time.
I made a few improvements to just polish up how the program is, after completing the coursework, so some of the things written in the report may not perfectly match the current program, but I thought I'd keep the report/presentation for posterity.

The bulk of the code is in main.cpp with a few extra files for camera, input and console output. Though, as the program currently is input is not required.
The code then has the main purpose of building sections of a sphere and then translating them to a given value. To do this a thread is created with this task while the main program keeps updating the draw loop.
The thread tasked with creating the sphere creates threads of its own based on the users input of how many threads they wish the program to use. The program then creates the sphere (with the supplied number of make threads) and moves the sphere (with the supplied number of move threads).
The threads make use of mutexes to be sure that they do not try to access the same sphere section simultaneously.

The goal of this program is to demonstrate the usefulness and the limitations of using threading in programming.
