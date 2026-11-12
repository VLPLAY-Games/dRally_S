#include "drally.h"
#include "drally_fonts.h"
#include "drally_keyboard.h"
#include "sfx.h"

#pragma pack(1)
typedef struct menubox_s {
	__DWORD__ 	row_n;			// +00
	__DWORD__ 	x;				// +04
	__DWORD__ 	y;				// +08
	__DWORD__ 	row_h;			// +0C
	__DWORD__ 	w;				// +10
	__DWORD__ 	h;				// +14
	__DWORD__ 	row_i;			// +18
} menubox_t;

	extern menubox_t ___185a5ch[9];
	extern char ___1866b8h[9][9][50];
	extern __POINTER__ ___1a112ch__VESA101_ACTIVESCREEN_PTR;
	extern font_props_t ___185ba9h;
	extern __POINTER__ ___1a10e0h;
	extern __BYTE__ ___185b58h[9][9];
	extern __POINTER__ ___1a10cch;
	extern __DWORD__ ___1a1e68h;
	extern __POINTER__ ___1a10f4h;
	extern __BYTE__ ___196a84h[];
	extern __DWORD__ ___24cc54h_sfx_volume;
#if defined(DR_MULTIPLAYER)
	extern __DWORD__ ___19bd60h;
#endif // DR_MULTIPLAYER

#if defined(DR_MULTIPLAYER)
void ___23758h(void);
#endif // DR_MULTIPLAYER

void dRally_Sound_pushEffect(__BYTE__ channel, __BYTE__ n, __DWORD__ unk, __DWORD__ a0, __DWORD__ a1, __DWORD__ a2);
void ___2ab50h(void);
void menu___13a98h(__DWORD__);
__BYTE__ ___5994ch(void);
void menu___3da48h(void);

void ___12e78h_cdecl(__POINTER__ A1, font_props_t * A2, const char * A3, __DWORD__ dst_off);
void ___1398ch__VESA101_PRESENTRECTANGLE(__DWORD__ offset, __POINTER__ src, __DWORD__ w, __DWORD__ h);


static int changerow(menubox_t * mbox, int c){ return (mbox->row_i = (mbox->row_n+mbox->row_i+c)%mbox->row_n); }

static void helper_menu(menubox_t * mbox, __POINTER__ font, int menu_i){

	int 	n, mbox_x, mbox_y;

	mbox_x = mbox->x+7;
	mbox_y = mbox->y+mbox->row_h*mbox->row_i+5;

	n = -1;
	while(++n < 22) memset(___1a112ch__VESA101_ACTIVESCREEN_PTR+0x280*(mbox_y+n+5)+mbox_x+2, 0xc4, mbox->w-20);

	___12e78h_cdecl(font, &___185ba9h, ___1866b8h[menu_i][mbox->row_i], 0x280*mbox_y+mbox_x+25);
	___1398ch__VESA101_PRESENTRECTANGLE(0x280*(mbox_y+1)+mbox_x, ___1a112ch__VESA101_ACTIVESCREEN_PTR+0x280*(mbox_y+1)+mbox_x, mbox->w-10, 0x20);
}

static void helper_icon(__DWORD__ A1){

	int 			i, j, mbox_x, mbox_y;
	__BYTE__ 		px;
	menubox_t *		mbox;

	
	mbox = &___185a5ch[A1];
	mbox_x = mbox->x+9;
	mbox_y = mbox->y+mbox->row_h*mbox->row_i+11;

	j = -1;
	while(++j < 20){

		i = -1;
		while(++i < 20){

			px = read_b(___1a10f4h+400*___1a1e68h+20*j+i);
			write_b(___1a112ch__VESA101_ACTIVESCREEN_PTR+0x280*(mbox_y+j)+mbox_x+i, (px != 0)?px:0xc4);
		}
	}

	___1398ch__VESA101_PRESENTRECTANGLE(0x280*mbox_y+mbox_x, ___1a112ch__VESA101_ACTIVESCREEN_PTR+0x280*mbox_y+mbox_x, 20, 20);
}

// MENU UP
static void menu___14010h(__DWORD__ A1){

	helper_menu(&___185a5ch[A1], ___1a10e0h, A1);
	while(!___185b58h[A1][changerow(&___185a5ch[A1], -1)]);
	helper_menu(&___185a5ch[A1], ___1a10cch, A1);
	helper_icon(A1);
}

// MENU DOWN
static void menu___14368h(__DWORD__ A1){

	helper_menu(&___185a5ch[A1], ___1a10e0h, A1);
	while(!___185b58h[A1][changerow(&___185a5ch[A1], 1)]);
	helper_menu(&___185a5ch[A1], ___1a10cch, A1);
	helper_icon(A1);
}

// MENU LAST ITEM
static void menu___13cech(__DWORD__ A1){

	helper_menu(&___185a5ch[A1], ___1a10e0h, A1);
	___185a5ch[A1].row_i = ___185a5ch[A1].row_n-1;
	helper_menu(&___185a5ch[A1], ___1a10cch, A1);
	helper_icon(A1);
}

// SPINNING TIRE ICON
void menu___13a98h(__DWORD__ A1){

	helper_icon(A1);
	___1a1e68h = (___1a1e68h+1)%50;
}



#if defined(DR_SWITCH)
#include "switch_touch.h"

/* A tap on a row of the menu selects it and confirms, as moving to it and pressing Enter does.
   The rows are row_h tall from the box's top; a row that is switched off cannot be tapped.
   Returns the row, or -1. */
static int touch_row(menubox_t * mbox, int menu_i){

	int tx, ty, row;

	if(!SwitchTouch_takeTap(&tx, &ty)) return -1;

	if((tx < (int)mbox->x-8)||(tx >= (int)(mbox->x+mbox->w)+8)||(ty < (int)mbox->y)||(mbox->row_h == 0)) return -1;

	row = (ty-(int)mbox->y)/(int)mbox->row_h;
	if((row < 0)||(row >= (int)mbox->row_n)||!___185b58h[menu_i][row]) return -1;

	if(row != (int)mbox->row_i){

		helper_menu(mbox, ___1a10e0h, menu_i);
		mbox->row_i = row;
		helper_menu(mbox, ___1a10cch, menu_i);
		helper_icon(menu_i);
	}

	return row;
}
#endif // DR_SWITCH

int ___146c4h_cdecl(__DWORD__ A1){

	int 		rslt, n;
	__BYTE__ 	LastScan;


	rslt = -2;
	n = 0;
	while((D(___196a84h) == 0)&&(rslt == -2)){

		___2ab50h();
		___2ab50h();
		menu___13a98h(A1);	// SPINNING TIRE ICON
#if defined(DR_SWITCH)
		{
			const int tapped = touch_row(&___185a5ch[A1], A1);

			if(tapped >= 0){

				rslt = tapped;
				dRally_Sound_pushEffect(1, SFX_CLICK_4, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			}
		}
#endif // DR_SWITCH

		LastScan = ___5994ch();

		if(A1 == 0){
 
			if(LastScan) n = 0;
			else if(++n > 1800){
			
				menu___3da48h();
				n = 0;
			}
		}

		switch(LastScan){
		case DR_SCAN_ESCAPE:
			if(A1){

				rslt = -1;
				dRally_Sound_pushEffect(1, SFX_CLICK_1, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			}
			else {

				if(___185a5ch[A1].row_i == (___185a5ch[A1].row_n-1)) break;
				menu___13cech(A1); // MENU LAST ITEM
				dRally_Sound_pushEffect(1, SFX_CLICK_2, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			}
			break;
		case DR_SCAN_ENTER:
		case DR_SCAN_SPACE:
		case DR_SCAN_KP_ENTER:
			rslt = (int)___185a5ch[A1].row_i;
			dRally_Sound_pushEffect(1, SFX_CLICK_4, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			break;
		case DR_SCAN_F1:
#if defined(DR_MULTIPLAYER)
			if(___19bd60h != 0) ___23758h();
#endif // DR_MULTIPLAYER
			break;
		case DR_SCAN_UP:
		case DR_SCAN_KP_8:
			menu___14010h(A1);	// MENU UP
			dRally_Sound_pushEffect(1, SFX_CLICK_2, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			break;
		case DR_SCAN_DOWN:
		case DR_SCAN_KP_2:
			menu___14368h(A1);	// MENU DOWN
			dRally_Sound_pushEffect(1, SFX_CLICK_2, 0, ___24cc54h_sfx_volume, 0x28000, 0x8000);
			break;
		default:
			break;
		}
	}

	if(D(___196a84h) != 0) rslt = -1;

	return rslt;
}
