#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <sddl.h>
#include <stdlib.h>
#include <conio.h>
#include <MMSystem.h>
#include <string.h>
#pragma comment (lib, "winmm.lib")
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
	char AudioPath[128];
}SONG_INFO;

SONG_INFO SongList[] = {
	{"Replica", "Yuuri", "replica.wav"},
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

typedef enum
{
	STATE_MAIN_MENU,
	STATE_SONG_SELECT,
	STATE_PLAY,
	STATE_EXIT
}GAME_STATE;

GAME_STATE g_GameState = STATE_MAIN_MENU;

typedef struct
{
	char Title[64];
	char Artist[64];
	char AudioPath[128];
	int PreviewTime;
} SONG;

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
			printf("¢¹ %s\n", menu[i]);
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
			TextOut(hdc, 100, 100 + 1 * 40, L"¢¹", 2);

		TextOut(hdc, 130, 100 + i * 40,
			MainMenuItems[i],
			(int)wcslen(MainMenuItems[i]));
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
		g_GameState = STATE_MAIN_MENU;
}

void RenderSongSelect(HDC hdc)
{
	TextOut(hdc, 50, 40, L"SELECT SONG", 11);

	for (int i = 0; i < SongCount; i++)
	{
		if (i == SongIndex)
			TextOut(hdc, 50, 80 + i * 30, L"¢¹", 2);

		TextOut(hdc, 80, 80 + i * 30,
			SongList[i].Title,
			(int)wcslen(SongList[i].Title));
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

void PlayMusic(const char* path)
{
	PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	while (1)
	{
		Sleep(1000);
	}
}

void StopMusic()
{
	PlaySound(NULL, 0, 0);
}

int lastIndex = -1;

void UpdateSongPreview()
{
	if (SongIndex != lastIndex)
	{
		StopMusic();
		PlayMusic(SongList[SongIndex].AudioPath);
		lastIndex = SongIndex;
	}
}

void PlayPreview(const char* path, int startMs)
{
	char cmd[256];

	mciSendStringA("close priview", NULL, 0, NULL);

	sprintf(cmd, "open \"%s\" type mpegvideo alias preview", path);
	mciSendStringA(cmd, NULL, 0, NULL);

	sprintf(cmd, "play preview from %d", startMs);
	mciSendStringA(cmd, NULL, 0, NULL);
}

void TPoint(char* TStr);

void LoadMap(const char* path)
{
	FILE* fp = fopen(path, "replica.wav");
	char line[256];

	while (fgets(line, sizeof(line), fp))
	{
		TPoint(line);
	}
	fclose(fp);
}
