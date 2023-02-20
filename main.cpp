#include <exception>
#include <string>
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <queue>
#include "notes.hpp"
#ifdef EMCXX
#include <emscripten.h>
#endif // EMCXX

class InitError : public std::exception
{
    std::string msg;
public:
    InitError();
    InitError( const std::string & );
    virtual ~InitError() throw();
    virtual const char * what() const throw();
};

InitError::InitError() :
    exception(),
    msg( SDL_GetError() )
{
}

InitError::InitError( const std::string & m ) :
    exception(),
    msg( m )
{
}

InitError::~InitError() throw()
{
}

SDL_Color board[32][20];


const char * InitError::what() const throw()
{
    return msg.c_str();
}

class SDL
{
    SDL_Window * m_window;
    SDL_Renderer * m_renderer;
public:
    SDL( Uint32 flags = 0 );
    virtual ~SDL();
    void draw();
};

SDL::SDL( Uint32 flags )
{
    if ( SDL_Init( flags ) != 0 )
        throw InitError();
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT,"#canvas");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC,"1");
    if ( SDL_CreateWindowAndRenderer( 720, 576, SDL_WINDOW_SHOWN,
                                      &m_window, &m_renderer ) != 0 )
        throw InitError();

	Mix_OpenAudio(48000, AUDIO_S16LSB, 2, 256);
	Uint16 format;
	int channels;
	extern int sfreq;
	Mix_QuerySpec(&sfreq, &format, &channels);

    SDL_SetWindowTitle(m_window,"Breakout.");
    SDL_RenderSetLogicalSize(m_renderer,720,576);
}

SDL::~SDL()
{
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_Quit();
}
SDL sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER |SDL_INIT_AUDIO);

int score;


void init_game_board(){
    for(int y=5;y<28;++y){
        board[y][0]={0x84,0x84,0x84,0xff};
        board[y][19]={0x84,0x84,0x84,0xff};

    }
    board[28][0] = {0x36,0x96,0x76};
    board[28][19] = {0xc2,0x3c,0x3c};
    for(int x=0;x<20;++x){
        board[4][x]={0x84,0x84,0x84,0xff};
        board[3][x]={0x84,0x84,0x84,0xff};
    }
    for(int x=1;x<19;++x){
        board[9][x]={0xff,0x00,0x00,0xff};
        board[10][x]={0xc0,0x61,0x2f,0xff};
        board[11][x]={0xad,0x6f,0x26,0xff};
        board[12][x]={0xa0,0xa0,0x20,0xff};
        board[13][x]={0x3d,0x97,0x3c,0xff};
        board[14][x]={0x37,0x3d,0xc1,0xff};
    }
}

int px=360;

SDL_Rect paddle;

int
SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}

int bx=10000,by=10000,br=6,bxd=0,byd=0;
TTF_Font *font;
void drawText(SDL_Renderer *screen, char *str, int x, int y, int sz, SDL_Color fgC, SDL_Color bgC, int leftanchor) {

	TTF_Init();

	font = TTF_OpenFont("./font.ttf", 30);
	if (!font) {
		printf("[ERROR] TTF_OpenFont() Failed with: %s\n", TTF_GetError());
		exit(2);
	}

	// SDL_Surface* textSurface = TTF_RenderText_Solid(font, string, fgC);
	// // aliased glyphs
	SDL_Surface *textSurface = TTF_RenderText_Solid(font, str, fgC); // anti-aliased glyphs
	SDL_Rect textLocation = {(int)(x - sz * leftanchor * (strlen(str) - 1)), y, (int)(sz * (strlen(str))), sz};
	SDL_Texture *t = SDL_CreateTextureFromSurface(screen, textSurface);

	SDL_RenderCopy(screen, t, NULL, &textLocation);

	SDL_DestroyTexture(t);
	SDL_FreeSurface(textSurface);
	TTF_CloseFont(font);

	TTF_Quit();
	// printf("[ERROR] Unknown error in drawText(): %s\n", TTF_GetError());
	// return 1;
}
int balls=5;
int games=0;
void SDL::draw()
{
    // Clear the window with a black background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    SDL_Rect brick={0,0,36,18};

    for(int y=0;y<32;++y){
        for(int x=0;x<20;++x){
            SDL_SetRenderDrawColor(m_renderer,board[y][x].r,board[y][x].g,board[y][x].b,board[y][x].a);
            brick.x=x*brick.w;
            brick.y=y*brick.h;
            SDL_RenderFillRect(m_renderer,&brick);
        }
    }

    paddle = {px-40,504,40*2,9};

    SDL_SetRenderDrawColor(m_renderer,0xc2,0x3c,0x3c,0xff);
    SDL_RenderFillRect(m_renderer, &paddle);
    SDL_RenderFillCircle(m_renderer,bx,by,br);

    char s[10];
    sprintf(s,"%03d",score);
    drawText(m_renderer,s, 100,0,50, {0x84,0x84,0x84,0xff}, {0,0,0,0},0);
    sprintf(s,"%02d",balls);
    drawText(m_renderer,s, 380,0,50, {0x84,0x84,0x84,0xff}, {0,0,0,0},1);
    sprintf(s,"%02d",games);
    drawText(m_renderer,s, 560,0,50, {0x84,0x84,0x84,0xff}, {0,0,0,0},1);



    // Show the window
    SDL_RenderPresent( m_renderer );

}

std::queue<struct songnote> s_eff_queue[4];

void s_beep(float len_ims, unsigned int freq, unsigned int channel = 0, double vol = 0.5) {
    s_eff_queue[channel].push(DSN(SQR, freq, (unsigned int)(len_ims / 1000 * 60), 50, vol));
}

int running=1;

bool intersects(SDL_Rect r1, SDL_Rect r2) {
    return !((r2.x > (r1.x+r1.w))
        || ((r2.x+r2.w) < r1.x)
        || ((r2.y+r2.h) < (r1.y))
        || (r2.y) > (r1.y+r1.h));
}
    int ret,prev,prev2;
int dest = 360;

int biasedRandom(int rec=0){

    ret = rand()%17;

    if(ret == prev || ret == prev2){
        return biasedRandom(1);
    }
    if(!rec){
        prev2 = prev;

        prev = ret;
    }
    return ret;
}


void ai_calc_dest(){
    int xd = bxd;
    int cx = bx+bxd;
    int cy = by+byd;
    if (byd < 0) return;
    while(cy < 504){
        if((((cx-br) < 36 )|| ((cx+br) > 684))) xd = -xd;

        cx += xd;
        cy += byd;

    }

    dest = cx + biasedRandom()*2* ((biasedRandom()%2)?1:-1);

}


int times;

void handle_colours(int x,int y){

        switch(y){
                        case 9:
                            s_beep(100,A3S);
                            score += 7;
                        break;

                        case 10:
                            s_beep(100,G3);
                            score += 7;
                        break;

                        case 11:
                            s_beep(100,D3S);
                            if(abs(byd)<9){
                                byd=byd*3/2;
                                std::cout << byd << std::endl;
                            }
                            score += 4;
                        break;

                        case 12:
                            s_beep(100,C3S);
                            score += 4;
                        break;

                        case 13:
                            s_beep(100,A2S);
                            score += 1;
                        break;

                        case 14:
                            s_beep(100,F2S);
                            score += 1;
                        break;


        }

            board[y][x]={0,0,0,0};


}

void ball_tick(){

    SDL_Rect brick={0,0,36,18};

    SDL_Rect ball = {bx-br,by-br,br*2,br*2};
    int  wascollision=0;
    for(int y=0;y<32;++y){
        for(int x=0;x<20;++x){
            if(board[y][x].a != 0){
                brick.x=x*brick.w;
                brick.y=y*brick.h;
                SDL_Rect ballytest = {bx-br,by-br+byd,br*2,br*2};
                if(intersects(ballytest,brick)){
                    byd = -byd;
                    by += byd;
                    if(board[y][x].r == 0x84&&board[y][x].g == 0x84){
                        s_beep(100,D4);
                    }
                    else{
                        handle_colours(x,y);
                    }
                    ai_calc_dest();
                }
                SDL_Rect ballxtest = {bx-br+bxd,by-br,br*2,br*2};
                if(intersects(ballxtest,brick)){
                    bxd = -bxd;
                    bx += bxd;
                    if(board[y][x].r == 0x84&&board[y][x].g == 0x84){
                        s_beep(100,C5);

                    }
                    else{
                        handle_colours(x,y);
                    }
                    ai_calc_dest();
                }

            }

        }
    }
    paddle = {px-40,504,40*2,9};

    if(intersects(ball,paddle)){
        bxd = (px-bx)/-6;
        s_beep(120,D4);
        byd = -byd;
       if ((score % 432) == 0&& score !=0){
        init_game_board();
        ++games;
        }
    }

    bx+=bxd;
    by+=byd;

}

void audio_tick() {
	int inc = 0;

	for (int i = 0; i < 4; ++i) {
		if (s_eff_queue[i].size() != 0) {
			if (s_eff_queue[i].front().playin == 0 && s_eff_queue[i].front().durrcount == 0) {
				s_eff_queue[i].front().durrcount = s_eff_queue[i].front().durr;
				s_eff_queue[i].front().playin = 1;
				unsigned int len;
				switch (s_eff_queue[i].front().type) {
				case SQR:
					s_eff_queue[i].front().buf = sqrtonegen(s_eff_queue[i].front().pitch, s_eff_queue[i].front().specval, s_eff_queue[i].front().vol, &len);
					break;
				case TRI:
					s_eff_queue[i].front().buf = tritonegen(s_eff_queue[i].front().pitch, s_eff_queue[i].front().vol, &len);
					break;
				case NOISE:
					s_eff_queue[i].front().buf = randtonegen(s_eff_queue[i].front().pitch, s_eff_queue[i].front().vol, &len, s_eff_queue[i].front().specval);
				}
				s_eff_queue[i].front().c = Mix_QuickLoad_RAW((Uint8 *)s_eff_queue[i].front().buf, len);
				Mix_PlayChannel(-1, s_eff_queue[i].front().c, -1);
			}
			--s_eff_queue[i].front().durrcount;
			if (s_eff_queue[i].front().durrcount <= 0) {
				Mix_FreeChunk(s_eff_queue[i].front().c);
				free(s_eff_queue[i].front().buf);
				s_eff_queue[i].pop();
			}
		}
	}

}

int ai_r,ai_l, ai_enable = 0;

void ai_tick(){

        if(px < (dest-2)){

            ai_r = 1;
        }
        else ai_r = 0;

        if((px) > dest+2){

            ai_l = 1;

        }
        else ai_l = 0;
}
void game_tick(){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        switch(e.type){
        case SDL_QUIT:
            running=0;
            break;

        case SDL_KEYDOWN:
            if(e.key.keysym.sym == SDLK_r){
                ai_enable = !ai_enable;
            }
            if(e.key.keysym.sym == SDLK_s){
                if(balls == 0){
                    balls = 5;
                    score = 0;
                    games = 0;
                    init_game_board();
                }
                if(balls && ((bx <=0  || by <=0 )|| (bx >= 720 || by >= 576))){
                    --balls;
                    bx=650;
                    by=504;
                    bxd=-3;
                    byd=-6;
                    s_beep(100,C5);
                }
            }
            break;
        }

    }

    const Uint8 *inp=SDL_GetKeyboardState(NULL);

    if(inp[SDL_SCANCODE_A] || (ai_enable && ai_l)){
        if(px>=0)
            px-=15;
    }
    if(inp[SDL_SCANCODE_D] || (ai_enable && ai_r)){
        if(px<=720)
        px+=15;
    }

    ball_tick();
    ai_tick();
    audio_tick();
    sdl.draw();
}

int main( int argc, char * argv[] )
{
    srand(time(0));
    init_game_board();
    #ifdef EMCXX
        emscripten_set_main_loop(game_tick,60,1);
    #else // EMCXX
        while(running){
            game_tick();
        }
    #endif
    return 1;
}
