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
#define Start_Pos 200

#define MAX_NOTES 1024

#define READ_NOTE_MIL 1500

#define JUDGE_PERFECT 800
#define JUDGE_GOOD 800
#define JUDGE_MISS 800

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25
#define JUDGE_LINE_Y 12
#define NOTE_SPEED 100
#define JUDGE_Y_TOLERANCE 1

#define NOTE_START_Y 1
#define NOTE_END_Y (JUDGE_LINE_Y - 1)

#define BPM 200
#define BEAT_MS (60000/ BPM)

static HWND hWnd;
static HINSTANCE hInst;

HFONT font;
HFONT font_combo;
HFONT font_score;
HFONT font_acc;
RECT rt;

typedef struct
{
	ULONGLONG time;
	int lane;
	float y;
	int active;
}NOTE;

NOTE Notes[MAX_NOTES];
int NoteCount = 0;

int IsPause = FALSE;

int PlayTimer = 0;
int map_playing = FALSE;
int beg_time = 0;

int KeyDown[4] = { 0 };
int KeyLight[4] = { 0 };

int UserDataLoad();

int g_TotalNotes = 0;
int g_HitPerfect = 0;
int g_HitGood = 0;
int g_HitMiss = 0;

int g_LastNoteTime = 0;

int g_FinalScore = 0;
float g_Accuracy = 0.0f;
char g_Grade = 'D';

int g_ShowResult = 0;

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
void LoadMap(char* MapName);
void DrawPlayTime(ULONGLONG currentTime);
void HideCursor(void);

void ResetScore(void);
void ComputeResult(void);
void RenderRsultScreen(void);

void MusicResume(int ID);
void MusicStop(int ID);

ULONGLONG GameStartTime;

HWND hWnd = NULL;
HINSTANCE hInst = NULL;

int LaneX[4] = { LANE_A_X, LANE_S_X, LANE_D_X, LANE_F_X };

char Screen[SCREEN_HEIGHT][SCREEN_WIDTH];
//char* NoteMapName = "replica - Yuuri [replica].map";
//char* NoteMapName = "Hurai - AKMU [Hurai].map";
char* NoteMapName = "Silhouette - BackNumber [Silhouette].map";


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
	{L"audio", L"Yuuri", L"audio.wav"},
	{L"Hurai", L"AKMU", L"Hurai.wav"},
	{L"Silhouette", L"BackNumber", L"Silhouette.wav"},
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

int UserDataLoad()
{
	FILE* fp;
	int index = 0;
	int data;

	fp = fopen("score.txt", "r");
	while (fscanf(fp, "%d", &data) != EOF)
	{
		index = (int)data;
	}
	fclose(fp);
	return index;
}

void gotoxy(int x, int y)
{
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void gotoxy(int x, int y);

void InitConsole(void)
{
	system("mode con cols = 40 lines = 25");
}

void DrawTitle()
{
	printf("\n");
	printf("************************\n");
	printf("       RHYTHM GAME     \n");
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
			printf("> %s\n", menu[i]);
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
	SetConsoleOutputCP(437);
	SetConsoleCP(437);

	HideCursor();

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
	hWnd = GetConsoleWindow();
	hInst - GetModuleHandle(NULL);
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, h01dBitmap, hNewBitmap;
	BITMAP Bitmap;

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	switch (g_GameState)
	{
	case STATE_MAIN_MENU:
		RenderMainMenu(hdc);
		break;
	case STATE_SONG_SELECT:
		RenderSongSelect(hdc);
		break;
	}

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
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

	g_LastNoteTime = 0;
	for (int i = 0; i < NoteCount; i++)
	{
		if (Notes[i].time > g_LastNoteTime)
			g_LastNoteTime = (int)Notes[i].time;
	}
}

void SongSelect()
{
	int key = 0;
	int lastIndex = -1;

	while (1)
	{
		system("cls");
		wprintf(L"---SELECT SONG---\n\n");

		for (int i = 0; i < SongCount; i++)
		{
			if (i == SongIndex)
				wprintf(L"> %ls\n", SongList[i].Title);
			else
				wprintf(L"   %ls\n", SongList[i].Title);
		}

		if (SongIndex != lastIndex)
		{
			PlaySongPreview(SongList[SongIndex].AudioPath, 30000, 4000);
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
			GameStartTime = GetTickCount64();
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

void MusicResume(int ID)
{
	mciSendCommandW(ID, MCI_RESUME, 0, NULL);
}

void MusicStop(int ID)
{
	mciSendCommandW(ID, MCI_CLOSE, 0, NULL);
}

void DrawLaneGuide(void)
{
	int laneX[4] = { LANE_A_X, LANE_S_X, LANE_D_X, LANE_F_X };

	for (int i = 0; i < 4; i++)
	{
		for (int y = NOTE_START_Y; y < JUDGE_LINE_Y; y++)
			Screen[y][laneX[i]] = '|';
	}
}

void PlayGame(void)
{
	system("cls");
	ResetScore();

	while (1)
	{
		ULONGLONG nowAbs = GetTickCount64();
		ULONGLONG now = nowAbs - GameStartTime;

		UpdateNotes(nowAbs);
		RenderGame(nowAbs);

		if (now > (ULONGLONG)g_LastNoteTime + JUDGE_MISS + 1000)
		{
			StopMusic();
			ComputeResult();
			RenderRsultScreen();

			while (1)
			{
				int k = _getch();
				if (k == 13)return;
				if (k == 27)return;
			}
		}

		if (_kbhit())
		{
			int key = _getch();
			key = toupper(key);

			if (key == 27)
				return;

			int lane = GetLaneFromKey(key);
			if (lane != -1)
				JudgeKey(lane, nowAbs);
		}
		Sleep(16);
	}
}

void LoadMapFile(const wchar_t* path)
{
	FILE* fp = _wfopen(path, L"r");

	if (!fp)
	{
		wprintf(L"Failed to open map file\n");
		return;
	}

	NoteCount = 0;

	while (NoteCount < MAX_NOTES)
	{
		int time, lane;
		if (fwscanf(fp, L"%d %d", &time, &lane) != 2)
			break;

		Notes[NoteCount].time = time;
		Notes[NoteCount].lane = lane;
		Notes[NoteCount].active = 0;
		Notes[NoteCount].y = 0;

		NoteCount++;

	}
	fclose(fp);

	wprintf(L"[MAP LOADED] Notes : %d\n", NoteCount);
	Sleep(500);

	g_LastNoteTime = 0;
	for (int i = 0; i < NoteCount; i++)
	{
		if ((int)Notes[i].time > g_LastNoteTime)
			g_LastNoteTime = (int)Notes[i].time;
	}
}

void UpdateNotes(ULONGLONG currentTime)
{
	ULONGLONG now = currentTime - GameStartTime;

	for (int i = 0; i < NoteCount; i++)
	{
		if (Notes[i].active == -1)
			continue;

		ULONGLONG appearTime = Notes[i].time - READ_NOTE_MIL;

		if (now < appearTime)
			continue;

		Notes[i].active = 1;

		float progress = (float)(now - appearTime) / READ_NOTE_MIL;

		if (Notes[i].active == 0)
			Notes[i].active = 1;
		
		if (progress < 0.0f)progress = 0.0f;
		if (progress > 1.0f)progress = 1.0f;

		Notes[i].y = NOTE_START_Y + progress * (NOTE_END_Y - NOTE_START_Y);

		if (now > (ULONGLONG) Notes[i].time + JUDGE_MISS)
		{
			Notes[i].active = -1;
			strcpy(JudgeText, "MISS");
			JudgeTime = GetTickCount64();

			g_HitMiss++;
		}
	}
}

int GetNoteY(const NOTE* note, ULONGLONG currentTime)
{
	ULONGLONG now = GetTickCount64() - GameStartTime;

	float progress =
		(float)(now - (note->time - READ_NOTE_MIL)) / READ_NOTE_MIL;

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
			strcpy(JudgeText, "PERPECT");
			JudgeTime = GetTickCount64();
			Notes[i].active = 0;
			return;
		}
		else if (abs(diff) < 200)
		{
			strcpy(JudgeText, "GOOD");
			JudgeTime = GetTickCount64();
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
	for (int i = 0; i < NoteCount; i++)
	{
		if (Notes[i].active != 1)
			continue;

		if (Notes[i].lane < 0 || Notes[i].lane >= 4)
			continue;

		int y = (int)(Notes[i].y + 0.5f);

		if (y < 0 || y >= SCREEN_HEIGHT)
			continue;

		int x = LaneX[Notes[i].lane];
		Screen[y][x] = '-';
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

	DrawLaneGuide();
	DrawNotes(currentTime);

	DrawJudgeLine();
	DrawKeyLabels();
	RenderJudge();

	DrawPlayTime(currentTime);

	PresentScreen();
}

void JudgeKey(int lane, ULONGLONG currentTime)
{
	ULONGLONG now = currentTime - GameStartTime;

	for (int i = 0; i < NoteCount; i++)
	{
		if (Notes[i].active != 1) continue;
		if (Notes[i].lane != lane) continue;

		if (abs(Notes[i].y - JUDGE_LINE_Y) > JUDGE_Y_TOLERANCE)
			continue;

		long diff = (long)(now - (ULONGLONG)Notes[i].time);

		if (abs(diff) <= JUDGE_PERFECT)
		{
			printf("PERFECT\n");
			JudgeTime = GetTickCount64();
			Notes[i].active = -1;

			g_HitPerfect++;
			return;
		}
		else if (abs(diff) <= JUDGE_GOOD)
		{
			printf("GOOD\n");
			JudgeTime = GetTickCount64();
			Notes[i].active = -1;

			g_HitGood++;
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
		Screen[y][x + i] = JudgeText[i];
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

void ResetScore(void)
{
	g_TotalNotes = NoteCount;
	g_HitPerfect = g_HitGood = g_HitMiss = 0;
	g_FinalScore = 0;
	g_Accuracy = 0.0f;
	g_Grade = 'D';
	g_ShowResult = 0;
}

void ComputeResult(void)
{
	int judged = g_HitPerfect + g_HitGood + g_HitMiss;
	if (judged <= 0) judged = 1;

	g_FinalScore = g_HitPerfect * 1000 + g_HitGood * 500;

	g_Accuracy = (g_HitPerfect * 1.0f + g_HitGood * 0.7f) / (float)g_TotalNotes * 100.0f;

	if (g_Accuracy >= 90.0f)g_Grade = 'A';
	else if (g_Accuracy >= 75.0f)g_Grade = 'B';
	else if (g_Accuracy >= 60.0f)g_Grade = 'C';
	else g_Grade = 'D';
}

void RenderRsultScreen(void)
{
	system("cls");

	printf("----RESULT---\n\n");
	printf("TOTAL : %d\n", g_TotalNotes);
	printf("PERFECT : %d\n", g_HitPerfect);
	printf("GOOD : %d\n", g_HitGood);
	printf("MISS : %d\n", g_HitMiss);

	printf("Score : %d\n", g_FinalScore);
	printf("ACC : %.2f%%\n", g_Accuracy);
	printf("GRADE : %c\n\n", g_Grade);
}

void DrawPlayTime(ULONGLONG currentTime)
{
	ULONGLONG elapsed = currentTime - GameStartTime;

	int ms = (int)(elapsed % 1000);
	int totalSec = (int)(elapsed / 1000);
	int sec = totalSec % 60;
	int min = totalSec / 60;

	char buf[32];
	sprintf(buf, "%02d:%02d:%03d", min, sec, ms);

	int x = 15;
	int y = 15;

	if (y < 0 || y >= SCREEN_HEIGHT)return;

	for (int i = 0; buf[i] && (x + i) < SCREEN_WIDTH; i++)
		Screen[y][x + i] = buf[i];
}

void HideCursor(void)
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 0;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(h, &info);
}

void PlaySongPreview(const wchar_t* path, int startMs, int durationMs)
{
	wchar_t cmd[256];

	mciSendStringW(L"clos preview", NULL, 0, NULL);

	swprintf(cmd, 256, L"open\"%ls\" type waveaudio alias preview", path);
	mciSendStringW(cmd, NULL, 0, NULL);

	swprintf(cmd, 256, L"play preview from &d", startMs);
	mciSendStringW(cmd, NULL, 0, NULL);

	Sleep(durationMs);
	mciSendStringW(L"stop Preview", NULL, 0, NULL);
}