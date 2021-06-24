
#include<iostream>
#include<chrono>
#include<vector>
#include<algorithm>
using namespace std;

#include<Windows.h>

int nScreenHeight = 40;
int nScreenWidth = 120;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159f / 4.00;
float fDepth = 16.0;

int main() {
	// we will directly use the console buffer for faster printing etc
	// creating screen buffer
	wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#....###########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.........###..#";
	map += L"#.........###..#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"###########....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1) {

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) fPlayerA -=  fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) fPlayerA +=  fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5 * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5 * fElapsedTime;

			if (map[(int)((int)fPlayerY * nMapWidth + (int)fPlayerX)] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5 * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5 * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5 * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5 * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5 * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5 * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; x++) {
			float fRayAngle = (fPlayerA - fFOV / 2) + ((float)x / nScreenWidth) * fFOV;
			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			
			float vector_X = sinf(fRayAngle);
			float vector_Y = cosf(fRayAngle);
			
			while (!bHitWall && fDistanceToWall < fDepth) {
				fDistanceToWall += 0.1f;
				int test_point_x = (int)(fPlayerX + fDistanceToWall * vector_X);
				int test_point_y = (int)(fPlayerY + fDistanceToWall * vector_Y);
				
				//out of bounds condition
				if (test_point_x < 0 || test_point_x >= nMapWidth || test_point_y < 0 || test_point_y >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else {
					if (map[test_point_y * nMapWidth + test_point_x] == '#') {
						bHitWall = true;
						vector<pair<float, float>> p;	//distance, dot product of two vectors - used to identify boundary points
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								// for each cell idetified as a wall, checking 4 corners
								float vx = (float)(test_point_x + tx) - fPlayerX;
								float vy = (float)(test_point_y + ty) - fPlayerY;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (vy * vector_Y + vx * vector_X) / d;
								p.push_back(make_pair(d,dot));
							}
						}
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) { return left.first < right.first; });
						float fBound = 0.008;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;

					}
				}
			}
			int nCeiling = nScreenHeight / 2.0 - (float)nScreenHeight / (float)fDistanceToWall;
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588;
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = ' ';

			if (bBoundary)	nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCeiling) { screen[y * nScreenWidth + x] = ' '; }
				else if (y <= nFloor) { screen[y * nScreenWidth + x] = nShade; }
				else {
					float b = 1.0f - (y - nScreenHeight / 2.0f) / (nScreenHeight / 2.0f);
					if (b < 0.25)			nShade = '#';
					else if (b < 0.5)		nShade = 'x';
					else if (b < 0.75)		nShade = '.';
					else if (b < 0.9)		nShade = '-';
					else					nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapHeight; ny++) {
				screen[(ny+1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

		screen[nScreenHeight * nScreenWidth - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0,0 }, &dwBytesWritten);
	}
	return 0;
}

