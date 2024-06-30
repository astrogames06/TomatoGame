#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#undef RAYGUI_IMPLEMENTATION

#include "player_img.h"
#include "tomato_img.h"
#include "bad_tomato_img.h"
#include "skull_img.h"


int high_score = 0;
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>

	extern "C"
	{
		EMSCRIPTEN_KEEPALIVE
		void SetHighScore(int x)
		{
			high_score = x;
		}
	}
#endif


const int WIDTH = 850;
const int HEIGHT = 450;

void UpdateDrawFrame();

int x = WIDTH/2-50/2;

enum GAME_STATE
{
	MENU,
	GAME,
	DEATH_SCREEN,
	TOMATO_INFO
};

void DrawAlien(int x, int y, Color color)
{
	DrawRectangle(x, y, 50, 50, color);
	DrawRectangle(x+10, y+30, 30, 10, BLACK);
	DrawRectangle(x+10, y+5, 10, 10, BLACK);
	DrawRectangle(x+30, y+5, 10, 10, BLACK);
}

int delay = 0;
int bad_delay = 0;
Image player_img;
Texture2D player;
std::vector<std::pair<Vector2, int>> tomatoes;

Image tomato_img;
Texture2D tomato_tex;

Image bad_tomato_img;
Texture2D bad_tomato_tex;

Image skull_img;
Texture2D skull_tex;

GAME_STATE game_state;


char text[64] = "";
bool editMode = false;

std::ofstream data_file;

#if defined(PLATFORM_WEB)
	EM_JS(void, SaveNum, (int  x), {
		localStorage.setItem("high_score", x);
	});

	EM_JS(int, GetHighScore, (), {
		return JSON.parse(localStorage.getItem("high_score"));
	});
#endif

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Israel Tomato Game");
	//ToggleFullscreen();
	game_state = MENU;
	data_file.open("data.txt", std::ios::app);

	if (!data_file)
        return 1;

	player_img.width = PLAYER_IMG_WIDTH;
	player_img.height = PLAYER_IMG_HEIGHT;
	player_img.format = PLAYER_IMG_FORMAT;
	player_img.data = PLAYER_IMG_DATA;
	player_img.mipmaps = 1;
	player = LoadTextureFromImage(player_img);
	player.width = 40;
	player.height = 65;

	tomato_img.width = TOMATO_IMG_WIDTH;
	tomato_img.height = TOMATO_IMG_HEIGHT;
	tomato_img.format = TOMATO_IMG_FORMAT;
	tomato_img.data = TOMATO_IMG_DATA;
	tomato_img.mipmaps = 1;
	tomato_tex = LoadTextureFromImage(tomato_img);
	tomato_tex.width = 40;
	tomato_tex.height = 40;

	bad_tomato_img.width = BAD_TOMATO_IMG_WIDTH;
	bad_tomato_img.height = BAD_TOMATO_IMG_HEIGHT;
	bad_tomato_img.format = BAD_TOMATO_IMG_FORMAT;
	bad_tomato_img.data = BAD_TOMATO_IMG_DATA;
	bad_tomato_img.mipmaps = 1;
	bad_tomato_tex = LoadTextureFromImage(bad_tomato_img);
	bad_tomato_tex.width = 40;
	bad_tomato_tex.height = 40;

	skull_img.width = SKULL_IMG_WIDTH;
	skull_img.height = SKULL_IMG_HEIGHT;
	skull_img.format = SKULL_IMG_FORMAT;
	skull_img.data = SKULL_IMG_DATA;
	skull_img.mipmaps = 1;
	skull_tex = LoadTextureFromImage(skull_img);
	skull_tex.width = 40;
	skull_tex.height = 56;


	#if defined(PLATFORM_WEB)
    	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
	#else
		SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
		//--------------------------------------------------------------------------------------

		// Main game loop
		while (!WindowShouldClose())    // Detect window close button or ESC key
		{
			UpdateDrawFrame();
		}
	#endif

	CloseWindow();

	return 0;
}

int points = 0;

void UpdateDrawFrame()
{

	BeginDrawing();
	
	ClearBackground(RAYWHITE);

	if (game_state == MENU)
	{
		DrawText("Tomato Game", WIDTH/2-MeasureText("Tomato Game", 40)/2, 50, 40, BLACK);
		DrawTexture(tomato_tex, MeasureText("Tomato Game", 40)-15, 45, WHITE);
		DrawTexture(tomato_tex, WIDTH-MeasureText("Tomato Game", 40)-25, 45, WHITE);

		DrawText("Made by Jesse Sher with Raylib and C++", WIDTH/2-MeasureText("Made by Jesse Sher with Raylib and C++", 20)/2, 100, 20, BLACK);
		if (GuiButton(Rectangle{
			WIDTH/2-120/2, 150, 120, 45
		}, "#119# PLAY!"))
		{
			tomatoes.clear();
			points = 0;
			x = WIDTH/2-50/2;
			game_state = GAME;
		}
		if (GuiButton(Rectangle{
			WIDTH/2-120/2, 200, 120, 45
		}, "#193# Tomatoes Info"))
		{
			game_state = TOMATO_INFO;
		}
	}
	else if (game_state == GAME)
	{
		delay++;
		bad_delay++;
		x = Clamp(x, 0, WIDTH-40);
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
			x -= 600 * GetFrameTime();
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
			x += 600 * GetFrameTime();

		if (IsGamepadAvailable(0)) {
            float axisValue = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
			

            x += axisValue * 5;
			for (int i = 0; i < GetGamepadAxisCount(0)*2; i++)
				std::cout << i << " : " << GetGamepadAxisMovement(0, i) << '\n';
        }

		if (delay >= GetFPS())
		{
			tomatoes.push_back({ { (float)GetRandomValue(50, WIDTH-50), -50}, 0 });
			delay = 0;
		}

		if (bad_delay >= GetFPS()/2)
		{
			tomatoes.push_back({ { (float)GetRandomValue(50, WIDTH-50), -50}, 1 });
			bad_delay = 0;
		}

		DrawTexture(player, x, HEIGHT-100, WHITE);
		std::string format_score = "Score: " + std::to_string(points);

		DrawText(format_score.c_str(), WIDTH/2-MeasureText(format_score.c_str(), 20)/2, 100, 20, BLACK);

		for (int i = 0; i < tomatoes.size(); i++)
		{
			tomatoes[i].first.y += 400 * GetFrameTime();
			//std::cout << "bad: " << tomatoes[i].second << '\n';
			if (tomatoes[i].second == 1)
			{
				DrawTextureV(bad_tomato_tex, tomatoes[i].first, WHITE);

				if (CheckCollisionRecs(
					Rectangle{
						tomatoes[i].first.x, tomatoes[i].first.y, 40, 40
					},
					Rectangle{
						(float)x, HEIGHT-100, 40, 65
					}
				))
				{
					game_state = DEATH_SCREEN;
				}
			}
			else
			{
				DrawTextureV(tomato_tex, tomatoes[i].first, WHITE);

				if (CheckCollisionRecs(
					Rectangle{
						tomatoes[i].first.x, tomatoes[i].first.y, 40, 40
					},
					Rectangle{
						(float)x, HEIGHT-100, 40, 65
					}
				))
				{
					tomatoes.erase(tomatoes.begin() + i);
					points++;
				}

				// if (tomatoes[i].first.y > HEIGHT)
				// {
				// 	game_state = DEATH_SCREEN;
				// }
			}
		}
	}
	else if (game_state == DEATH_SCREEN)
	{
		if (points > high_score)
		{
			#if defined(PLATFORM_WEB)
				SaveNum(points);
			#endif
		}
		// #if defined(PLATFORM_WEB)
		// 	EM_ASM({
		// 		alert(JSON.parse(localStorage.getItem("high_score")));
		// 	});
		// #endif
		if (GuiButton(
			Rectangle { WIDTH/2-120/2, 250, 120, 45 },
			"#185# Home"
		))
		{
			data_file << text << " : " << points << '\n';
			game_state = MENU;
			text[0] = '\0'; 
		}
		DrawText("ENTER NAME: ", WIDTH/2-MeasureText("ENTER NAME: ", 20)/2, 150, 20, BLACK);
		if (GuiTextBox((Rectangle){ WIDTH/2-200/2, 185, 200, 35 }, text, 64, editMode))
		{
			editMode = !editMode;
		}
		DrawText("YOU LOST!", WIDTH/2-MeasureText("YOU LOST!", 40)/2, 50, 40, BLACK);
		DrawTexture(skull_tex, MeasureText("YOU LOST!", 40)-60, 45, WHITE);
		DrawTexture(skull_tex, WIDTH-MeasureText("YOU LOST!", 40)+25, 45, WHITE);

		std::string format_score = "Score: " + std::to_string(points);

		#if defined(PLATFORM_WEB)
			std::string format_high_score = "High Score: " + std::to_string(GetHighScore());

			DrawText(format_high_score.c_str(), (WIDTH/2-MeasureText(format_high_score.c_str(), 20)/2), 120, 20, BLACK);
		#endif

		DrawText(format_score.c_str(), WIDTH/2-MeasureText(format_score.c_str(), 20)/2, 100, 20, BLACK);
	}
	else if (game_state == TOMATO_INFO)
	{
		DrawText("Tomatoes Info", WIDTH/2-MeasureText("Tomatoes Info", 40)/2, 50, 40, BLACK);
		DrawTexture(tomato_tex, MeasureText("Tomatoes Info", 40)-60, 45, WHITE);
		DrawTexture(tomato_tex, WIDTH-MeasureText("Tomatoes Info", 40)+25, 45, WHITE);
		DrawText(
			"Cherry Tomatoes are a delicious type of tomato",
			WIDTH/2-MeasureText("Cherry Tomatoes are a delicious type of tomato", 20)/2,
			150, 20, BLACK
		);
		DrawText(
			"that not many people know where they came from.",
			WIDTH/2-MeasureText("that not many people know where they came from.", 20)/2,
			180, 20, BLACK
		);
		DrawText(
			"There roots originate from Israel no pun intended.",
			WIDTH/2-MeasureText("There roots originate from Israel no pun intended.", 20)/2,
			210, 20, BLACK
		);
		DrawText(
			"It was invented by Israeli scientists and now",
			WIDTH/2-MeasureText("It was invented by Israeli scientists and now", 20)/2,
			240, 20, BLACK
		);
		DrawText(
			"are one of the most sold tomatoes in the world.",
			WIDTH/2-MeasureText("are one of the most sold tomatoes in the world.", 20)/2,
			270, 20, BLACK
		);
		DrawText(
			"We now consume over 182 million tons of tomatoes a year.",
			WIDTH/2-MeasureText("We now consume over 182 million tons of tomatoes a year.", 20)/2,
			300, 20, BLACK
		);

		if (GuiButton(
			Rectangle { WIDTH/2-120/2, 350, 120, 45 },
			"#185# Home"
		))
		{
			game_state = MENU;
		}
	}

	EndDrawing();
}