#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#define range 15
#define delay 3
#define gametime 60
/* 텍스트 색상 관련 함수 모음 */
void colorset(int background, int textcolor)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, (background << 4) + textcolor);
}

typedef enum colors // 컬러들을 미리 선언해둔다. --> colorset에서 사용용
{
    black,        // 0
    blue,         // 1
    green,        // 2
    skyBlue,      // 3
    red,          // 4
    pink,         // 5
    Orange,       // 6
    white,        // 7
    gray,         // 8
    lightBlue,    // 9
    brightGreen,  // 10
    sky,          // 11
    brightRed,    // 12
    brightpink,   // 13
    brightyellow, // 14
    brightwhite,  // 15
    yellow,       // 16
} colors;

/*게임 플레이 관련*/
int is_game_running = 1;
time_t first_time;
int selected_map;
typedef struct player
{
    int x;
    int y;
    int is_tagger; // 술래 여부를 저장하는 변수
    // int score; 아이템 먹으면 증가하는 식으로 고민해봐야함
} PL;

typedef struct msg
{
    char str[70];
} end_msg;

PL player1, player2;
end_msg msg[4] = {{"쓰디쓴 패배를 맛보았습니다."}, {"달콤한 승리의 영광을 눈 앞에서 빼았겼습니다."}, {"농락 당하게 될 예정입니다."}, {" \"아야!\" 했습니다"}};

int dy[9] = {1, 1, 1, 0, 0, 0, -1, -1, -1}; // logic을 위한 방향 배열(3by3 search 구현용)
int dx[9] = {1, 0, -1, 1, 0, -1, 1, 0, -1}; // BFS/DFS 알고리즘에서 쓰는거 활용

int text[3][range][range] = { // 0:통행 가능 공간 1: 장애물 2:Player1 3:Player2
    {
        {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0}, // map 1
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}},
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
        {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0}, // map 2
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
        {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 
    },
    {
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}, // map 3
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}}
    };
    
int map[3][range][range] = { // 0:통행 가능 공간 1: 장애물 2:Player1 3:Player2
    {
        {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0}, // map 1
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}},
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
        {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0}, // map 2
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
        {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
        {0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}, // map 3
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
    }
};

void init() // 초기값을 초기화하는 함수
{
    player1.x = 0;
    player1.y = 0;
    player2.x = 14;
    player2.y = 14; // 포지션 값 초기화
    srand((unsigned)time(NULL));
    int a = rand() & 1;
    selected_map = rand() % 3; // 0,1,2중 하나
    if (a == 0)
    {
        player1.is_tagger = 1;
        player2.is_tagger = 0;
    }
    else
    {
        player1.is_tagger = 0;
        player2.is_tagger = 1;
    }
}

void update() // 키 입력에 따라 사용자의 좌표를 이동 시키는 함수
{
    // 레퍼런스
    /*https://learn.microsoft.com/ko-kr/windows/win32/api/winuser/nf-winuser-getasynckeystate*/
    if (GetAsyncKeyState('A') & 0x8000 && player1.x > 0 && map[selected_map][player1.y][player1.x - 1] != 1)
    {
        player1.x--;
        Sleep(delay);
    }
    if (GetAsyncKeyState('D') & 0x8000 && player1.x < range - 1 && map[selected_map][player1.y][player1.x + 1] != 1)
    {
        player1.x++;
        Sleep(delay);
    }
    if (GetAsyncKeyState('W') & 0x8000 && player1.y > 0 && map[selected_map][player1.y - 1][player1.x] != 1)
    {
        player1.y--;
        Sleep(delay);
    }
    if (GetAsyncKeyState('S') & 0x8000 && player1.y < range - 1 && map[selected_map][player1.y + 1][player1.x] != 1)
    {
        player1.y++;
        Sleep(delay);
    }
    // 키가 눌렸다면, 맵 범위 체크 및 장애물 체크 후 이동
    if (GetAsyncKeyState(VK_LEFT) & 0x8000 && player2.x > 0 && map[selected_map][player2.y][player2.x - 1] != 1)
    {
        player2.x--;
        Sleep(delay);
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && player2.x < range - 1 && map[selected_map][player2.y][player2.x + 1] != 1)
    {
        player2.x++;
        Sleep(delay);
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000 && player2.y > 0 && map[selected_map][player2.y - 1][player2.x] != 1)
    {
        player2.y--;
        Sleep(delay);
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000 && player2.y < range - 1 && map[selected_map][player2.y + 1][player2.x] != 1)
    {
        player2.y++;
        Sleep(delay);
    }
}

void make()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD home = {0, 0};
    SetConsoleCursorPosition(hConsole, home);
    // 남은 시간 연산 관련 로직직
    time_t now = time(NULL);
    int remain = gametime - (int)(now - first_time); // 시간이 얼마나 지났는지
    if (remain < 0)
        remain = 0; // 시간이 음수가 되면 안되기 때문에 바꿔줌줌

    int i, j;
    map[selected_map][player1.y][player1.x] = 2;
    map[selected_map][player2.y][player2.x] = 3;
    for (i = 0; i < range; i++)
    {
        for (j = 0; j < range; j++)
        {
            switch (map[selected_map][i][j])
            {
            case 1:
                colorset(black, red);
                printf("◼");
                break;
            case 2:
                colorset(black, blue);
                printf("⚈");
                break;
            case 3:
                colorset(black, Orange);
                printf("⚉");
                break;
            default:
                colorset(black, white);
                printf("◻");
                break;
            }
        }
        printf("\n");
    }

    colorset(black, white);
    printf("\nPlayer1");
    colorset(black, blue);
    printf("(⚈)");
    colorset(black, white);
    printf(" : WASD      ");
    printf("Player2");
    colorset(black, Orange);
    printf("(⚉)");
    colorset(black, white);
    printf(" : Arrow Key\n");
    printf("tagger  :  ");
    printf("Player %d (", (player1.is_tagger == 1) ? 1 : 2);
    colorset(black, (player1.is_tagger == 1) ? blue : Orange);
    (player1.is_tagger == 1) ? printf("⚈") : printf("⚉");
    colorset(black, white);
    printf(")  남은 시간 : %02d\n", remain);

    // 원래 텍스트 배열 복원
    map[selected_map][player1.y][player1.x] = text[selected_map][player1.y][player1.x];
    map[selected_map][player2.y][player2.x] = text[selected_map][player2.y][player2.x];
}

void information()
{
    puts("                      ██ 게임 규칙 ██                              ");
    puts("       1. 두 명의 플레이어중 한 명이 tagger가 됩니다. ");
    puts("       2. tagger칸을 기준으로 3by3 칸에 플레이어가 두명일 경우, tagger 역할과 진영이 전환됩니다.");
    puts("       3. 60초의 시간이 지난 후, tagger인 플레이어가 패배합니다.");
}

void menu() // 초기 화면
{
    puts("████████╗ █████╗  ██████╗               ████████╗ █████╗  ██████╗ ");
    puts("╚══██╔══╝██╔══██╗██╔════╝               ╚══██╔══╝██╔══██╗██╔════╝ ");
    puts("   ██║   ███████║██║  ███╗    █████╗       ██║   ███████║██║  ███╗");
    puts("   ██║   ██╔══██║██║   ██║    ╚════╝       ██║   ██╔══██║██║   ██║");
    puts("   ██║   ██║  ██║╚██████╔╝                 ██║   ██║  ██║╚██████╔╝");
    puts("   ╚═╝   ╚═╝  ╚═╝ ╚═════╝                  ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ");
    puts("");
    information();
    printf("    Press any key to continue >> ");
    while (1)
    {
        if (getch())
        {
            break;
        }
    }
    system("cls");
    puts("loading...");
    Sleep(delay * 333);
}

typedef struct pair
{
    int x;
    int y;
} twop;

int logic()
{
    int i;
    twop cur;
    PL *tagger;
    PL *runner;
    const char *name;

    tagger = (player1.is_tagger == 1) ? &player1 : &player2; // tagger,runner 고르기
    runner = (player1.is_tagger == 1) ? &player2 : &player1;

    name = (runner == &player1) ? "Player 1" : "Player 2";

    for (i = 0; i < 9; i++)
    {
        cur.x = tagger->x + dx[i];
        cur.y = tagger->y + dy[i];
        if (runner->x == cur.x && runner->y == cur.y)
        {
            runner->x = 0;
            runner->y = 0; // 초기 위치로 이동
            tagger->x = 14;
            tagger->y = 14;
            runner->is_tagger = 1;
            tagger->is_tagger = 0;
            system("cls");
            colorset(black, red);
            puts("BOOM! 💥");
            colorset(black, brightpink);
            printf("%s이(가) 터져버리고 말았습니다!\n", name);
            puts("진영이 반전됩니다");
            puts(" ");
            Sleep(delay * 475);
            system("cls");
        }
    }
}

void res()
{
    system("cls");
    puts("██████╗    ██████╗    ██████╗   ███╗   ███╗");
    puts("██╔══██╗  ██╔═══██╗  ██╔═══██╗  ████╗ ████║");
    puts("██████╔╝  ██║   ██║  ██║   ██║  ██╔████╔██║");
    puts("██╔══██╗  ██║   ██║  ██║   ██║  ██║╚██╔╝██║");
    puts("██████╔╝  ██╚═══██╗  ██╚═══██╗  ██║ ╚═╝ ██║");
    puts("╚═════╝    ██████╝    ██████╝   ╚═╝     ╚═╝");
    colorset(black, white);
    printf("Player %d님이 Player %d님에 의해  %s ", (player1.is_tagger == 1) ? 1 : 2, (player1.is_tagger == 1) ? 2 : 1, msg[rand() % 4].str);
}

int main(int argc, char const *argv[])
{
    system("chcp 65001 > nul");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE; // 마우스 커서를 안보이게 하는 역할
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    init();
    menu();
    first_time = time(NULL);
    while (is_game_running)
    {
        if ((int)(time(NULL) - first_time) >= gametime)
            break;
        make(); //ERR 404 LINE!!!
        logic();
        update();
        Sleep(4);
    }
    res();
    return 0;
} 