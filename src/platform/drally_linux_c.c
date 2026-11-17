#include "drally.h"
#include "drally_display.h"


#pragma pack(1)
typedef struct textbit {
	unsigned char 	ascii;
	unsigned char 	fg:4;
	unsigned char 	bg:3;
} textbit;

unsigned int INT8_FRAME_COUNTER = 0;
extern unsigned int ___60458h;
extern textbit * B800;
extern unsigned char * VGA13_ACTIVESCREEN;
extern unsigned char VGA13_ACTIVESCREEN_2[];
extern unsigned char VESA101_ACTIVESCREEN[];

unsigned int Ticks = 0;
unsigned int VRetraceTicks = 0;

void IO_Loop(void);
void __VGA13_PRESENTSCREEN__(void);
void __VESA101_PRESENTSCREEN__(void);
void __PRESENTSCREEN__(void);
__BYTE__ dRally_Keyboard_popLastKey();

static struct GX {
	int 			ActiveMode;
	int 			WindowMode;
	struct {
		SDL_Surface * Surface;
	} VGA13;
	struct {
		SDL_Surface * Surface;
	} VESA101;
	SDL_Surface * 	Surface;
	SDL_Window * 	Window;
	SDL_Renderer * 	Renderer;
	SDL_Texture * 	Texture;
} GX = {0};


extern __DWORD__ ___60441h;
extern __DWORD__ ___6045ch;
extern __BYTE__ ___60446h;
extern void (*___6044ch)(void);
void __VRETRACE_WAIT_IF_INACTIVE(void);

static void IRQ0_TimerISR(void){

	if((___6045ch != 0)&&(___60441h == 0)) __VRETRACE_WAIT_IF_INACTIVE();
	INT8_FRAME_COUNTER++;
	if(___60446h == 1) ___6044ch();
}

int skip;

#if defined(DR_SWITCH)

#include "frame_clock.h"
#include "switch_config.h"

/* 1 when the picture waits for the screen (vsync). The retrace waits below are then not needed:
   presenting is the wait. */
static int DR_VSYNC = 0;
static FrameClock SW_Clock;

unsigned int __GET_FRAME_COUNTER(void){

	const unsigned rate = (___60458h != 0) ? ___60458h : 70;
	const unsigned frame_us = 1000000u/rate;
	const Uint64 now_us = SDL_GetPerformanceCounter()*1000000ULL/SDL_GetPerformanceFrequency();
	const int frames = FrameClock_advance(&SW_Clock, now_us, frame_us, 7);

	if(frames > 0){

		/* frames the game missed are counted at once, as the original timer interrupt did */
		skip = frames-1;
		INT8_FRAME_COUNTER += skip;
		IRQ0_TimerISR();
	}
	else if((FrameClock_remaining(&SW_Clock, now_us, frame_us) > 3000)) SDL_Delay(1);

	IO_Loop();

	return INT8_FRAME_COUNTER;
}

#else

unsigned int __GET_FRAME_COUNTER(void){

	unsigned int NewTicks;
	unsigned int FrameMs = 1000/___60458h;// - 1;


	NewTicks = SDL_GetTicks()-Ticks;	
	
	if(NewTicks < (FrameMs-3)) SDL_Delay(1);
	else if(NewTicks >= FrameMs){

		Ticks = SDL_GetTicks();

		skip = NewTicks/FrameMs - 1;
		INT8_FRAME_COUNTER += skip;
		IRQ0_TimerISR();

		//if(!skip) __PRESENTSCREEN__();
	}

	IO_Loop();
	
	return INT8_FRAME_COUNTER;
}

#endif // DR_SWITCH

void __VRETRACE_WAIT_FOR_START(void){
	
#if defined(DR_SWITCH)
	if(DR_VSYNC) return;	// presenting waits for the screen already
#endif
	unsigned int FrameMs = 1000/___60458h;// - 1;

	VRetraceTicks = SDL_GetTicks();
	VRetraceTicks %= FrameMs;
	
	//if(VRetraceTicks) SDL_Delay(FrameMs - VRetraceTicks);
	//SDL_Delay(1);
	SDL_Delay(FrameMs - VRetraceTicks);
}

void __VRETRACE_WAIT_IF_INACTIVE(void){

	unsigned int FrameMs = 1000/___60458h;//1000/70;

	IO_Loop();
#if defined(DR_SWITCH)
	if(DR_VSYNC) return;	// presenting waits for the screen already
#endif

	VRetraceTicks = SDL_GetTicks();
	VRetraceTicks %= FrameMs;
	if(VRetraceTicks > (FrameMs - 3)){
		
		SDL_Delay(FrameMs - VRetraceTicks);
	}
}

void __TIMER_SET_TIMER(void){

	//Ticks = SDL_GetTicks();
}

void __WAIT_5(void){

	unsigned int tmp = 5;

	tmp *= ___60458h;
	tmp += __GET_FRAME_COUNTER();

	while((__GET_FRAME_COUNTER() < tmp) && !dRally_Keyboard_popLastKey());
}




void __VESA101_SETMODE();
void __DISPLAY_SET_PALETTE_COLOR(int b, int g, int r, int n);


#if defined(DR_SWITCH)

/* One texture is kept for the whole run and refilled each frame from the 8-bit picture and its
   palette, instead of creating and destroying a texture 70 times a second. */
static SDL_Texture * SW_Texture = NULL;
static int SW_TextureW = 0, SW_TextureH = 0;
static Uint32 SW_Pixels[640*480];

/* Smart scaling: the picture is enlarged by a whole number with nearest-neighbour (every pixel
   stays a sharp block), into a bigger texture that the renderer then fits to the screen with
   bilinear filtering. Only the edges between blocks are smoothed. */
static SDL_Texture * SW_Target = NULL;
static int SW_TargetW = 0, SW_TargetH = 0;

static void sw_make_textures(int w, int h){

	const int smart = SwitchConfig.scaling == 1;
	int ow = 1280, oh = 720, k = 1;

	if(SW_Texture) SDL_DestroyTexture(SW_Texture);
	if(SW_Target) SDL_DestroyTexture(SW_Target);
	SW_Target = NULL;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, smart ? "0" : "2");
	SW_Texture = SDL_CreateTexture(GX.Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	SW_TextureW = w;
	SW_TextureH = h;

	if(smart){

		if(GX.Renderer) SDL_GetRendererOutputSize(GX.Renderer, &ow, &oh);
		k = (oh+h-1)/h;					/* the smallest whole number that is not smaller than the screen */
		if(k < 1) k = 1;
		while((k > 1) && ((k*w > 4096) || (k*h > 4096))) k--;

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		SW_Target = SDL_CreateTexture(GX.Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, k*w, k*h);
		SW_TargetW = k*w;
		SW_TargetH = k*h;
		printf("[dRally.DISPLAY] smart scaling %dx%d x%d -> %dx%d %s\n", w, h, k, ow, oh, SW_Target ? "on" : "unavailable, using plain scaling");
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
}

void __PRESENTSCREEN__(void){

	if(GX.ActiveMode){

		const int w = GX.Surface->w;
		const int h = GX.Surface->h;
		const SDL_Palette * pal = GX.Surface->format->palette;
		const Uint8 * src = (const Uint8 *)GX.Surface->pixels;
		Uint32 lut[256];
		int x, y;

		for(x = 0; x < 256; x++){

			const SDL_Color c = pal->colors[x];
			lut[x] = 0xff000000u|((Uint32)c.r<<16)|((Uint32)c.g<<8)|c.b;
		}

		for(y = 0; y < h; y++){

			Uint32 * dst = SW_Pixels+y*w;
			const Uint8 * line = src+y*GX.Surface->pitch;

			for(x = 0; x < w; x++) dst[x] = lut[line[x]];
		}

		if(!SW_Texture || SW_TextureW != w || SW_TextureH != h){

			sw_make_textures(w, h);
		}

		SDL_UpdateTexture(SW_Texture, NULL, SW_Pixels, w*sizeof(Uint32));
		if(SW_Target && (SDL_SetRenderTarget(GX.Renderer, SW_Target) == 0)){

			SDL_RenderCopy(GX.Renderer, SW_Texture, NULL, NULL);
			SDL_SetRenderTarget(GX.Renderer, NULL);
			SDL_RenderClear(GX.Renderer);
			SDL_RenderCopy(GX.Renderer, SW_Target, NULL, NULL);
		}
		else {

			SDL_RenderClear(GX.Renderer);
			SDL_RenderCopy(GX.Renderer, SW_Texture, NULL, NULL);
		}
		SDL_RenderPresent(GX.Renderer);
	}
}

void SwitchLocal_present(const unsigned char *pixels, int width, int height){
	static SDL_Texture *texture;
	static int textureHeight;
	int logicalWidth, logicalHeight;
	Uint32 palette[256];

	if(!GX.ActiveMode) return;
	if(!texture || textureHeight != height){
		if(texture) SDL_DestroyTexture(texture);
		texture = SDL_CreateTexture(GX.Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		textureHeight = height;
	}
	if(!texture) return;
	for(int i = 0; i < 256; ++i){
		const SDL_Color c = GX.Surface->format->palette->colors[i];
		palette[i] = 0xff000000u | ((Uint32)c.r << 16) | ((Uint32)c.g << 8) | c.b;
	}
	for(int i = 0; i < width*height; ++i) SW_Pixels[i] = palette[pixels[i]];
	if(SDL_UpdateTexture(texture, NULL, SW_Pixels, width*sizeof(Uint32)) != 0) return;

	SDL_RenderGetLogicalSize(GX.Renderer, &logicalWidth, &logicalHeight);
	SDL_RenderSetLogicalSize(GX.Renderer, 0, 0);
	SDL_RenderClear(GX.Renderer);
	SDL_RenderCopy(GX.Renderer, texture, NULL, NULL);
	SDL_RenderPresent(GX.Renderer);
	SDL_RenderSetLogicalSize(GX.Renderer, logicalWidth, logicalHeight);
}

#else

void __PRESENTSCREEN__(void){

	if(GX.ActiveMode){

		GX.Texture = SDL_CreateTextureFromSurface(GX.Renderer, GX.Surface);
		SDL_RenderCopy(GX.Renderer, GX.Texture, NULL, NULL);
		SDL_RenderPresent(GX.Renderer);
		SDL_DestroyTexture(GX.Texture);
		GX.Texture = NULL;
	}
}

#endif // DR_SWITCH

void __VGA13_PRESENTSCREEN__(void){

    __PRESENTSCREEN__();
}

void __VESA101_PRESENTSCREEN__(void){

	__PRESENTSCREEN__();
}

void __DISPLAY_SET_PALETTE_COLOR(int b, int g, int r, int n){

    SDL_Color col;

    col.r = (r<<2)|(r>>4);
    col.g = (g<<2)|(g>>4);
    col.b = (b<<2)|(b>>4);

	if(GX.ActiveMode) SDL_SetPaletteColors(GX.Surface->format->palette, &col, n, 1);
}

void DISPLAY_CLEAR_PALETTE(void){

	int 	n;

	n = -1;
	while(++n < 0x100) __DISPLAY_SET_PALETTE_COLOR(0, 0, 0, n);
}

void dRally_Display_init(int mode){

	SDL_ShowCursor(SDL_DISABLE);
	SDL_DisableScreenSaver();

	if(!GX.VGA13.Surface){

		switch(mode){
		case W_SHRINK:
			GX.WindowMode = W_SHRINK;
			GX.VGA13.Surface = SDL_CreateRGBSurfaceWithFormatFrom(VGA13_ACTIVESCREEN_2+20*320, 320, 200, 8, 320, SDL_PIXELFORMAT_INDEX8);
			break;
		case W_LETTERBOX:
			GX.WindowMode = W_LETTERBOX;
			GX.VGA13.Surface = SDL_CreateRGBSurfaceWithFormatFrom(VGA13_ACTIVESCREEN_2, 320, 240, 8, 320, SDL_PIXELFORMAT_INDEX8);
			break;
		default:
			printf("[dRally.DISPLAY] Invalid Window Mode [#%d]. Defaults to W_SHRINK [#%d]\n", mode, W_SHRINK);
			GX.WindowMode = W_SHRINK;
			GX.VGA13.Surface = SDL_CreateRGBSurfaceWithFormatFrom(VGA13_ACTIVESCREEN_2+20*320, 320, 200, 8, 320, SDL_PIXELFORMAT_INDEX8);
			break;		
		}	
	}

	if(!GX.VESA101.Surface) GX.VESA101.Surface = SDL_CreateRGBSurfaceWithFormatFrom(VESA101_ACTIVESCREEN, 640, 480, 8, 640, SDL_PIXELFORMAT_INDEX8);

	if(!GX.Window){

		GX.Window = SDL_CreateWindow(
			"dRally / Open Source Engine / Death Rally [1996]",	// window title
			SDL_WINDOWPOS_CENTERED,      						// initial x position
			SDL_WINDOWPOS_CENTERED,       						// initial y position
			W_WIDTH,                  							// width, in pixels
			W_HEIGHT,											// height, in pixels
#if defined(DR_SWITCH)
			SDL_WINDOW_HIDDEN|SDL_WINDOW_FULLSCREEN				// flags - see below
#else
			SDL_WINDOW_HIDDEN									// flags - see below
#endif
		);
	}

	if(!GX.Renderer){

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
#if defined(DR_SWITCH)
		/* vsync: the picture waits for the screen; the game's own clock keeps 70 frames a second */
		if(SwitchConfig.vsync) GX.Renderer = SDL_CreateRenderer(GX.Window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
		if(!GX.Renderer)
#endif
		GX.Renderer = SDL_CreateRenderer(GX.Window, -1, SDL_RENDERER_ACCELERATED);
		if(!GX.Renderer){

			SDL_Log("Accelerated renderer failed (%s), using the software renderer", SDL_GetError());
			GX.Renderer = SDL_CreateRenderer(GX.Window, -1, SDL_RENDERER_SOFTWARE);
		}
#if defined(DR_SWITCH)
		{
			SDL_RendererInfo info;

			DR_VSYNC = GX.Renderer && (SDL_GetRendererInfo(GX.Renderer, &info) == 0) && (info.flags&SDL_RENDERER_PRESENTVSYNC);
			printf("[dRally.DISPLAY] vsync %s\n", DR_VSYNC ? "on" : "off");
		}
#endif
	}
}

#if defined(DR_SWITCH)

#include "switch_touch.h"

/* A touch on the window (0..1 each way) to the game's screen: the picture is the logical size the
   renderer was given (640x480 in the menus, 320x240 in a race), centred in the window. */
void dRally_Display_mapTouch(float nx, float ny, int * gx, int * gy){

	int ow = 1280, oh = 720;
	const int lw = (GX.ActiveMode == VGA13) ? 320 : 640;
	const int lh = (GX.ActiveMode == VGA13) ? 240 : 480;

	if(SwitchConfig.widescreen){

		SwitchTouch_mapStretched(nx, ny, lw, lh, gx, gy);
		return;
	}

	if(GX.Renderer) SDL_GetRendererOutputSize(GX.Renderer, &ow, &oh);
	SwitchTouch_mapToGame(nx, ny, ow, oh, lw, lh, gx, gy);
}

/* The logical size of the picture: its own size (4:3 with bars, whole-number friendly), or none at
   all with widescreen, so the picture fills the 16:9 screen. */
static void DR_SetLogical(int w, int h){

	if(SwitchConfig.widescreen) SDL_RenderSetLogicalSize(GX.Renderer, 0, 0);
	else SDL_RenderSetLogicalSize(GX.Renderer, w, h);
}

/* After scaling or widescreen was changed in the settings menu. */
void dRally_Display_applyConfig(void){

	if(GX.Renderer && GX.ActiveMode) DR_SetLogical((GX.ActiveMode == VGA13) ? 320 : 640, (GX.ActiveMode == VGA13) ? 240 : 480);
	SW_TextureW = SW_TextureH = 0;						/* the textures are made again */
}

#else

#define DR_SetLogical(w, h) SDL_RenderSetLogicalSize(GX.Renderer, w, h)

#endif // DR_SWITCH

void dRally_Display_clean(void){

	if(GX.VGA13.Surface) SDL_FreeSurface(GX.VGA13.Surface);
	if(GX.VESA101.Surface) SDL_FreeSurface(GX.VESA101.Surface);
	if(GX.Texture) SDL_DestroyTexture(GX.Texture);
#if defined(DR_SWITCH)
	if(SW_Texture) SDL_DestroyTexture(SW_Texture);
	if(SW_Target) SDL_DestroyTexture(SW_Target);
	SW_Texture = NULL;
	SW_Target = NULL;
#endif
	if(GX.Renderer) SDL_DestroyRenderer(GX.Renderer);
	if(GX.Window) SDL_DestroyWindow(GX.Window);
}

void __VGA3_SETMODE(void){

	GX.ActiveMode = VGA3;
}

void __VGA13_SETMODE(void){

	if(GX.ActiveMode != VGA13){

		switch(GX.WindowMode){
		case W_LETTERBOX:
			memset(VGA13_ACTIVESCREEN_2, 0, 320*240);
			DR_SetLogical(320, 240);
			break;
		case W_SHRINK:
		default:
			SDL_SetWindowSize(GX.Window, W_WIDTH, 5*W_HEIGHT/6);
			break;
		}

		VGA13_ACTIVESCREEN = VGA13_ACTIVESCREEN_2+20*320;
		GX.Surface = GX.VGA13.Surface;
		//SDL_SetRenderDrawColor(GX.Renderer, 0, 0, 0, 255);
		//SDL_RenderClear(GX.Renderer);
		//SDL_RenderPresent(GX.Renderer);
		if(SDL_GetWindowFlags(GX.Window)&SDL_WINDOW_HIDDEN) SDL_ShowWindow(GX.Window);
		GX.ActiveMode = VGA13;
	}
}

void __VESA101_SETMODE(void){

	if(GX.ActiveMode != VESA101){

		switch(GX.WindowMode){
		case W_LETTERBOX:
			DR_SetLogical(640, 480);
			break;
		case W_SHRINK:
		default:
			SDL_SetWindowSize(GX.Window, W_WIDTH, W_HEIGHT);
			break;
		}

		GX.Surface = GX.VESA101.Surface;
		//SDL_SetRenderDrawColor(GX.Renderer, 0, 0, 0, 255);
		//SDL_RenderClear(GX.Renderer);
		//SDL_RenderPresent(GX.Renderer);
		if(SDL_GetWindowFlags(GX.Window)&SDL_WINDOW_HIDDEN) SDL_ShowWindow(GX.Window);
		GX.ActiveMode = VESA101;
	}
}

void DISPLAY_GET_PALETTE(unsigned char * dst){

	int n;

	if(GX.ActiveMode){

		n = -1;
		while(++n < 0x100){
	
			dst[3*n] = GX.Surface->format->palette->colors[n].r >> 2;
			dst[3*n+1] = GX.Surface->format->palette->colors[n].g >> 2;
			dst[3*n+2] = GX.Surface->format->palette->colors[n].b >> 2;
		}
	}
}

void __DISPLAY_GET_PALETTE_COLOR(unsigned char * dst, unsigned char n){

	if(GX.ActiveMode){

		dst[0] = GX.Surface->format->palette->colors[n].r >> 2;
		dst[1] = GX.Surface->format->palette->colors[n].g >> 2;
		dst[2] = GX.Surface->format->palette->colors[n].b >> 2;
	}
}

void save_s3m(__POINTER__ src, unsigned int size, const char * name){

	char buffer[20] = {0};
	int n = -1;

	while(name[++n] != '.') buffer[n] = name[n];
	buffer[n] = '.';
	buffer[n+1] = 'S';
	buffer[n+2] = '3';
	buffer[n+3] = 'M';


	FILE * fd = fopen(buffer, "wb");

	fwrite(src, size, 1, fd);

	fclose(fd);
}

void save_xm(__POINTER__ src, unsigned int size, const char * name){

	char buffer[20] = {0};
	int n = -1;

	while(name[++n] != '.') buffer[n] = name[n];
	buffer[n] = '.';
	buffer[n+1] = 'X';
	buffer[n+2] = 'M';


	FILE * fd = fopen(buffer, "wb");

	fwrite(src, size, 1, fd);

	fclose(fd);
}

void switch_b(__POINTER__ b1, __POINTER__ b2){

	__BYTE__ 	b_tmp;

	b_tmp = B(b1);
	B(b1) = B(b2);
	B(b2) = b_tmp;
}


	extern __BYTE__ ___243ca4h[];
	extern __BYTE__ SUPERGLOBAL___243898h[];
	extern __BYTE__ SUPERGLOBAL___243894h[];
	extern __BYTE__ ___243ca8h[];
	extern __BYTE__ ___243874h[];
	extern __BYTE__ ___2438d0h[];

void incCounter(int n){

	if(n == 1) D(___243ca4h)++;
	if(n == 2) D(SUPERGLOBAL___243898h)++;
	if(n == 3) D(SUPERGLOBAL___243894h)++;
	if(n == 4) D(___243ca8h)++;
	if(n == 5) D(___243874h)++;
	if(n == 6) D(___2438d0h)++;
}

void decCounter(int n){

	if(n == 1) D(___243ca4h)--;
	if(n == 2) D(SUPERGLOBAL___243898h)--;
	if(n == 3) D(SUPERGLOBAL___243894h)--;
	if(n == 4) D(___243ca8h)--;
	if(n == 5) D(___243874h)--;
	if(n == 6) D(___2438d0h)--;
}

__DWORD__ getCounter(int n){

	if(n == 1) return D(___243ca4h);
	if(n == 2) return D(SUPERGLOBAL___243898h);
	if(n == 3) return D(SUPERGLOBAL___243894h);
	if(n == 4) return D(___243ca8h);
	if(n == 5) return D(___243874h);
	if(n == 6) return D(___2438d0h);

	return 0;
}

void resetCounter(int n){

	if(n == 1) D(___243ca4h) = 0;
	if(n == 2) D(SUPERGLOBAL___243898h) = 0;
	if(n == 3) D(SUPERGLOBAL___243894h) = 0;
	if(n == 4) D(___243ca8h) = 0;
	if(n == 5) D(___243874h) = 0;
	if(n == 6) D(___2438d0h) = 0;
}

void setCounter(int n, __DWORD__ val){

	if(n == 1) D(___243ca4h) = val;
	if(n == 2) D(SUPERGLOBAL___243898h) = val;
	if(n == 3) D(SUPERGLOBAL___243894h) = val;
	if(n == 4) D(___243ca8h) = val;
	if(n == 5) D(___243874h) = val;
	if(n == 6) D(___2438d0h) = val;
}
