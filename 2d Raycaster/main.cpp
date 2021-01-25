/*
* Created 1/23/2021 at 2:08 PM
* 
*/

#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <cmath>
#include <chrono>
using namespace std;

#define PI 3.1415926535

int nScreenWidth = 120;
int nScreenHeight = 60;

int nMapWidth = 20;
int nMapHeight = 20;

// Player variables
int FOV = 90;
float fPlayerX = 1.0f;
float fPlayerY = 1.0f;
float fPlayerA = 0.0f;

float fPlayerVel = 0.005f;
float fPlayerTurnSpeed = 17.5f;

// The length of the step in between ray-wall checks 
float deltaStep = 0.08f;

// Convert from radians to degrees
float degrees(float radians);
// Convert from degrees to radians
float radians(float degrees);

int main()
{	// << I'm trying this out because I think it looks clean
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	
	// Set the size of the screen to the acceptable parameters to prevent strange glitches (doesn't work right now)
	SMALL_RECT srWindowInfo = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &srWindowInfo);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth, (short)nScreenHeight });

	// Set the characters to be 8 by 8
	CONSOLE_FONT_INFOEX cfiConsoleFont;
	cfiConsoleFont.cbSize = sizeof(cfiConsoleFont);
	cfiConsoleFont.dwFontSize = { 8, 8 };
	cfiConsoleFont.nFont = 0;
	cfiConsoleFont.FontFamily = FF_DONTCARE;
	cfiConsoleFont.FontWeight = 400;
	wcscpy_s(cfiConsoleFont.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfiConsoleFont);

	// Initialize two timepoints
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	wstring map;
	map += L"####################";
	map += L"#..................#";
	map += L"#..###........###..#";
	map += L"#..###........###..#";
	map += L"#..###........###..#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#########......#####";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#....####..####....#";
	map += L"#....#........#....#";
	map += L"#....#........#....#";
	map += L"#....##########....#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"####################";

	/*map += L"####################";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#......#######.....#";
	map += L"#......#######.....#";
	map += L"#......#######.....#";
	map += L"#......#######.....#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"####################";*/

	for (;;)
	{
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
			screen[i] = ' ';

		//  Determine how much time has elapsed between frames
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Turn the player
		if (GetAsyncKeyState((unsigned)'A') & 0x8000)
			fPlayerA -= float(fPlayerTurnSpeed * fElapsedTime);
		if (GetAsyncKeyState((unsigned)'D') & 0x8000)
			fPlayerA += float(fPlayerTurnSpeed * fElapsedTime);

		// Determines what the x and y steps are for the player based on the angle of the player and their velocity
		float playerVelX = fPlayerVel * cosf(radians(fPlayerA));
		float playerVelY = fPlayerVel * sinf(radians(fPlayerA));

		if (GetAsyncKeyState((unsigned)'W') & 0x8000)
		{
			fPlayerX += playerVelX;
			fPlayerY += playerVelY;
		}

		if (GetAsyncKeyState((unsigned)'S') & 0x8000)
		{
			fPlayerX -= playerVelX;
			fPlayerY -= playerVelY;
		}

		// Run for each column on screen
		for (int x = 0; x < nScreenWidth; x++) 
		{
			// Determine the angle of the ray 
			float rayAngle = fPlayerA - ((float)FOV / 2) + ((float)x / (float)nScreenWidth) * (float)FOV;
			bool hitWall = false;

			// Find the x and y steps based on the length of a step
			float deltaX = deltaStep * cosf(radians(rayAngle));
			float deltaY = deltaStep * sinf(radians(rayAngle));

			// Copy the player coordinates to new variables
			float stepX = fPlayerX;
			float stepY = fPlayerY;

			// If a wall is hit, end loop. Otherwise, keep adding to the stepX and stepY coordinates to find a wall
			while (!hitWall)
			{
				if (map[(int)stepY * nMapWidth + (int)stepX] == '#')
				{
					hitWall = true;
					break;
				}

				stepX += deltaX;
				stepY += deltaY;
			}

			// Find the distance between the player and the wall
			float a = fPlayerX - stepX;
			float b = fPlayerY - stepY;
			float distance = sqrtf(a * a + b * b);

			// Correct the distance
			float correctedDistance = distance * cosf(radians(fPlayerA - rayAngle));

			float sliceHeight = (nScreenHeight / (correctedDistance + 1));
			sliceHeight *= FOV / nScreenHeight;

			int ceilingGap = (nScreenHeight - sliceHeight) / 2;

			wchar_t wShade;

			if (distance < 3.0)
				wShade = L'\u2588';
			else if (distance < 8.0)
				wShade = L'\u2593';
			else if (distance < 14.0)
				wShade = L'\u2592';
			else
				wShade = L'\u2591';

			for (int y = ceilingGap; y < nScreenHeight - ceilingGap; y++)
			{
				screen[y * nScreenWidth + x] = wShade;
			}
		}


		wchar_t playerPrevCharacter = map[(int)fPlayerY * nMapWidth + (int)fPlayerX];
		map[(int)fPlayerY * nMapWidth + (int)fPlayerX] = L'P';

		for (int x = 0; x < nMapWidth; x++)
		{
			for (int y = 0; y < nMapHeight; y++)
			{
				screen[y * nScreenWidth + x] = map[y * nMapWidth + x];
			}
		}

		map[(int)fPlayerY * nMapWidth + (int)fPlayerX] = playerPrevCharacter;

		screen[nScreenWidth * nScreenHeight] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}

float degrees(float radians)
{
	return radians * (180 / PI);
}

float radians(float degrees)
{
	return (degrees * PI) / 180;
}