#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <sddl.h>
#include <stdlib.h>
#include <conio.h>
//#include "resource.rc"


#define KEY_A 'A'
#define KEY_S 'S'
#define KEY_D 'D'
#define KEY_F 'F'

#define MAX_TSTAMP 100
#define M_ROW 10
#define N_ROW 20

#define IDB_BITMAP1 101
#define READ_NOTE_MIL 2000
#define Start_Pos 200

HWND hWnd = NULL;
HINSTANCE hInst = NULL;

typedef struct
{
	char AudioFilename[260];
	int AudioLeadIn;
	int PreviewTime;
}GENERAL;

typedef struct
{
	char Title[64];
	char Artist[64];
	char AudioFile[128];
}SONG_INFO;

SONG_INFO SongList[] = {
	{"Blue Sky", "DJ Alpha", "blue.wav"},
	{"Night Drive", "Beta", "night.wav"},
	{"Cyber Beat", "Gamma", "cyver.wav"}
};

int SongCount = 3;
int SongIndex = 0;

GENERAL M_General;

enum MENU
{
	MENU_START,
	MENU_OPTION,
	MENU_EXIT,
	MENU_MAX
};

void DrawTitle()
{
	printf("\n");
	printf("************************\n");
	printf("      ¡ØRHYTHM GAME¡Ø    \n");
	printf("************************\n");
}

void DrawMenu(int select)
{
	const char* menu[MENU_MAX] =
	{
		"START GAME",
		"OPOTION",
		"EXIT"
	};
	for (int i = 0; i < MENU_MAX; i++)
	{
		if (i == select)
			printf("¢º %s\n", menu[i]);
		else
			printf("   %s\n", menu[i]);
	}
}

int MainMenu()
{
	int select = 0;
	int key = 0;

	while (1)
	{
		system("cls");
		DrawTitle();
		DrawMenu(select);

		key = _getch();

		if (key == 224)
		{
			key = _getch();
			switch (key)
			{
			case 72 :
				if (select > 0) select--;
				break;
			case 80 :
				if (select < MENU_MAX - 1) select++;
				break;
			}
		}
		else if (key == 13)
		{
			return select;
		}
	}

}

//int note_width = 0;
//int note_height = 0;
//
//int PlayTimer = 0;
//int map_playing = FALSE;
//int beg_time = 0;
//
//int TimingPoints[MAX_TSTAMP][M_ROW] = { 0 };
//int ImagePoints[MAX_TSTAMP][N_ROW] = { 0 };
//
//void Trim(const char* src, char* dst)
//{
//	while (*src == ' ' || *src == '\t')
//		src++;
//
//
//	while (*src && *src != '\n' && *src != '\r')
//	{
//		*dst++ = *src++;
//	}
//
//	*dst = '\0';
//}
//
//
//void TPoint(char* TStr)
//{
//	int row[6] = { 0 };
//	char* ptr = strtok(TStr, ",");
//	char strs[200] = { 0 };
//	int i = 0, key = 0;
//
//	while (ptr != NULL)
//	{
//		Trim(ptr, strs);
//		row[i] = atoi(strs);
//		ptr = strtok(NULL, ",");
//
//		i++;
//	}
//	switch (row[0])
//	{
//	case 64 :
//		key = 0;
//		break;
//	case 192 :
//		key = 1;
//		break;
//	case 320 :
//		key = 2;
//		break;
//	case 448 :
//		key = 3;
//		break;
//	}
//	TimingPoints[row[2]][key] = 1;
//	if (row[3] == 128)
//	{
//		for (int n = row[2]; n <= row[5]; n++)
//			ImagePoints[n][key] = 1;
//	}
//	else
//	{
//		ImagePoints[row[2]][key] = 1;
//	}
//}
//
//void ReadProperty_General(char* str)
//{
//	char nstr[200] = { 0 };
//	char* ptr = strtok(str, ":");
//
//	if (ptr == NULL)return;
//
//	Trim(ptr, nstr);
//
//	if (strcmp(nstr, "AudioFilename") == 0)
//	{
//		ptr = strtok(NULL, ":");
//		Trim(ptr, nstr);
//		strcpy(M_General.AudioFilename, nstr);
//	}
//	else if (strcmp(nstr, "AudioLeadln") == 0)
//	{
//		ptr = strtok(NULL, ":");
//		if (ptr == NULL) return;
//
//		Trim(ptr, nstr);
//		M_General.AudioLeadIn = atoi(ptr);
//	}
//	else if (strcmp(nstr, "PreviewTime") == 0)
//	{
//		ptr = strtok(NULL, ":");
//		if (ptr == NULL) return;
//
//		Trim(ptr, nstr);
//		M_General.PreviewTime = atoi(ptr);
//	}
//}
//
//inline void Render()
//{
//	hWnd = GetConsoleWindow();
//	hInst = GetModuleHandle(NULL);
//	HDC hDC, hMemDC;
//	static HDC hBackDC;
//	HBITMAP hBackBitmap, h0ldBitmap, hNewBitmap;
//	BITMAP Bitmap;
//
//	hDC = GetDC(hWnd);
//
//	hMemDC = CreateCompatibleDC(hDC);
//	hBackDC = CreateCompatibleDC(hDC);
//
//	hBackBitmap = CreateCompatibleBitmap(hDC, 100, 500);
//	h0ldBitmap = (HBITMAP)Select0bject(hBackDC, hBackBitmap);
//
//	hNewBitmap = LoadBitmap(hInst, ¤»¤¼¤±MAKEINTRESOURCE(IDB_BITMAP1));
//	Getobject(hNewBitmap, sizeof(BITMAP), &Bitmap);
//	Selectobject(hMemDC, hNewBitmap);
//
//	note_width = Bitmap.bmWidth;
//	note_height = Bitmap.bmHeight;
//	for (int i = PlayTimer; i < PlayTimer + READ_NOTE_MIL; i++)
//	{
//		if (PlayTimer < 0)
//			break;
//		for (int j = 0; j < 4; j++)
//		{
//			if (ImagePoints[i][j] == 1)
//			{
//				GdiTransparentBit(hBackDC, j * Bitmap.bmWidth + Start_Pos, (i - PlayTimer - 500) * (-0.9), Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, Bitmap.bmHeight, RGB(255, 0, 228));
//
//			}
//		}
//	}
//	Deleteobject(hNewBitmap);
//
//	BitBlt(hDC, 0, 0, 1000, 500, hBackDC, 0, 0, SRCCOPY);
//
//	Deleteobject(Selectobject(hBackDC, hBackBitmap));
//	DeleteDC(hBackDC);
//	DeleteDC(hMemDC);
//
//	ReleaseDC(hWnd, hDC);
//}

int main()
{
	int menu = MainMenu();

	switch (menu)
	{
	case MENU_START :
		printf("Game Start\n");
		Sleep(1000);
		break;
	case MENU_OPTION :
		printf("Option Menu\n");
		Sleep(1000);
		break;
	case MENU_EXIT :
		printf("EXIT\n");
		Sleep(1000);
		break;
	}


	return 0;
}

const char* MainMenuItems[] = {
	"START",
	"EXIT"
};

int MainMenuIndex = 0;

void UpdateMainMenu(int key)
{
	if (key == VK_UP)
		MainMenuIndex = (MainMenuIndex - 1 + 2) % 2;
	if (key == VK_DOWN)
		MainMenuIndex = (MainMenuIndex - 1) % 2;
	if (key == VK_RETURN)
	{
		if (MainMenuIndex == 0)
			g_GameState = STATE_SONG_SELECT;
		else
			g_GameState = STATE_EXIT;
	}
}

void RenderMainMenu(HDC hdc)
{
	for (int i = 0; i < 2; i++)
	{
		if (i == MainMenuIndex)
			TextOut(hdc, 100, 100 + 1 * 40, "¢º", 2);

		Textout(hdc, 130, 100 + i * 40,
			MainMenuItems[i],
			strlen(MainMenuItems[i]));
	}
}

void UpdateSongSelect(int key)
{
	if (key == VK_UP)
		SongIndex = (SongIndex - 1 + SongCount) % SongCount;

	if (key == VK_DOWN)
		SongIndex = (SongIndex + 1) % SongCount;

	if (key == VK_RETURN)
		g_GameState = STATE_PLAY;

	if (key == VK_ESCAPE)
		g_GameState  STATE_MAIN_MENU;
}

void RenderSongSelect(HDC hdc)
{
	TextOut(hdc, 50, 40, "SELECT SONG". 11);

	for (int i = 0; i < SongCount; i++)
	{
		if (i == SongIndex)
			TextOut(hdc, 50, 80 + i * 30, "¢º", 2);

		TextOut(hdc, 80, 80 + i * 30,
			SongList[i].Title,
			strlen(SongList[i].Title));
	}
}

void Update(int key)
{
	switch (g_GameState)
	{
	case STATE_MAIN_MENU :
			UpdateMainMenu(key);
			break;
	case STATE_SONG_SELECT :
		UpdateSongSelect(key);
		break;
	}
}

void Render(HDC hdc)
{
	switch (g_GameState)
	{
	case STATE_MAIN_MENU :
		RenderMainMenu(hdc);
		break;
	case STATE_SONG_SELECT :
		RenderSongSelect(hdc);
		break;
	}
}