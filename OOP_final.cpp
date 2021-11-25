//把程式包成struct整理

#include <stdio.h>
#include <stdlib.h>
#include<string>
#include <SDL.h>					// Using SDL
#include "SDL2_gfxPrimitives.h"		// Using SDL2_gfx
#include "SDL_image.h"				// Using SDL2 image extension library
#include "SDL_ttf.h"				// Using truetype font
#include "SDL_mixer.h" 
#include<math.h>					//旋轉矩陣要用到
#include <time.h>					//倒數計時器

#define PI 3.1415926535897

//Screen dimension constants
const int WIDTH = 800;
const int HEIGHT = 800;

//不同字體
const int SOLID = 100;
const int SHADED = 101;
const int BLENDED = 102;

//包成struct
struct basketball
{
	double vx;	//Initialize the ball velocity
	double vy;
	double px;	//initial basketball position
	double py;
	double w;
	double h;
	double size_mul;
	double theta;
	double alpha;
};
typedef struct basketball basketball;

double g = 0.0;		//計算之後系統的加速度(帶等加速度公式) 1
double fs = 0.0;	//假設空氣阻力
double theta = 0;

struct basket
{
	double x;
	double y;
	double w;
	double h;
	double basket;
	double  v;
};
typedef struct basket basket;


//initial score(呈現在螢幕面板的數字)
int score = 0;
char scorec[10] = "\0";//宣告轉換int to char的變數(才能顯示上去)


//宣告一個倒數計時的時間變數
clock_t startTime = NULL;
clock_t countTime;	//設定時間開始 clock參數
int milliseconds = 0;
int seconds = 60;				//從60開始減
char timeb[10] = "\0";//宣告轉換int to char的變數(才能顯示上去)

struct ImageData
{
	char path[100];
	SDL_Texture *texture;
	int width;
	int height;
};

struct TextData
{
	char path[100];
	SDL_Texture *texture;
	int width;
	int height;
};


//滑鼠會用到的參數
enum MouseState
{
	NONE = 0,
	IN_LB_SC = 2,  // Inside, Left Button, Single Click
	OUT = 1, // Mouse out
	IN_LB_PR = 6,  // Inside, Left Button, Press
	HOVER = 10 // Mouse hover
};

int mouseX, mouseY;
bool MLBD = false,//Mouse Left Button Down 判別是否按下滑鼠左鍵
ButtonChecking = true;//確認按下是否為有效按鍵:內設為正確


int initSDL(); // Starts up SDL and creates window
void closeSDL(); // Frees media and shuts down SDL
TextData loadTextTexture(const char *str, const char *fontPath, int fontSize, Uint8 fr, Uint8 fg, Uint8 fb, int textType, Uint8 br, Uint8 bg, Uint8 bb);
void textRender(SDL_Renderer *renderer, TextData text, int posX, int posY, int cx, int cy, double angle, SDL_RendererFlip flip);
bool setTextureAlpha(SDL_Texture *texture, Uint8 alpha);
bool loadAudio();


ImageData  loadTexture(char *path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b, int w, int h);
void imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, double w, double h, int cx, int cy, double angle);


SDL_Window *window = NULL; // The window we'll be rendering to
SDL_Renderer *renderer = NULL; // The window renderer


//投籃機音樂 進球音效指標初始化宣告
Mix_Music *music1 = NULL;
Mix_Music *music2 = NULL;


Mix_Chunk *effect1 = NULL;
Mix_Chunk *effect2 = NULL;

// Current displayed texture
int initSDL()
{
	// Initialize SDL	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		// Error Handling		
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return 1;

	}

	// Create window	
		// SDL_WINDOWPOS_UNDEFINED: Used to indicate that you don't care what the window position is.
	window = SDL_CreateWindow("OOP SDL Tutorial", 50, 50, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 2;
	}

	// Initialize PNG loading	
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image failed: %s\n", IMG_GetError());
		return 3;
	}


	// Create renderer	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 3;
	}

	// Initialize SDL_ttf	
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return 4;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return 5;
	}

	return 0;
}


void closeSDL()
{
	//free the music and effect
	Mix_FreeMusic(music1);
	Mix_FreeMusic(music2);
	Mix_FreeChunk(effect1);
	Mix_FreeChunk(effect2);

	// Destroy renderer	
	// Destroy window	
	// Quit Image subsystem
	// Quit SDL subsystems
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_Quit();

	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

TextData loadTextTexture(const char *str, const char *fontPath, int fontSize, Uint8 fr, Uint8 fg, Uint8 fb, int textType, Uint8 br, Uint8 bg, Uint8 bb)
{
	TextData text;

	// TTF Font sturct	
	TTF_Font *ttfFont = NULL;

	// Open the font	
	ttfFont = TTF_OpenFont(fontPath, fontSize);
	if (ttfFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
	}

	// A structure that represents a color.	
	SDL_Color textFgColor = { fr, fg, fb }, textBgColor = { br, bg, bb };

	// Render text surface

	SDL_Surface *textSurface = NULL;

	// Creates a solid/shaded/blended color surface from the font, text, and color given.	
	if (textType == SOLID)
	{
		// Quick and Dirty
		textSurface = TTF_RenderText_Solid(ttfFont, str, textFgColor);
	}
	else if (textType == SHADED)
	{
		// Slow and Nice, but with a Solid Box
		textSurface = TTF_RenderText_Shaded(ttfFont, str, textFgColor, textBgColor);
	}
	else if (textType == BLENDED)
	{
		// Slow Slow Slow, but Ultra Nice over another image
		textSurface = TTF_RenderText_Blended(ttfFont, str, textFgColor);
	}

	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		// Create texture from surface pixels
		text.texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (text.texture == NULL)
		{
			printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		}

		//Get text dimensions and information
		text.width = textSurface->w;
		text.height = textSurface->h;

		// Get rid of old loaded surface
		SDL_FreeSurface(textSurface);
	}

	// Free font	
	TTF_CloseFont(ttfFont);

	//return font data
	return text;
}

void textRender(SDL_Renderer *renderer, TextData text, int posX, int posY, int cx, int cy, double angle, SDL_RendererFlip flip)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	r.w = text.width;
	r.h = text.height;

	SDL_Point center = { cx, cy };

	SDL_RenderCopyEx(renderer, text.texture, NULL, &r, angle, &center, flip);
}

bool setTextureAlpha(SDL_Texture *texture, Uint8 alpha)
{
	// Set and enable standard alpha blending mode for a texture	
	if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) == -1)
	{
		printf("SDL_SetTextureBlendMode failed: %s\n", SDL_GetError());
		return -1;
	}

	// Modulate texture alpha	
	if (SDL_SetTextureAlphaMod(texture, alpha) == -1)
	{
		printf("SDL_SetTextureAlphaMod failed: %s\n", SDL_GetError());
		return -1;
	}
}

bool loadAudio()
{
	// Load music
	// https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_55.html
	// This can load WAVE, MOD, MIDI, OGG, MP3, FLAC, and any file that you use a command to play with.
	music1 = Mix_LoadMUS("../audio/start.mp3");
	if (music1 == NULL)
	{
		printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}
	music2 = Mix_LoadMUS("../audio/lose.mp3");
	if (music2 == NULL)
	{
		printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

	// Load sound effects
	// https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_19.html
	// This can load WAVE, AIFF, RIFF, OGG, and VOC files.
	effect1 = Mix_LoadWAV("../audio/two_point.wav");
	if (effect1 == NULL)
	{
		printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}
	effect2 = Mix_LoadWAV("../audio/3point.wav");
	if (effect2 == NULL)
	{
		printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

}

//SDL_Texture *loadTexture(char *path, int *width, int *height, bool ckEnable)
//新增調整圖片大小部分
ImageData loadTexture(char *path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b, int w, int h)
{
	ImageData img;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL)
	{
		printf("IMG_Load failed: %s\n", IMG_GetError());
	}
	else
	{
		// Set the color key (transparent pixel) in a surface.
		// https://wiki.libsdl.org/SDL_SetColorKey
		// The color key defines a pixel value that will be treated as transparent in a blit. 
		// It is a pixel of the format used by the surface, as generated by SDL_MapRGB().
		// Use SDL_MapRGB() to map an RGB triple to an opaque pixel value for a given pixel format.
		// https://wiki.libsdl.org/SDL_MapRGB
		SDL_SetColorKey(loadedSurface, ckEnable, SDL_MapRGB(loadedSurface->format, r, g, b));

		// Create texture from surface pixels
		img.texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (img.texture == NULL)
		{
			printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		}

		//Get image dimensions
		img.width = w;
		img.height = h;

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//return newTexture;
	return img;
}

//更新的旋轉FUNCTION
void imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, double w, double h, int cx, int cy, double angle)
{

	SDL_Rect  dst;
	dst.x = posX;
	dst.y = posY;
	dst.w = w;
	dst.h = h;

	SDL_Point center = { cx, cy };

	SDL_RenderCopyEx(renderer, img.texture, NULL, &dst, angle, &center, SDL_FLIP_NONE);
}



void drawbasket(basket &basket, basketball &ball)
{
	SDL_Rect outlineRect = { basket.x, basket.y, WIDTH * 1 / 3, HEIGHT * 1 / 4 };
	SDL_Rect outlineRects = { basket.x + 63, basket.y + 73, WIDTH * 1 / 6, HEIGHT * 1 / 9 };
	SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
	SDL_RenderDrawRect(renderer, &outlineRect);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderDrawRect(renderer, &outlineRects);
	ellipseColor(renderer, basket.x + 133, basket.y + 175, 75, 10, 0xFFFFFF00);
	//thickLineColor(renderer, 100, 30, 200, 400, 10, 0x987654FF);
	//aalineColor(renderer, 50, 50, 200, 300, 0xAABBCCDD);

	if (ball.py > basket.y + 175 && ball.py <= basket.y + 210 && ball.vy > 0 && (ball.px > basket.x + 80) && (ball.px < basket.x + 180))
	{
		arcColor(renderer, basket.x - 93, basket.y + 290, 190, 323, 350, 0xFFFFFFFF);
		arcColor(renderer, basket.x + 362, basket.y + 290, 190, 190, 217, 0xFFFFFFFF);
		SDL_RenderDrawLine(renderer, basket.x + 75, basket.y + 180, basket.x + 120, basket.y + 260);
		SDL_RenderDrawLine(renderer, basket.x + 100, basket.y + 180, basket.x + 150, basket.y + 263);
		SDL_RenderDrawLine(renderer, basket.x + 130, basket.y + 185, basket.x + 180, basket.y + 255);
		SDL_RenderDrawLine(renderer, basket.x + 160, basket.y + 183, basket.x + 190, basket.y + 220);
		SDL_RenderDrawLine(renderer, basket.x + 95, basket.y + 183, basket.x + 80, basket.y + 218);
		SDL_RenderDrawLine(renderer, basket.x + 125, basket.y + 183, basket.x + 90, basket.y + 240);
		SDL_RenderDrawLine(renderer, basket.x + 145, basket.y + 186, basket.x + 98, basket.y + 262);
		SDL_RenderDrawLine(renderer, basket.x + 187, basket.y + 183, basket.x + 135, basket.y + 265);
		SDL_Delay(10);
	}
	else if (ball.py > basket.y + 210 && ball.py <= basket.y + 250 && ball.vy > 0 && (ball.px > basket.x + 80) && (ball.px < basket.x + 180))
	{
		arcColor(renderer, basket.x - 93, basket.y + 290, 190, 323, 340, 0xFFFFFFFF);
		arcColor(renderer, basket.x + 362, basket.y + 290, 190, 200, 217, 0xFFFFFFFF);
		SDL_RenderDrawLine(renderer, basket.x + 80, basket.y + 180, basket.x + 115, basket.y + 230);
		SDL_RenderDrawLine(renderer, basket.x + 100, basket.y + 180, basket.x + 140, basket.y + 230);
		SDL_RenderDrawLine(renderer, basket.x + 130, basket.y + 185, basket.x + 165, basket.y + 230);
		SDL_RenderDrawLine(renderer, basket.x + 160, basket.y + 183, basket.x + 186, basket.y + 224);
		SDL_RenderDrawLine(renderer, basket.x + 102, basket.y + 183, basket.x + 80, basket.y + 218);
		SDL_RenderDrawLine(renderer, basket.x + 125, basket.y + 183, basket.x + 95, basket.y + 230);
		SDL_RenderDrawLine(renderer, basket.x + 145, basket.y + 186, basket.x + 114, basket.y + 232);
		SDL_RenderDrawLine(renderer, basket.x + 187, basket.y + 183, basket.x + 139, basket.y + 235);
		SDL_Delay(10);
	}
	else if (ball.py > basket.y + 250 && ball.py <= basket.y + 290 && ball.vy > 0 && (ball.px > basket.x + 80) && (ball.px < basket.x + 180))
	{
		arcColor(renderer, basket.x - 93, basket.y + 290, 190, 323, 330, 0xFFFFFFFF);
		arcColor(renderer, basket.x + 362, basket.y + 290, 190, 210, 217, 0xFFFFFFFF);
		SDL_RenderDrawLine(renderer, basket.x + 80, basket.y + 180, basket.x + 99, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 100, basket.y + 180, basket.x + 120, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 130, basket.y + 185, basket.x + 145, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 160, basket.y + 183, basket.x + 172, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 180, basket.y + 183, basket.x + 190, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 102, basket.y + 183, basket.x + 90, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 125, basket.y + 183, basket.x + 110, basket.y + 200);
		SDL_RenderDrawLine(renderer, basket.x + 145, basket.y + 186, basket.x + 124, basket.y + 202);
		SDL_RenderDrawLine(renderer, basket.x + 187, basket.y + 183, basket.x + 156, basket.y + 200);
		SDL_Delay(10);
	}

	else
	{
		//輔助線們
		//SDL_RenderDrawLine(renderer, 10, basket.y + 73, 790, basket.y + 73);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 175, 790, basket.y + 175);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 210, 790, basket.y + 210);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 250, 790, basket.y + 250);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 290, 790, basket.y + 290);

		//SDL_RenderDrawLine(renderer, basket.x + 80, 10, basket.x + 80, 790);
		//SDL_RenderDrawLine(renderer, basket.x + 60, 10, basket.x + 60, 790);
		//SDL_RenderDrawLine(renderer, basket.x + 180, 10, basket.x + 180, 790);
		//SDL_RenderDrawLine(renderer, basket.x + 210, 10, basket.x + 210, 790);
		//SDL_RenderDrawLine(renderer, 10, basket.y + basket.h / 2, 790, basket.y + basket.h / 2);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 180, 790, basket.y + 180);
		//SDL_RenderDrawLine(renderer, 10, basket.y + 160, 790, basket.y + 160);
		//basket.y + basket.h / 2
		//SDL_RenderDrawLine(renderer, basket.x + basket.w / 2 - 150, 10, basket.x + basket.w / 2 - 150, 790);
		//SDL_RenderDrawLine(renderer, basket.x + basket.w / 2 - 40, 10, basket.x + basket.w / 2 - 40, 790);
		//basket.x + basket.w / 2 - 150
		//basket.x + basket.w / 2 - 40
		arcColor(renderer, basket.x - 93, basket.y + 290, 190, 323, 360, 0xFFFFFFFF);
		arcColor(renderer, basket.x + 362, basket.y + 290, 190, 180, 217, 0xFFFFFFFF);
		SDL_RenderDrawLine(renderer, basket.x + 80, basket.y + 180, basket.x + 130, basket.y + 295);
		SDL_RenderDrawLine(renderer, basket.x + 110, basket.y + 180, basket.x + 160, basket.y + 295);
		SDL_RenderDrawLine(renderer, basket.x + 140, basket.y + 185, basket.x + 175, basket.y + 245);
		SDL_RenderDrawLine(renderer, basket.x + 170, basket.y + 183, basket.x + 185, basket.y + 220);
		SDL_RenderDrawLine(renderer, basket.x + 95, basket.y + 183, basket.x + 80, basket.y + 218);
		SDL_RenderDrawLine(renderer, basket.x + 125, basket.y + 183, basket.x + 90, basket.y + 245);
		SDL_RenderDrawLine(renderer, basket.x + 155, basket.y + 186, basket.x + 100, basket.y + 290);
		SDL_RenderDrawLine(renderer, basket.x + 180, basket.y + 183, basket.x + 125, basket.y + 290);
	}
}



// When using SDL, you have to use "int main(int argc, char* args[])"
// int main() and void main() are not allowed
int main(int argc, char* args[])
{

	char imgPath1[100] = "../images/basketball.png";
	char imgPath2[100] = "../images/basket2.png";
	char imgPath3[100] = "../images/log.PNG";

	ImageData ball, basket, log;
	int i = 1;
	int ii=1;

	struct basketball ball1;
	struct basketball ball2;

	//10個項目初始化
	ball1.vx = 0;
	ball1.vy = 0;
	ball1.px = WIDTH / 2 - WIDTH / 18;
	ball1.py = WIDTH / 2 + 150;
	ball1.h = WIDTH / 12;
	ball1.w = WIDTH / 12;
	ball1.size_mul = 1;
	ball1.alpha = 255;
	ball1.theta = 0;

	struct basket basket1;
	basket1.h = WIDTH / 3;
	basket1.w = WIDTH / 3;
	basket1.x = WIDTH / 2 - WIDTH / 4;
	basket1.y = 50;
	basket1.basket = basket1.x + (basket1.w / 2);		//偵測位置隨著籃框位置更動而更動
	basket1.v = 4;


	//side 的部份
	char shoot_timeb[10] = "\0";
	int shoot_time = 10;
	bool shoot_or_not = false;

	//10個項目初始化
	ball2.vx = 0;
	ball2.vy = 0;
	ball2.px = 200;
	ball2.py = WIDTH / 2 + 150;
	ball2.h = WIDTH / 12;
	ball2.w = WIDTH / 12;
	ball2.size_mul = 1;
	ball2.alpha = 255;
	ball2.theta = 0;

	// Start up SDL and create window
	if (initSDL())
	{
		printf("Failed to initialize SDL!\n");
		return -1;
	}

	//音樂音效指向設定
	loadAudio();

	ball = loadTexture(imgPath1, true, NULL, NULL, NULL, ball1.w, ball1.h);
	basket = loadTexture(imgPath2, true, NULL, NULL, NULL, basket1.w, basket1.h);
	log = loadTexture(imgPath3, true, NULL, NULL, NULL, 1000, 100);

	//判斷進球
	bool in = false;
	bool bomb = false;

	//判斷是否投球
	bool shoot = false;

	//設定亂數種子,讓進球球亂彈的機率為1/5
	int random_number = 5;	//初始化大於2
	srand(time(NULL));


	//Main loop flag
	bool quit = false;
	

	//Event handler
	SDL_Event e;

	MouseState mouseState;

	const Uint8* keystates = SDL_GetKeyboardState(NULL);

	int determine_ball_to_shoot = 0;	//判斷球是否投出(滑鼠)

	bool press_mouse = false;
	bool press_keyboard = false;
	bool choose_side = false;
	bool choose_front = false;
	
	bool retry_or_not = false;
	bool music_play = false;

	//While application is running
	//一直重複執行的迴圈

	while (!quit)
	{
		mouseState = NONE;

		if (press_mouse == false && press_keyboard == false && choose_side == false && choose_front == false)
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(renderer);

			SDL_Rect outlineRect_mouse = { 200, 400,100, 100 };
			SDL_Rect outlineRect_key = { 320, 400,100, 100 };
			SDL_Rect outlineRects = { 600, 400,100, 100 };

			TextData text = loadTextTexture("front", "../fonts/lazy.ttf", 50, 255, 255, 255, SOLID, NULL, NULL, NULL);
			textRender(renderer, text, 250, 330, NULL, NULL, NULL, SDL_FLIP_NONE);

			TextData text7 = loadTextTexture("mouse", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
			textRender(renderer, text7,210, 430, NULL, NULL, NULL, SDL_FLIP_NONE);

			TextData text8 = loadTextTexture("key", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
			textRender(renderer, text8, 340, 430, NULL, NULL, NULL, SDL_FLIP_NONE);

			TextData text2 = loadTextTexture("side", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
			textRender(renderer, text2, 620, 420, NULL, NULL, NULL, SDL_FLIP_NONE);

			SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
			SDL_RenderDrawRect(renderer, &outlineRect_mouse);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
			SDL_RenderDrawRect(renderer, &outlineRects);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
			SDL_RenderDrawRect(renderer, &outlineRect_key);
			SDL_RenderPresent(renderer);
		}


		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			mouseState = NONE;

			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				SDL_GetMouseState(&mouseX, &mouseY);

				//按下滑鼠左鍵時
				if (e.button.button == SDL_BUTTON_LEFT && press_mouse == false && press_keyboard == false && choose_side == false && choose_front == false)
				{
					//GET MOUSE POSITION
					
					if (mouseX > 200 && mouseX < 300 && mouseY> 400 && mouseY < 500)
					{
						choose_front = true;
						press_mouse = true;
						Mix_PlayMusic(music1, 0);
					}

					if (mouseX > 320 && mouseX < 420 && mouseY> 400 && mouseY < 500)
					{
						choose_front = true;
						press_keyboard = true;
						Mix_PlayMusic(music1, 0);
					}

					// basket1.x + 63, basket1.y + 273, WIDTH * 1 / 6, HEIGHT * 1 / 9 
					if (mouseX > 600 && mouseX < 700 && mouseY>  400 && mouseY < 500)
					{
						choose_side = true;
					}				
				}

				//按下滑鼠左鍵時
				//retry主選單

				if (e.button.button == SDL_BUTTON_LEFT && seconds <= 0)
				{

					//retry
					if (mouseX > 200 && mouseX < 300 && mouseY> 400 && mouseY < 500)
					{
						press_mouse = false;
						press_keyboard = false;
						choose_side = false;
						choose_front = false;

						//10個項目初始化
						ball1.vx = 0;
						ball1.vy = 0;
						ball1.px = WIDTH / 2 - WIDTH / 18;
						ball1.py = WIDTH / 2 + 150;
						ball1.h = WIDTH / 12;
						ball1.w = WIDTH / 12;
						ball1.size_mul = 1;
						ball1.alpha = 255;
						ball1.theta = 0;

						basket1.h = WIDTH / 3;
						basket1.w = WIDTH / 3;
						basket1.x = WIDTH / 2 - WIDTH / 4;
						basket1.y = 50;
						basket1.basket = basket1.x + (basket1.w / 2);		//偵測位置隨著籃框位置更動而更動
						basket1.v = 4;

						//Mix_PlayMusic(music1, 0);
						seconds = 60;

						retry_or_not = true;

						//判斷進球
						in = false;
						bomb = false;

						//判斷是否投球
						shoot = false;

						g = 0.0;		//計算之後系統的加速度(帶等加速度公式) 1
						fs = 0.0;	//假設空氣阻力
						theta = 0;
						score = 0;
					}

					//exit
					if (mouseX > 500 && mouseX < 600 && mouseY>  400 && mouseY < 500)
					{
						quit = true;
					}
				}

				//side retry
				if (e.button.button == SDL_BUTTON_LEFT && shoot_time==0)
				{

					//retry
					if (mouseX > 200 && mouseX < 300 && mouseY> 400 && mouseY < 500)
					{
						press_mouse = false;
						press_keyboard = false;
						choose_side = false;
						choose_front = false;

						//10個項目初始化
						ball2.vx = 0;
						ball2.vy = 0;
						ball2.px = 200;
						ball2.py = WIDTH / 2 + 150;
						ball2.h = WIDTH / 12;
						ball2.w = WIDTH / 12;
						ball2.size_mul = 1;
						ball2.alpha = 255;
						ball2.theta = 0;
						shoot_time = 10;

						retry_or_not = true;

						//判斷進球
						in = false;
						bomb = false;

						//判斷是否投球
						shoot = false;

						g = 0.0;		//計算之後系統的加速度(帶等加速度公式) 1
						fs = 0.0;	//假設空氣阻力
						theta = 0;
						score = 0;
					}

					//exit
					if (mouseX > 500 && mouseX < 600 && mouseY>  400 && mouseY < 500)
					{
						quit = true;
					}
				}
			}


			//mouseHandleEvent(&e,&mouseX,&mouseY,ball1,&random_number);
			//判斷同時按下(button event)及移動(mouse event)為拖曳,可以將球射出
			//偵測滑鼠位移狀態
			if (e.type == SDL_MOUSEBUTTONUP && (choose_side == true || choose_front == true) && e.type == SDL_MOUSEMOTION && shoot_time > 0)
			{
				MLBD = false;
			}
			else if (e.type == SDL_MOUSEMOTION && choose_side == true&& shoot_time>0)    //e->type == SDL_MOUSEBUTTONUP)
			{
				//資料顯示_滑鼠座標-起
				printf("滑鼠x座標: %d, 滑鼠y座標 : %d\n", e.button.x, e.button.y);
				bool inside = false; //滑鼠在球的外面
				//資料顯示_滑鼠座標-止

				//如果滑鼠在球的裡面,開始感應滑鼠動作
				if (mouseX >= ball2.px && mouseX <= (ball2.px + ball2.w) && mouseY >= ball2.py && mouseY <= (ball2.py + ball2.h) && (MLBD == true) && shoot_time > 0)
				{
					inside = true;
					printf("滑鼠狀態:拖曳\n");
					determine_ball_to_shoot = 1;

					shoot_or_not = true;
					//用相對座標判斷球投出之情形

					ball2.vx = -(e.motion.xrel);

					//ball.vy = -(e->motion.yrel);
					ball2.vy = -30;
					ball2.size_mul = 0.99;
					g = 1;	//球開始飛才進入重力加速系統

					if (ball2.vx > 30)
					{
						ball2.vx = 30;
					}
					if (ball2.vx < 0)
					{
						ball2.vx = 0;
						ball2.vy = 0;
					}

					fs = (-ball2.vx) / 500.0;
					theta = 10;

					//出手瞬間即判斷是否會進
					random_number = rand() % 10 + 1; //數值在1~10間
					//令1 2為彈出去 其餘為進球 機率為1/5

					//if (random_number <= 2)	out = 1;

				}
			}

			//debug 判斷條件如果是同時"拖曳"跟"放下按鍵時,它沒有反應
			if (e.type == SDL_MOUSEBUTTONUP && (choose_side == true || choose_front == true)&& e.type == SDL_MOUSEMOTION && press_mouse == true && seconds < 60)
			{
				MLBD = false;
			}
			else if (e.type == SDL_MOUSEMOTION && choose_front == true&& press_mouse==true&& seconds<60)    //e->type == SDL_MOUSEBUTTONUP)
			{
				//資料顯示_滑鼠座標-起
				printf("滑鼠x座標: %d, 滑鼠y座標 : %d\n", e.button.x, e.button.y);
				bool inside = false; //滑鼠在球的外面
				//資料顯示_滑鼠座標-止

				//如果滑鼠在球的裡面,開始感應滑鼠動作
				if (mouseX >= ball1.px && mouseX <= (ball1.px + ball1.w) && mouseY >= ball1.py && mouseY <= (ball1.py + ball1.h) && (MLBD == true))//&& //seconds < 60)
				{
					inside = true;
					printf("滑鼠狀態:拖曳\n");
					determine_ball_to_shoot = 1;


					//用相對座標判斷球投出之情形

					ball1.vx = -(e.motion.xrel);
					if (ball1.vx > 30)
					{
						ball1.vx = 30;
					}
					if (ball1.vx < -30)
					{
						ball1.vx = -30;
					}
					//ball.vy = -(e->motion.yrel);
					ball1.vy = -30;
					ball1.size_mul = 0.99;
					g = 1;	//球開始飛才進入重力加速系統
					fs = (-ball1.vx) / 500.0;
					theta = 10;

					//出手瞬間即判斷是否會進
					random_number = rand() % 10 + 1; //數值在1~10間
					//令1 2為彈出去 其餘為進球 機率為1/5

					//if (random_number <= 2)	out = 1;

					if (ball1.w < WIDTH / 14 || ball1.h < WIDTH / 14)
					{
						ball1.size_mul = 1;
					}

				}
			}

			//偵測滑鼠按鍵狀態
			//按下按鍵
			if (e.type == SDL_MOUSEBUTTONDOWN && (choose_side == true || choose_front == true))
			{
				//按下滑鼠左鍵時
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					//GET MOUSE POSITION
					SDL_GetMouseState(&mouseX, &mouseY);
					MLBD = true;
					//資料顯示_滑鼠座標
					printf("按下滑鼠左鍵\n");
				}
			}
			
			//放開按鍵
			if (e.type == SDL_MOUSEBUTTONUP && (choose_side == true || choose_front == true))
			{
				//if 放開滑鼠左鍵
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					MLBD = false;

					//資料顯示
					printf("放開滑鼠左鍵\n");
				}
			}
			

		}

		if (press_keyboard == true)
		{
			//可以改成 選單選鍵盤控制時執行
			if (keystates[SDL_SCANCODE_RIGHT])
			{
				ball1.px += 15;
				//到達邊界停止
				if (ball1.px + ball1.w / 2 > WIDTH)
				{
					ball1.px -= 15;
				}
			}
			else if (keystates[SDL_SCANCODE_LEFT])
			{
				ball1.px -= 15;
				//到達邊界停止
				if (ball1.px + ball1.w / 2 < 0)
				{
					ball1.px += 15;
				}
			}

			else if (keystates[SDL_SCANCODE_UP] && seconds < 60)
			{

				ball1.vy = -24;
				ball1.size_mul = 0.99;
				g = 1;	//球開始飛才進入重力加速系統
				theta = 10;

				//出手瞬間即判斷是否會進
				random_number = rand() % 10 + 1; //數值在1~10間
				//令1 2為彈出去 其餘為進球 機率為1/5

				//if (random_number <= 2)	out = 1;

				if (ball1.w < WIDTH / 14 || ball1.h < WIDTH / 14)
				{
					ball1.size_mul = 1;
				}
			}
		}

		if (choose_front == true)
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(renderer);

			/*
			//音樂結束 開始倒數(只會設定一次)
			if (Mix_PlayingMusic() == 0 && (startTime == NULL|| retry_or_not == true))
			{
				startTime = clock();
			}

			//要一直讀的 倒數功能
			if (Mix_PlayingMusic() == 0 && (startTime != NULL || retry_or_not == true) && seconds > 0)
			{
				countTime = clock();
				milliseconds = countTime - startTime;
				seconds = 60 - (milliseconds / CLOCKS_PER_SEC);	//開始到現在經過的時間	
				retry_or_not = false;
			}
			*/
			//snprintf(timeb, sizeof(timeb), "%d", seconds);	//一開始就顯示


			//籃框移動的code
			basket1.x += basket1.v;
			if (basket1.x + basket1.w > WIDTH || basket1.x < 0)
				basket1.v = -basket1.v;

			//球的大小速度變化
			ball1.px += ball1.vx;
			ball1.py += ball1.vy;
			ball1.w = ball1.size_mul * ball1.w;
			ball1.h = ball1.size_mul * ball1.h;

			ball1.vy += g;	//速度+g 讓球上升時產生加速度效果
			ball1.theta += theta;
			ball1.vx += fs;	//考量空氣阻力阻礙球的活動


			if (ball1.vx > -2 && ball1.vx < 2 && determine_ball_to_shoot == 1)
			{
				ball1.vx = 0;
				fs = 0;
				determine_ball_to_shoot = 0;

			}



			if (ball1.w < WIDTH / 14 || ball1.h < WIDTH / 14)
			{
				ball1.size_mul = 1;
			}


			//球開始下降時判定進球與否
			if (ball1.vy > 0 && ball1.py <= basket1.y + 180 && ball1.py >= basket1.y + 160 && in == false)
			{
				//進球判定(x座標範圍判定)
				if ((ball1.px >= basket1.x + 80) && (ball1.px <= basket1.x + 180) && random_number > 2 && seconds > 15)
				{
					Mix_PlayChannel(-1, effect1, 0);	//兩分球音效
					score += 2;
					ball1.alpha = 120;	//製造穿過去的感覺
					in = true;
				}
				else if ((ball1.px >= basket1.x + 80) && (ball1.px <= basket1.x + 180) && random_number > 2 && seconds <= 15)
				{
					Mix_PlayChannel(-1, effect2, 0);	//三分球音效
					score += 3;
					ball1.alpha = 120;	//製造穿過去的感覺
					in = true;
				}
				else if ((ball1.px >= basket1.x + 80) && (ball1.px <= basket1.x + 180) && random_number <= 2 && bomb == false)
				{
					bomb = true;	//讓它只彈一次
					ball1.vy = -5;
				}
				else if (ball1.px >= basket1.x + 60 && ball1.px < basket1.x + 80)
				{
					ball1.vy = -5;
				}
				else if (ball1.px > basket1.x + 180 && ball1.px <= basket1.x + 210)
				{
					ball1.vy = -5;
				}
				theta = 0;
			}

			if (ball1.py > basket1.y + 73 && ball1.py <= basket1.y + 175 && ball1.vy > 0 && (ball1.px > basket1.x + basket1.w / 2 - 170) && (ball1.px < basket1.x + basket1.w / 2 - 50))
			{
				SDL_RenderDrawLine(renderer, basket1.x + 180, basket1.y + 160, basket1.x + 130, basket1.y + 295);
				SDL_RenderDrawLine(renderer, basket1.x + 160, basket1.y + 160, basket1.x + 160, basket1.y + 295);
				SDL_RenderDrawLine(renderer, basket1.x + 160, basket1.y + 165, basket1.x + 175, basket1.y + 245);
				SDL_RenderDrawLine(renderer, basket1.x + 160, basket1.y + 163, basket1.x + 185, basket1.y + 220);
				SDL_RenderDrawLine(renderer, basket1.x + 65, basket1.y + 163, basket1.x + 80, basket1.y + 218);
				SDL_RenderDrawLine(renderer, basket1.x + 165, basket1.y + 163, basket1.x + 90, basket1.y + 245);
				SDL_RenderDrawLine(renderer, basket1.x + 155, basket1.y + 166, basket1.x + 100, basket1.y + 290);
				SDL_RenderDrawLine(renderer, basket1.x + 160, basket1.y + 163, basket1.x + 125, basket1.y + 290);
				SDL_Delay(10);
			}

			//一直顯示(font面板要印出來的東西)
			//printf("present score:%d\n", score);
			snprintf(scorec, sizeof(scorec), "%d", score);//轉int to char[]

			//當前一顆球向下墜落至消失或消失於屏幕時,重置一顆新的
			//加了
			if (ball1.py > HEIGHT / 2 + 150)
			{
				ball1.py = WIDTH / 2 + 150;
				ball1.w = WIDTH / 12;
				ball1.h = WIDTH / 12;
				ball1.vx = 0;
				ball1.vy = 0;			//速度歸零
				ball1.theta = 0;		//停止炫球
				ball1.alpha = 255;		//回復原樣
				in = false;			//重製in旗標
				bomb = false;		//重製bomb旗標
				random_number = 5;
				g = 0;
				theta = 0;
			}

			if (ball1.px + ball1.w > WIDTH || ball1.px < 0)
			{
				ball1.vx = -ball1.vx;
			}


			if (basket1.v > 0 && score >= 20 * i)
			{
				basket1.v += 2;
				i++;
			}
			if (basket1.v < 0 && score >= 20 * i)
			{
				basket1.v -= 2;
				i++;
			}

			//如果game over,就只顯示game over
			
			if (seconds <= 0 && score <= 30*ii)
			{
				// Clear screen
				//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
				//SDL_RenderClear(renderer);
			
				//Mix_PlayMusic(music2, 0);

				//音樂結束 開始倒數(只會設定一次)
				
		

				TextData text4 = loadTextTexture("GAME OVER", "../fonts/lazy.ttf", 80, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text4, 220, 250, NULL, NULL, NULL, SDL_FLIP_NONE);


				//SDL_Delay(1000);	//delay 5s


				//返回或結訴的選單

				// Clear screen
				

				SDL_Rect outlineRect = {200, 400,100, 100};
				SDL_Rect outlineRects = {500, 400,100, 100 };

				TextData text5 = loadTextTexture("retry", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text5, 210,420, NULL, NULL, NULL, SDL_FLIP_NONE);

				TextData text6 = loadTextTexture("exit", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text6, 520, 420, NULL, NULL, NULL, SDL_FLIP_NONE);

				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(renderer, &outlineRect);
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(renderer, &outlineRects);
				
			}
			else
			{
				if (seconds <= 0 && score > 30*ii)
				{
					seconds = 60;
					startTime = NULL;
					ii++;
				}

				if (Mix_PlayingMusic() == 0 && (startTime == NULL || retry_or_not == true))
				{
					startTime = clock();
				}

				//要一直讀的 倒數功能
				if (Mix_PlayingMusic() == 0 && (startTime != NULL || retry_or_not == true) && seconds > 0)
				{
					countTime = clock();
					milliseconds = countTime - startTime;
					seconds = 60 - (milliseconds / CLOCKS_PER_SEC);	//開始到現在經過的時間	
					retry_or_not = false;
				}

				// Draw basket
				drawbasket(basket1, ball1);

				snprintf(timeb, sizeof(timeb), "%d", seconds);
				TextData text = loadTextTexture("current score", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				TextData text2 = loadTextTexture(scorec, "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				TextData text3 = loadTextTexture(timeb, "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);

				//drawball(&ball_theta);

				textRender(renderer, text, basket1.x + 30, basket1.y + 5, NULL, NULL, NULL, SDL_FLIP_NONE);
				textRender(renderer, text2, basket1.x + 130, basket1.y + 35, NULL, NULL, NULL, SDL_FLIP_NONE);
				textRender(renderer, text3, basket1.x + 110, basket1.y + 90, NULL, NULL, NULL, SDL_FLIP_NONE);


				//imgRender(renderer, basket, basketx, baskety, basketw, basketh);


				setTextureAlpha(ball.texture, ball1.alpha);
				imgRender(renderer, log, 0, WIDTH / 2 + 130, 800, 100, NULL, NULL, NULL);
				imgRender(renderer, ball, ball1.px, ball1.py, ball1.w, ball1.h, ball1.w / 2, ball1.h / 2, ball1.theta);


				//SDL_UpdateWindowSurface(window);
			}

			// Update screen
			SDL_RenderPresent(renderer);

		}

		if (choose_side == true)
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(renderer);

			if (shoot_time <= 0)
			{
				snprintf(scorec, sizeof(scorec), "%d%c", score*10,37);
				TextData text2 = loadTextTexture(scorec, "../fonts/lazy.ttf", 100, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text2, 300,200, NULL, NULL, NULL, SDL_FLIP_NONE);


				SDL_Rect outlineRect = { 200, 400,100, 100 };
				SDL_Rect outlineRects = { 500, 400,100, 100 };

				TextData text5 = loadTextTexture("retry", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text5, 210, 420, NULL, NULL, NULL, SDL_FLIP_NONE);

				TextData text6 = loadTextTexture("exit", "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text6, 520, 420, NULL, NULL, NULL, SDL_FLIP_NONE);

				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(renderer, &outlineRect);
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(renderer, &outlineRects);
			}
			else
			{
				snprintf(shoot_timeb, sizeof(shoot_timeb), "%d", shoot_time);
				TextData text = loadTextTexture(shoot_timeb, "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text, 380, 100, NULL, NULL, NULL, SDL_FLIP_NONE);

				snprintf(scorec, sizeof(scorec), "%d", score);
				TextData text2 = loadTextTexture(scorec, "../fonts/lazy.ttf", 30, 255, 255, 255, SOLID, NULL, NULL, NULL);
				textRender(renderer, text2, 480, 100, NULL, NULL, NULL, SDL_FLIP_NONE);

				//籃框
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0xFF);
				SDL_RenderDrawLine(renderer, 685, 250, 685, 655);
				SDL_RenderDrawLine(renderer, 620, 220, 770, 310);
				SDL_RenderDrawLine(renderer, 620, 90, 770, 180);
				SDL_RenderDrawLine(renderer, 620, 90, 620, 220);
				SDL_RenderDrawLine(renderer, 770, 180, 770, 310);
				arcColor(renderer, 645, 270, 45, 210, 329, 0xFFFFFFFF);
				arcColor(renderer, 645, 240, 40, 20, 167, 0xFFFFFFFF);

				//球的大小速度變化
				ball2.px += ball2.vx;
				ball2.py += ball2.vy;

				ball2.vy += g;	//速度+g 讓球上升時產生加速度效果
				ball2.theta += theta;
				ball2.vx += fs;	//考量空氣阻力阻礙球的活動


				//考量空氣阻力讓球靜止
				if (ball2.vx > -2 && ball2.vx < 2 && determine_ball_to_shoot == 1)
				{
					ball2.vx = 0;
					fs = 0;
					determine_ball_to_shoot = 0;
				}

				if (ball2.px + ball2.w / 2 > 695 &&ball2.px + ball2.w / 2 < 705 && ball2.py >= 140 && ball2.py <= 220)
				{
					ball2.vx = -ball2.vx;
				}

				if (/*ball2.px + ball2.w > WIDTH ||*/ ball2.px < 0)
				{
					ball2.vx = -ball2.vx;
				}

				


				//當前一顆球向下墜落至消失或消失於屏幕時,重置一顆新的
				//加了
				if (ball2.py > HEIGHT / 2 + 150)
				{
					ball2.px = rand() % 200 + 100;
					ball2.py = WIDTH / 2 + 150;
					ball2.w = WIDTH / 12;
					ball2.h = WIDTH / 12;
					ball2.vx = 0;
					ball2.vy = 0;			//速度歸零
					ball2.theta = 0;		//停止炫球
					ball2.alpha = 255;		//回復原樣
					in = false;			//重製in旗標
					bomb = false;		//重製bomb旗標

					random_number = 5;
					theta = 0;
					g = 0;
				}

				random_number = rand() % 10 + 1;


				//球開始下降時判定進球與否
				if (ball2.vy > 0 && ball2.py >= 200 && ball2.py <= 250 && in == false)
				{

					
					//進球判定(x座標範圍判定)
					if ((ball2.px >= 600) && (ball2.px <= 680) && random_number > 2)
					{
						//Mix_PlayChannel(-1, effect1, 0);	//兩分球音效
						score ++;
						ball2.alpha = 120;	//製造穿過去的感覺
						in = true;
					}

					else if ((ball2.px >= 580) && (ball2.px <= 600) && random_number <= 2 && bomb == false)
					{
						bomb = true;	//讓它只彈一次
						ball2.vy = -5;
					}
					else if (ball2.px >= 680 && ball2.px < 700)
					{
						ball2.vy = -5;
					}

					if (shoot_or_not == true)
					{
						shoot_time--;
						shoot_or_not = false;
					}
					//theta = 0;
				}


				setTextureAlpha(ball.texture, ball2.alpha);
				imgRender(renderer, ball, ball2.px, ball2.py, ball2.w, ball2.h, ball2.w / 2, ball2.h / 2, ball2.theta);


				//SDL_UpdateWindowSurface(window);

			}
			// Update screen
			SDL_RenderPresent(renderer);
		}
	}

	//Free resources and close SDL
	closeSDL();

	//system("pause");
	return 0;
}
