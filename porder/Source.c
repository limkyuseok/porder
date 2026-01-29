#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <sddl.h>
#include <stdlib.h>
#include <conio.h>
#include <MMSystem.h>
#include <string.h>
#include <locale.h>
#pragma comment (lib, "winmm.lib")
//#include "resource.rc"


#define LANE_A_X 8
#define LANE_S_X 14
#define LANE_D_X 20
#define LANE_F_X 26

#define MAX_TSTAMP 100
#define M_ROW 10
#define N_ROW 20

#define IDB_BITMAP1 101
#define READ_NOTE_MIL 2000
#define Start_Pos 200

#define MAX_NOTES 1024

#define NOTE_FALL_TIME (BEAT_MS*8)

#define JUDGE_PERFECT 50
#define JUDGE_GOOD 120
#define JUDGE_MISS 200

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define JUDGE_LINE_Y 12
#define NOTE_SPEED 100
#define JUDGE_Y_TOLERANCE 1

#define BPM 200
#define BEAT_MS (60000/ BPM)

typedef struct
{
	ULONGLONG time;
	int lane;
	int y;
	int active;
}NOTE;

NOTE Notes[MAX_NOTES];
int NoteCount = 0;

int MainMenu(void);
void SongSelect(void);
void PlayMusic(const wchar_t* path);
void StopMusic(void);
void PlayGame(void);
void ClearScreenBuffer(void);
void DrawJudgeLine(void);
void DrawNotes(ULONGLONG currentTime);
void RenderGame(ULONGLONG currentTime);
void JudgeKey(int lane, ULONGLONG currentTime);
void UpdateMissNotes(ULONGLONG currentTime);
void PlayGame(void);
void UpdateNotes(ULONGLONG currentTime);
void RenderJudge(void);
void PresentScreen(void);
void DrawLaneGuide(void);
void InitConsole(void);
void DrawKeyLabels(void);

ULONGLONG GameStartTime;

HWND hWnd = NULL;
HINSTANCE hInst = NULL;

char Screen[SCREEN_HEIGHT][SCREEN_WIDTH];

char JudgeText[16] = "";
ULONGLONG JudgeTime = 0;

typedef struct
{
	char AudioFilename[260];
	int AudioLeadIn;
	int PreviewTime;
}GENERAL;

typedef struct
{
	wchar_t Title[64];
	wchar_t Artist[64];
	wchar_t AudioPath[128];
}SONG_INFO;

SONG_INFO SongList[] =
{
	{L"replica", L"Yuuri", L"replica.wav"},
	{L"가을아침", L"IU", L"가을아침.wav"},
	{L"hiroin", L"BackNumber", L"hiroin.wav"},
};

int SongCount = sizeof(SongList) / sizeof(SongList[0]);
int SongIndex = 0;

int GetLaneFromKey(int key);

GENERAL M_General;

void LoadMapFile(const wchar_t* path);

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

void gotoxy(int x, int y)
{
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void gotoxy(int x, int y);

void InitConsole(void)
{
	system("mod con cols = 40 lines = 25");
}

void DrawTitle()
{
	printf("\n");
	printf("************************\n");
	printf("      ※RHYTHM GAME※    \n");
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
			printf("▷ %s\n", menu[i]);
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
			case 72:
				if (select > 0) select--;
				break;
			case 80:
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
	setlocale(LC_ALL, " ");
	InitConsole();

	setlocale(LC_ALL, "");

	GameStartTime = timeGetTime();

	while (1)
	{
		int menu = MainMenu();

		if (menu == MENU_START)
		{
			SongIndex = 0;
			SongSelect();
		}
		else if (menu == MENU_OPTION)
		{
			printf("Option Menu\n");
			Sleep(1000);
		}
		else if (menu == MENU_EXIT)
		{
			break;
		}
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
			TextOut(hdc, 100, 100 + 1 * 40, L"▷", 2);

		TextOut(
			hdc,
			130,
			100 + i * 40,
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
			TextOut(hdc, 50, 80 + i * 30, L"▷", 2);

		TextOut(hdc,
			80,
			80 + i * 30,
			SongList[i].Title,
			(int)wcslen(SongList[i].Title));
	}
}

void Update(int key)
{
	switch (g_GameState)
	{
	case STATE_MAIN_MENU:
		UpdateMainMenu(key);
		break;
	case STATE_SONG_SELECT:
		UpdateSongSelect(key);
		break;
	}
}

void Render(HDC hdc)
{
	switch (g_GameState)
	{
	case STATE_MAIN_MENU:
		RenderMainMenu(hdc);
		break;
	case STATE_SONG_SELECT:
		RenderSongSelect(hdc);
		break;
	}
}

void PlayMusic(const wchar_t* path)
{
	PlaySoundW(path, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
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
	wchar_t cmd[256];

	mciSendStringA(L"close priview", NULL, 0, NULL);

	sprintf(cmd, L"open \"%s\" type mpegvideo alias preview", path);
	mciSendStringA(cmd, NULL, 0, NULL);

	sprintf(cmd, L"play preview from %d", startMs);
	mciSendStringA(cmd, NULL, 0, NULL);
}

void TPoint(char* TStr)
{
	printf("%s", TStr);
}

void LoadMap(const char* path)
{
	FILE* fp = fopen(path, "r");
	char line[256];

	while (fgets(line, sizeof(line), fp))
	{
		TPoint(line);
	}
	fclose(fp);
}

void SongSelect()
{
	int key = 0;

	while (1)
	{
		system("cls");
		wprintf(L"---SELECT SONG---\n\n");

		if (SongCount <= 0)
		{
			wprintf(L"(No Songs Loaded)\n");
		}

		for (int i = 0; i < SongCount; i++)
		{
			if (i == SongIndex)
				wprintf(L"▶ %ls\n", SongList[i].Title);
			else
				wprintf(L"   %ls\n", SongList[i].Title);
		}
		key = _getch();

		if (key == 224)
		{
			key = _getch();
			if (key == 72)
				SongIndex = (SongIndex - 1 + SongCount) % SongCount;
			if (key == 80)
				SongIndex = (SongIndex + 1) % SongCount;
		}
		else if (key == 13)
		{
			LoadMapFile(L"replica.map");
			PlayMusic(SongList[SongIndex].AudioPath);
			PlayGame();
		}
		else if (key == 27)
		{
			StopMusic();
			return;
		}
	}
}

void DrawLaneGuide(void)
{
	int laneX[4] = { LANE_A_X, LANE_S_X, LANE_D_X, LANE_F_X };

	for (int i = 0; i < 4; i++)
	{
		for (int y = 0; y < JUDGE_LINE_Y; y++)
			Screen[y][laneX[i]] = '|';
	}
}

void PlayGame(void)
{
	system("cls");

	GameStartTime = GetTickCount64();

	while (1)
	{
		ULONGLONG now = GetTickCount64() - GameStartTime;

		UpdateNotes(now);
		UpdateMissNotes(now);
		RenderGame(now);

		if (_kbhit())
		{
			int key = _getch();
			key = toupper(key);

			if (key == 27)
				return;

			int lane = GetLaneFromKey(key);
			if (lane != -1)
				JudgeKey(lane, now);
		}
		Sleep(16);
	}
}

void LoadMapFile(const wchar_t* path)
{
	FILE* fp = _wfopen(path, L"r");
	wchar_t line[256];

	if (!fp)
	{
		wprintf(L"Failed to open map file\n");
		return;
	}

	NoteCount = 0;

	while (!feof(fp) && NoteCount < MAX_NOTES)
	{
		int time, lane;

		if (fwscanf(fp, L"%d %d", &time, &lane) == 2)
		{
			Notes[NoteCount].time = time;
			Notes[NoteCount].lane = lane;
			Notes[NoteCount].active = 0;
			Notes[NoteCount].y = 0;

			NoteCount++;
		}
	}
	fclose(fp);

	wprintf(L"[MAP LOADED] Notes : %d\n", NoteCount);
	Sleep(500);
}

void UpdateNotes(ULONGLONG currentTime)
{
	for (int i = 0; i < NoteCount; i++)
	{
		ULONGLONG appearTime = Notes[i].time - NOTE_FALL_TIME;

		if (currentTime < appearTime)
			continue;

		if (Notes[i].active == 0)
		{
			Notes[i].active = 1;
		}

			float progress =
				(float)(currentTime - appearTime) / NOTE_FALL_TIME;

		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;

		Notes[i].y = (int)(progress * JUDGE_LINE_Y);

		if (currentTime > Notes[i].time + JUDGE_MISS)
		{
			Notes[i].active = 0;
			printf("MISS\n");
		}
	}
}

int GetNoteY(const NOTE* note, ULONGLONG currentTime)
{
	ULONGLONG now = GetTickCount64() - GameStartTime;

	float progress =
		(float)(now - (note->time - NOTE_FALL_TIME)) / NOTE_FALL_TIME;

	if (progress < 0.0f) progress = 0.0f;
	if (progress > 1.0f) progress = 1.0f;

	return(int)(progress * 10);
}

int GetLaneFromKey(int key)
{
	switch (key)
	{
	case 'A': return 0;
	case 'S': return 1;
	case 'D': return 2;
	case 'F': return 3;
	}
	return -1;
}

void JudgeInput(int lane)
{
	ULONGLONG now = GetTickCount64();

	for (int i = 0; i < NoteCount; i++)
	{
		if (!Notes[i].active) continue;
		if (Notes[i].lane != lane) continue;

		long diff = (long)(now - Notes[i].time);

		if (abs(diff) < 100)
		{
			printf("PERFECT\n");
			Notes[i].active = 0;
			return;
		}
		else if (abs(diff) < 200)
		{
			printf("GOOD\n");
			Notes[i].active = 0;
			return;
		}
	}
}


void UpdateMissNotes(ULONGLONG currentTime)
{
	for (int i = 0; i < NoteCount; i++)
	{
		if (!Notes[i].active) continue;

		if (currentTime > (ULONGLONG)Notes[i].time + 300)
		{
			Notes[i].active = 0;
		}
	}
}

void ClearScreenBuffer()
{
	for (int y = 0; y < SCREEN_HEIGHT; y++)
		for (int x = 0; x < SCREEN_WIDTH; x++)
			Screen[y][x] = ' ';
}

void DrawNotes(ULONGLONG currentTime)
{
	int laneX[4] = { LANE_A_X, LANE_S_X, LANE_D_X, LANE_F_X };

	for (int i = 0; i < NoteCount; i++)
	{
		if (Notes[i].active == 0)
			continue;

		if (Notes[i].y > JUDGE_LINE_Y + 2)
			continue;

		int x = laneX[Notes[i].lane];
		int y = Notes[i].y;

		if (y >= 0 && y < SCREEN_HEIGHT)
			Screen[y][x] = '0';
	}
}

void DrawJudgeLine(void)
{
	for (int x = 0; x < SCREEN_WIDTH; x++)
		Screen[JUDGE_LINE_Y][x] = '-';
	
}

void RenderGame(ULONGLONG currentTime)
{
	ClearScreenBuffer();
	DrawNotes(currentTime);
	DrawLaneGuide();
	DrawJudgeLine();
	DrawKeyLabels();
	RenderJudge();
	PresentScreen();
}

void JudgeKey(int lane, ULONGLONG currentTime)
{
	for (int i = 0; i < NoteCount; i++)
	{
		if (!Notes[i].active) continue;
		if (Notes[i].lane != lane) continue;

		if (abs(Notes[i].y - JUDGE_LINE_Y) > JUDGE_Y_TOLERANCE)
			continue;

		long diff = (long)(currentTime - Notes[i].time);

		if (abs(diff) <= JUDGE_PERFECT)
		{
			printf("PERFECT\n");
			Notes[i].active = 0;
			return;
		}
		else if (abs(diff) <= JUDGE_GOOD)
		{
			printf("GOOD\n");
			Notes[i].active = 0;
			return;
		}
	}
}

void RenderJudge(void)
{
	if (JudgeText[0] == '\0')return;

	ULONGLONG now = GetTickCount64();

	if (now - JudgeTime > 500)
	{
		JudgeText[0] = '\0';
		return;
	}
	
	int x = 15;
	int y = JUDGE_LINE_Y + 2;

	for (int i = 0; JudgeText[i] && x + i < 20; i++)
		Screen[y][x + 1] = JudgeText[i];
}

void PresentScreen(void)
{
	gotoxy(0, 0);
	
	for (int y = 0; y < SCREEN_HEIGHT; y++)
	{
		for (int x = 0; x < SCREEN_WIDTH; x++)
			putchar(Screen[y][x]);
		putchar('\n');
	}
}

void DrawKeyLabels(void)
{
	Screen[JUDGE_LINE_Y + 1][LANE_A_X] = 'A';
	Screen[JUDGE_LINE_Y + 1][LANE_S_X] = 'S';
	Screen[JUDGE_LINE_Y + 1][LANE_D_X] = 'D';
	Screen[JUDGE_LINE_Y + 1][LANE_F_X] = 'F';
}