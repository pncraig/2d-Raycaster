/*
* Created 1/23/2021 at 2:08 PM
* 
*/

#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <sstream>
#include <cmath>
#include <chrono>
using namespace std;

#define PI 3.1415926535

// Variables inportant to the console
int nScreenWidth = 192;
int nScreenHeight = 96;
float FPS;

int nMapWidth = 40;
int nMapHeight = 20;

// Player variables
int FOV = 60;
float fPlayerX = 9.6f;
float fPlayerY = 4.2f;
float fPlayerA = 0.0f;

// Player characteristics
float fPlayerVel = 5.0f;
float fPlayerTurnSpeed = 43.5f;
// Represents how many delta steps are taken before hit testing stops
int nPlayerRadius = 3;

// The length of the step in between ray-wall checks 
float deltaStep = 0.08f;

// Light variables
const int nNumberOfLights = 3;
// Array of lights (always should have a .5 in order to increase the accuracy of rays)
float lights[nNumberOfLights][2] = { {10.5f, 3.5f}, { 10.5f, 17.5f }, { 30.5f, 13.5f } };
// Makes it easier for me to find the lightest shade because each shading value now has a corresponding integer value,
// so I can compare shades to finding the darkest shading
enum shade {
	light,
	medium,
	dark,
	full
};
// Holds the possible shades for easy access
wchar_t shades[4] = { L'\u2591', L'\u2592', L'\u2593', L'\u2588' };

// Keeps an angle in the range [0 - 360)
float loopAngle(float angle);

// Keeps a value in a certain range
float constrain(float x, float lo, float hi);

// Convert from radians to degrees
float degrees(float radians);
// Convert from degrees to radians
float radians(float degrees);

// Struct which holds texture information
struct texture {
	wchar_t character{};	// Character representation on the map
	int width{};
	int height{};
	wstring textureMap{};
};

int main()
{	// << I'm trying this out because I think it looks clean

	// Create an array of CHAR_INFO structures and a screen buffer. CHAR_INFO members include a union which holds character
	// info and a Attributes member which holds the character attributes
	CHAR_INFO* screen = new CHAR_INFO[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	
	// Set the size of the screen to the acceptable parameters to prevent strange glitches (doesn't work right now)
	SMALL_RECT srWindowInfo = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &srWindowInfo);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth, (short)nScreenHeight });

	// Set the characters to be the desired size
	CONSOLE_FONT_INFOEX cfiConsoleFont;
	cfiConsoleFont.cbSize = sizeof(cfiConsoleFont);
	cfiConsoleFont.dwFontSize = { 5, 5 };
	cfiConsoleFont.nFont = 0;
	cfiConsoleFont.FontFamily = FF_DONTCARE;
	cfiConsoleFont.FontWeight = 400;
	wcscpy_s(cfiConsoleFont.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfiConsoleFont);

	// Initialize two timepoints
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// Define my textures
	/*
											#==============================#
											|		Texturing Table:	   |
											|	- = no color			   |
											|	a = red					   |
											|	b = blue				   |
											|	c = green				   |
											|	d = purple				   |
											|	e = yellow				   |
											|	f = light blue			   |
											|	g = pink				   |
											|	h = light green			   |
											|	i = pastel				   |
											|	j = cyan				   |
											|	k = white				   |
											#==============================#
	*/

	// Variables which associate a color with a character
	const char cNoColor = '-';
	const char cRed = 'a';
	const char cBlue = 'b';
	const char cGreen = 'c';
	const char cPurple = 'd';
	const char cYellow = 'e';
	const char cLightBlue = 'f';
	const char cPink = 'g';
	const char cLightGreen = 'h';
	const char cPastel = 'i';
	const char cCyan = 'j';
	const char cWhite = 'k';

	// Simple red brick texture
	texture brickTexture;
	brickTexture.character = L'#';
	brickTexture.width = 10;
	brickTexture.height = 10;
	brickTexture.textureMap += L"kaaakkaaak";
	brickTexture.textureMap += L"kaaakkaaak";
	brickTexture.textureMap += L"kkkkkkkkkk";
	brickTexture.textureMap += L"akkaaakkaa";
	brickTexture.textureMap += L"akkaaakkaa";
	brickTexture.textureMap += L"kkkkkkkkkk";
	brickTexture.textureMap += L"kaaakkaaak";
	brickTexture.textureMap += L"kaaakkaaak";
	brickTexture.textureMap += L"kkkkkkkkkk";
	brickTexture.textureMap += L"akkaaakkaa";

	// A smiley face set in a white background
	texture smileyTexture;
	smileyTexture.character = L'%';
	smileyTexture.width = 10;
	smileyTexture.height = 10;
	smileyTexture.textureMap += L"kkkkkkkkkk";
	smileyTexture.textureMap += L"kkkkkkkkkk";
	smileyTexture.textureMap += L"kkkdkkdkkk";
	smileyTexture.textureMap += L"kkkdkkdkkk";
	smileyTexture.textureMap += L"kkkdkkdkkk";
	smileyTexture.textureMap += L"kkkkkkkkkk";
	smileyTexture.textureMap += L"kckkkkkkck";
	smileyTexture.textureMap += L"kkckkkkckk";
	smileyTexture.textureMap += L"kkkcccckkk";
	smileyTexture.textureMap += L"kkkkkkkkkk";

	// The letter "P". Notice how this texture is larger than the other: the raycaster can handle any texture size
	texture letterTexture;
	letterTexture.character = L'&';
	letterTexture.width = 15;
	letterTexture.height = 15;
	letterTexture.textureMap += L"iiiiiiiiiiiiiii";
	letterTexture.textureMap += L"iibbbbbbbiiiiii";
	letterTexture.textureMap += L"iibbiiiibbiiiii";
	letterTexture.textureMap += L"iibbiiiiibbiiii";
	letterTexture.textureMap += L"iibbiiiiiibiiii";
	letterTexture.textureMap += L"iibbiiiiiibiiii";
	letterTexture.textureMap += L"iibbiiiiiibiiii";
	letterTexture.textureMap += L"iibbiiiiibbiiii";
	letterTexture.textureMap += L"iibbiiiibbiiiii";
	letterTexture.textureMap += L"iibbbbbbbiiiiii";
	letterTexture.textureMap += L"iibbiiiiiiiiiii";
	letterTexture.textureMap += L"iibbiiiiiiiiiii";
	letterTexture.textureMap += L"iibbiiiiiiiiiii";
	letterTexture.textureMap += L"iibbiiiiiiiiiii";
	letterTexture.textureMap += L"iibbiiiiiiiiiii";

	// Create an array of textures
	const int nNumberOfTextures = 3;
	texture textures[nNumberOfTextures] = { brickTexture, smileyTexture, letterTexture };

	// Create and fill a map
	wstring map;
	map += L"%%%%%%%%%%%%%%%%%%%%####################";
	map += L"#..................##..................#";
	map += L"#..###........###..##..................#";
	map += L"#..###........###..##..................#";
	map += L"#..###........###..##..................#";
	map += L"#..................##..................#";
	map += L"#......................................#";
	map += L"#..........................###.###.....#";
	map += L"#..................##......#.....#.....#";
	map += L"#########......######......#######.....#";
	map += L"#..................##......#%%%%%#.....#";
	map += L"#..................##......#.....#.....#";
	map += L"#..................##......#.....#.....#";
	map += L"#....####..####....##......#.....#.....#";
	map += L"#....#........#....##......#.....#.....#";
	map += L"#....#........#....##......#.....#.....#";
	map += L"#....&&&&&&&&&&....##..................#";
	map += L"#..................##..................#";
	map += L"#..................##..................#";
	map += L"########################################";

	//map += L"####################";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#......#######.....#";
	//map += L"#......#######.....#";
	//map += L"#......#######.....#";
	//map += L"#......#######.....#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"#..................#";
	//map += L"####################";

	// Place characters which represent the lights on the map
	for (int i = 0; i < nNumberOfLights; i++)
	{
		map[(int)lights[i][1] * nMapWidth + (int)lights[i][0]] = L'O';
	}

	// Infinite loop!
	for (;;)
	{
		// Clear the characters and colors of the last frame
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
		{
			// Color the sky blue and the ground green
			int backgroundColor = 0;
			if (i < int(nScreenWidth * nScreenHeight / 2))
				backgroundColor = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
			else
				backgroundColor = BACKGROUND_GREEN;

			screen[i].Char.UnicodeChar = L' ';
			screen[i].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | backgroundColor;
		}

		//  Determine how much time has elapsed between frames
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Update the frames per second
		FPS = 1.0f / fElapsedTime;

		// Turn the player
		if (GetAsyncKeyState((unsigned)'A') & 0x8000)
			fPlayerA -= float(fPlayerTurnSpeed * fElapsedTime);
		if (GetAsyncKeyState((unsigned)'D') & 0x8000)
			fPlayerA += float(fPlayerTurnSpeed * fElapsedTime);

		// Keeps the player angle from getting too large
		fPlayerA = loopAngle(fPlayerA);

		// Determines what the x and y steps are for the player based on the angle of the player and their velocity
		float playerVelX = fPlayerVel * cosf(radians(fPlayerA)) * fElapsedTime;
		float playerVelY = fPlayerVel * sinf(radians(fPlayerA)) * fElapsedTime;

		// Move player forward and backwards
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

		/*
												#====================#
												|					 |
												|	   Collide		 |
												|					 |
												#====================#
		*/


		// I'd like to figure out how to improve this collision system
		// Variables that will be filled with info from the ray with the longest length
		float maxDistance = -FLT_MAX;
		float intersectX = fPlayerX;
		float intersectY = fPlayerY;
		// Cast a ray out in a circle around the player
		for (int angle = 0; angle < 360; angle += 20)
		{
			bool missedWall = true;
			
			// Get the individual components of the ray slope
			float deltaX = deltaStep * cosf(radians((float)angle));
			float deltaY = deltaStep * sinf(radians((float)angle));

			// Copy of the player coordinates
			float stepX = fPlayerX;
			float stepY = fPlayerY;

			// Instead of running until the ray hits a wall, run until the ray is outside of the player radius
			for (float i = 0.0f; i < (float)nPlayerRadius * deltaStep; i += deltaStep)
			{
				stepX += deltaX;
				stepY += deltaY;

				// If the ray hits a wall unflag that it missed a wall
				if (map[(int)stepY * nMapWidth + (int)stepX] != '.' && map[(int)stepY * nMapWidth + (int)stepX] != 'O')
				{
					missedWall = false;
					break;
				}
			}
			
			if (missedWall)
				continue;

			// Calculate distance to intersection point
			float a = fPlayerX - stepX;
			float b = fPlayerY - stepY;
			// No need to correct the distance for collisions
			float distance = sqrtf(a * a + b * b);

			// Check if the ray is the one which will be used to correct the player position
			if (distance > maxDistance)
			{
				maxDistance = distance;
				intersectX = stepX;
				intersectY = stepY;
			}
		}

		if (intersectX != fPlayerX && intersectY != fPlayerY)
		{
			// Move the player out of the wall. Multiply by a decimal to smooth out the movement
			fPlayerX += float((fPlayerX - intersectX) * 0.5);
			fPlayerY += float((fPlayerY - intersectY) * 0.5);
		}
		
		// Just in case the above method fails, prevents the player from going out of bounds
		fPlayerX = constrain(fPlayerX, 0.0f, (float)nMapWidth - 1.0f);
		fPlayerY = constrain(fPlayerY, 0.0f, (float)nMapHeight - 1.0f);
		
		/*
												#====================#
												|					 |
												|		Render		 |
												|					 |
												#====================#
		*/
		// Cast a ray for each column on screen
		for (int x = 0; x < nScreenWidth; x++)
		{
			// Determine the angle of the ray by moving to the far left of the screen, and moving the angle forward by the 
			// field of view divided by the width, to the get the change for an increase of 1 in x, times x, to get the 
			// angle increase for the current x
			float rayAngle = fPlayerA - ((float)FOV / 2) + ((float)FOV / (float)nScreenWidth * (float)x);
			bool hitWall = false;
			bool blank = false;

			// Find the x and y step lengths based on the length of a step use soh cah toa
			float deltaX = deltaStep * cosf(radians(rayAngle));
			float deltaY = deltaStep * sinf(radians(rayAngle));

			// Copy the player coordinates to new variables
			float stepX = fPlayerX;
			float stepY = fPlayerY;

			// Stores the texture of the wall type
			texture* sampleTexture{};

			// If a wall is hit, end loop. Otherwise, keep adding to the stepX and stepY coordinates to find a wall
			while (!hitWall)
			{
				stepX += deltaX;
				stepY += deltaY;

				// The ray is intersecting a wall
				if (map[(int)stepY * nMapWidth + (int)stepX] != '.' && map[(int)stepY * nMapWidth + (int)stepX] != 'O')
				{
					// If the ray-wall intersection point is in a corner, flag the wall to be blank
					if (((double)stepX - (int)stepX > 0.92 && (double)stepY - (int)stepY > 0.92) || 
						((double)stepX - (int)stepX < 0.08 && (double)stepY - (int)stepY < 0.08) ||
						((double)stepX - (int)stepX < 0.08 && (double)stepY - (int)stepY > 0.92) ||
						((double)stepX - (int)stepX > 0.92 && (double)stepY - (int)stepY < 0.08))
					{
						// blank = true;
					}

					for (int i = 0; i < nNumberOfTextures; i++)
					{
						if (textures[i].character == map[(int)stepY * nMapWidth + (int)stepX])
						{
							sampleTexture = &textures[i];
							break;
						}
					}

					hitWall = true;
				}
			}

			// Find the distance between the player and the wall
			float a = fPlayerX - stepX;
			float b = fPlayerY - stepY;
			float distance = sqrtf(a * a + b * b);
			// The multiplication makes the distance what it would've been if the grid squares were ten units wide instead of one
			// Completely fixes distortion and makes the result just look better
			distance *= 10;

			// Correct the fish eye effect
			float correctedDistance = distance * cosf(radians(fPlayerA - rayAngle));

			// Calculates the height of the wall, the *15 is the distance to the projection plane
			float sliceHeight = (nScreenHeight / correctedDistance) * 15;
			int ceilingGap = int((nScreenHeight - sliceHeight) / 2);
			int savedCeilingGap = ceilingGap;

			// If the ceiling gap is less than zero, the slice takes up more than the whole screen
			if (ceilingGap < 0)
				ceilingGap = 0;
			

			// If the column should be blank, make it blank
			if (blank)
				ceilingGap = nScreenWidth / 2;

			// The purpose of the lines until the // end is to lessen visual artifacts by shifting the intersection point to the
			// edge that it actually intersected
			// This fixes the issue by insuring rays which double back on themselves make it out of the wall

			// Used to determine whether I use the x or y as the normX value. We want the coordinate which is not moved closer 
			// to the edge
			bool useXOffset = false;
			bool useYOffset = false;

			// Distance from the x intersection value and the left wall of the grid it is in
			float xDistanceFromLeft = stepX - (int)stepX;
			// Distance from the x intersection value and the right wall of the grid it is in
			float xDistanceFromRight = 1 - xDistanceFromLeft;
			// The distance that we will consider as the definitive x distance is the smallest distance
			float xDistance;
			if (xDistanceFromLeft < xDistanceFromRight)
				xDistance = xDistanceFromLeft;
			else
				xDistance = xDistanceFromRight;

			// Distance from the y intersection value to the top wall of its current grid
			float yDistanceFromTop = stepY - (int)stepY;
			// Distance from the y intersection value to the bottom wall of its current grid
			float yDistanceFromBottom = 1 - yDistanceFromTop;
			// The distance which we will consider the definitive y distance is the smallest distance
			float yDistance;
			if (yDistanceFromTop < yDistanceFromBottom)
				yDistance = yDistanceFromTop;
			else
				yDistance = yDistanceFromBottom;

			// Determine which coordinate is the closest to the edge and shift it towards that edge
			if (xDistance < yDistance)
			{
				stepX = roundf(stepX);
				useYOffset = true;
			}
			else
			{
				stepY = roundf(stepY);
				useXOffset = true;
			}

			//end

			/*
			* The algorithm behind the light casting is this: 
			* 1. Determine the angle between the intersection point of the current slice of the wall and the light source
			* 2. Cast a ray towards the light source
			* 3. If the ray hits a wall before it hits a light source, skip all calculations. Otherwise, continue casting the ray
			* 4. If the ray hits a light source before a wall, find the distance between the intersection point and the wall
			*    and color the wall based on the distance
			*/

			// The character the slice will be drawn as and the current shade of the slice
			wchar_t wShade;
			shade sShade = light;

			for (int i = 0; i < nNumberOfLights && !blank; i++)
			{
				bool foundWall = false;
				shade newShade;

				// Calculate the x and y distances from the light source to the intersection point
				// adjacent = x, opposite = y
				float adjacent = lights[i][0] - stepX;
				float opposite = lights[i][1] - stepY;

				// This is the angle between the slice and the light source
				// We'll leave it in radians to make future calculations easier
				// tan(theta) = opposite / adjacent
				// theta = atan(opposite / adjacent)
				float lightRayAngle = atan2f(opposite, adjacent);

				// Get the individual changes in steps
				float lightDeltaX = deltaStep * cosf(lightRayAngle);
				float lightDeltaY = deltaStep * sinf(lightRayAngle);

				// Copy the point where a ray from the player intersected the level geometry
				float lightStepX = stepX;
				float lightStepY = stepY;

				// Cast the ray while a wall hasn't been found
				while (!foundWall)
				{
					lightStepX += lightDeltaX;
					lightStepY += lightDeltaY;

					// If the ray hits a light source, break the loop, but don't flag anything because we need to perform 
					// calculations to shade this slice
					if (map[(int)lightStepY * nMapWidth + (int)lightStepX] == 'O')
					{
						break;
					}

					// If the ray hits a wall, flag that it has hit a wall so we can skip calculations
					if (map[(int)lightStepY * nMapWidth + (int)lightStepX] != '.')
					{
						foundWall = true;
					}
				}

				// If the ray hit a wall before a light source, skip the calculations
				if (foundWall)
				{
					continue;
				}

				// Calculate the distance between the light source and the intersection point of a ray cast from the player
				// using values computed earlier
				float distance = sqrtf(opposite * opposite + adjacent * adjacent);

				// Determine how the slice should be shaded using exclusively this ray
				if (distance < 4.5f)
					newShade = full;
				else if (distance < 9.0f)
					newShade = dark;
				else if (distance < 14.0f)
					newShade = medium;
				else
					newShade = light;

				// Check if the shade computed by this ray is the darkest. If so, make it the actual shade of the slice
				if (newShade > sShade)
					sShade = newShade;
			}

			// Use the fact that enums are really just integers to look up the correct shading value
			wShade = shades[(int)sShade];

			// Fill the column with a slice of the appropiate height
			for (int y = ceilingGap; y < nScreenHeight - ceilingGap; y++)
			{
				// The coordinates of the points on a wall in the range 0 - 1
				float normX = 0.0f;
				// Translate the y so that it is equal to 0, then normalize using the slice height. The plus one fixed
				// the accessing a string index that doesn't exist error
				float normY = (y - savedCeilingGap) / (sliceHeight + 1.0f);

				// Because texturing is based on what the player sees (meaning a pseudo 3d wall), we need to decide whether to use
				// the x or y coordinates of the intersection point to determine the texture column to use
				if (useXOffset)
					normX = stepX - (int)stepX;
				else if (useYOffset)
					normX = stepY - (int)stepY;

				// The to be color of the current character cell
				int color = 0;

				// Use the modulus operator to determine the x of the texture sample
				int sampleX = int(normX * sampleTexture->width);
				int sampleY = int(normY * sampleTexture->height);
				
				// The character at the sampled texture coords
				wchar_t sampledCoords;
				
				// Failsafe to prevent the program from accessing a character value that doesn't exist
				int textureIndex = sampleY * sampleTexture->width + sampleX;
				if (textureIndex >= sampleTexture->width * sampleTexture->height)
					sampledCoords = L'c';
				else
					sampledCoords = sampleTexture->textureMap[textureIndex];

				// Find the matching color for the texture sample
				switch (sampledCoords)
				{
					case cNoColor:
						color = -1;
						break;
					case cRed:
						color = FOREGROUND_RED;
						break;
					case cBlue:
						color = FOREGROUND_BLUE;
						break;
					case cGreen:
						color = FOREGROUND_GREEN;
						break;
					case cPurple:
						color = FOREGROUND_RED | FOREGROUND_BLUE;
						break;
					case cYellow:
						color = FOREGROUND_RED | FOREGROUND_GREEN;
						break;
					case cLightBlue:
						color = FOREGROUND_BLUE | FOREGROUND_GREEN;
						break;
					case cPink:
						color = FOREGROUND_RED | FOREGROUND_INTENSITY;
						break;
					case cLightGreen:
						color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
						break;
					case cPastel:
						color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
						break;
					case cCyan:
						color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
						break;
					case cWhite:
						color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;
						break;
				}

				// If there should be no color, skip coloring the character
				if (color < 0)
					continue;

				// Color the character and update its unicode character
				screen[y * nScreenWidth + x].Attributes = color;
				screen[y * nScreenWidth + x].Char.UnicodeChar = wShade;
			}
		}

		// Saves the previous player character so that after displaying the map I can erase the previous player position
		wchar_t playerPrevCharacter = map[(int)fPlayerY * nMapWidth + (int)fPlayerX];
		map[(int)fPlayerY * nMapWidth + (int)fPlayerX] = L'P';

		// Draw map
		for (int x = 0; x < nMapWidth; x++)
		{
			for (int y = 0; y < nMapHeight; y++)
			{
				screen[y * nScreenWidth + x].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				screen[y * nScreenWidth + x].Char.UnicodeChar = map[y * nMapWidth + x];
			}
		}

		// Reset map
		map[(int)fPlayerY * nMapWidth + (int)fPlayerX] = playerPrevCharacter;

		// Set the console title to display the framerate
		wchar_t consoleTitle[100];
		swprintf_s(consoleTitle, 100, L"Console Raycaster | FPS = %f", FPS);
		SetConsoleTitle(consoleTitle);

		// Write the array of CHAR_INFO structures to the screen buffer
		SMALL_RECT window = { 0, 0, (short)nScreenWidth, (short)nScreenHeight };
		WriteConsoleOutput(hConsole, screen, { (short)nScreenWidth, (short)nScreenHeight }, { 0, 0 }, &window);
	}

	return 0;
}

float loopAngle(float angle)
{
	if (angle > 360)
		return angle - 360;
	else if (angle <= 0)
		return angle + 360;
	else return angle;
}

float constrain(float x, float lo, float hi)
{
	if (x < lo)
		return lo;
	else if (x > hi)
		return hi;
	else return x;
}

float degrees(float radians)
{
	return float(radians * (180 / PI));
}

float radians(float degrees)
{
	return float((degrees * PI) / 180);
}