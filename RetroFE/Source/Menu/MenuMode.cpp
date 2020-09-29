#include "MenuMode.h"
#include "../Utility/Utils.h"
#include <iostream>
#include "../SDL.h"
#include "../Utility/Utils.h"

/// -------------- DEFINES --------------
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//#define MENU_DEBUG
#define MENU_ERROR

#ifdef MENU_DEBUG
#define MENU_DEBUG_PRINTF(...)   printf(__VA_ARGS__);
#else
#define MENU_DEBUG_PRINTF(...)
#endif //MENU_DEBUG

#ifdef MENU_ERROR
#define MENU_ERROR_PRINTF(...)   printf(__VA_ARGS__);
#else
#define MENU_ERROR_PRINTF(...)
#endif //MENU_ERROR

#define SCREEN_HORIZONTAL_SIZE      240 //RES_HW_SCREEN_HORIZONTAL
#define SCREEN_VERTICAL_SIZE        240 //RES_HW_SCREEN_VERTICAL

#define SCROLL_SPEED_PX             30 //This means no animations but also no tearing effect
#define FPS_MENU                    50
#define ARROWS_PADDING				8

#define MENU_ZONE_WIDTH             SCREEN_HORIZONTAL_SIZE
#define MENU_ZONE_HEIGHT            SCREEN_VERTICAL_SIZE
#define MENU_BG_SQURE_WIDTH         180
#define MENU_BG_SQUREE_HEIGHT       140

#define MENU_FONT_NAME_TITLE        "/usr/games/menu_resources/OpenSans-Bold.ttf"
#define MENU_FONT_SIZE_TITLE        22
#define MENU_FONT_NAME_INFO         "/usr/games/menu_resources/OpenSans-Bold.ttf"
#define MENU_FONT_SIZE_INFO         16
#define MENU_FONT_NAME_SMALL_INFO   "/usr/games/menu_resources/OpenSans-Regular.ttf"
#define MENU_FONT_SIZE_SMALL_INFO   13
#define MENU_PNG_BG_PATH            "/usr/games/menu_resources/zone_bg.png"
#define MENU_PNG_ARROW_TOP_PATH     "/usr/games/menu_resources/arrow_top.png"
#define MENU_PNG_ARROW_BOTTOM_PATH  "/usr/games/menu_resources/arrow_bottom.png"

#define GRAY_MAIN_R                 85
#define GRAY_MAIN_G                 85
#define GRAY_MAIN_B                 85
#define WHITE_MAIN_R                236
#define WHITE_MAIN_G                236
#define WHITE_MAIN_B                236

#define MAX_SAVE_SLOTS              9

#define MAXPATHLEN                  512


/// -------------- STATIC VARIABLES --------------
SDL_Surface * MenuMode::backup_hw_screen = NULL;
int MenuMode::backup_key_repeat_delay=0;
int MenuMode::backup_key_repeat_interval=0;
TTF_Font *MenuMode::menu_title_font = NULL;
TTF_Font *MenuMode::menu_info_font = NULL;
TTF_Font *MenuMode::menu_small_info_font = NULL;
SDL_Surface *img_arrow_top = NULL;
SDL_Surface *img_arrow_bottom = NULL;
SDL_Surface ** MenuMode::menu_zone_surfaces = NULL;
int *MenuMode::idx_menus = NULL;
int MenuMode::nb_menu_zones = 0;
int MenuMode::menuItem=0;
int MenuMode::stop_menu_loop = 0;

SDL_Color MenuMode::text_color = {GRAY_MAIN_R, GRAY_MAIN_G, GRAY_MAIN_B};
int MenuMode::padding_y_from_center_menu_zone = 18;
uint16_t MenuMode::width_progress_bar = 100;
uint16_t MenuMode::height_progress_bar = 20;
uint16_t MenuMode::x_volume_bar = 0;
uint16_t MenuMode::y_volume_bar = 0;
uint16_t MenuMode::x_brightness_bar = 0;
uint16_t MenuMode::y_brightness_bar = 0;

int MenuMode::volume_percentage = 0;
int MenuMode::brightness_percentage = 0;

#undef X
#define X(a, b) b,
const char *MenuMode::aspect_ratio_name[] = {ASPECT_RATIOS};
int MenuMode::aspect_ratio = ASPECT_RATIOS_TYPE_STRECHED;
int MenuMode::aspect_ratio_factor_percent = 50;
int MenuMode::aspect_ratio_factor_step = 10;

Configuration *MenuMode::config = NULL;
int MenuMode::savestate_slot = 0;
int MenuMode::indexChooseLayout = 0;

/// USB stuff
int usb_data_connected = 0;
int usb_sharing = 0;


/// -------------- FUNCTIONS IMPLEMENTATION --------------
void MenuMode::init(Configuration &c)
{
	MENU_DEBUG_PRINTF("Init MenuMode\n");
	/// ----- Loading the fonts -----
	menu_title_font = TTF_OpenFont(MENU_FONT_NAME_TITLE, MENU_FONT_SIZE_TITLE);
	if(!menu_title_font){
		MENU_ERROR_PRINTF("ERROR in init_menu_SDL: Could not open menu font %s, %s\n", MENU_FONT_NAME_TITLE, SDL_GetError());
	}
	menu_info_font = TTF_OpenFont(MENU_FONT_NAME_INFO, MENU_FONT_SIZE_INFO);
	if(!menu_info_font){
		MENU_ERROR_PRINTF("ERROR in init_menu_SDL: Could not open menu font %s, %s\n", MENU_FONT_NAME_INFO, SDL_GetError());
	}
	menu_small_info_font = TTF_OpenFont(MENU_FONT_NAME_SMALL_INFO, MENU_FONT_SIZE_SMALL_INFO);
	if(!menu_small_info_font){
		MENU_ERROR_PRINTF("ERROR in init_menu_SDL: Could not open menu font %s, %s\n", MENU_FONT_NAME_SMALL_INFO, SDL_GetError());
	}

	/// ----- Copy virtual_hw_screen at init ------
	SDL_Surface * virtual_hw_screen = SDL::getWindow();
	backup_hw_screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
			virtual_hw_screen->w, virtual_hw_screen->h, 32, 0, 0, 0, 0);
	if(backup_hw_screen == NULL){
		MENU_ERROR_PRINTF("ERROR in init_menu_SDL: Could not create backup_hw_screen: %s\n", SDL_GetError());
	}

	/// ------ Load arrows imgs -------
	img_arrow_top = IMG_Load(MENU_PNG_ARROW_TOP_PATH);
	if(!img_arrow_top) {
		MENU_ERROR_PRINTF("ERROR IMG_Load: %s\n", IMG_GetError());
	}
	img_arrow_bottom = IMG_Load(MENU_PNG_ARROW_BOTTOM_PATH);
	if(!img_arrow_bottom) {
		MENU_ERROR_PRINTF("ERROR IMG_Load: %s\n", IMG_GetError());
	}

	/// ------ Save config pointer ------
	config = &c;

	/// ------ Init menu zones ------
	init_menu_zones();

	return;
}


void MenuMode::end( )
{
	MENU_DEBUG_PRINTF("End MenuMode \n");
	/// ------ Close font -------
	TTF_CloseFont(menu_title_font);
	TTF_CloseFont(menu_info_font);
	TTF_CloseFont(menu_small_info_font);

	/// ------ Free Surfaces -------
	for(int i=0; i < nb_menu_zones; i++){
		if(menu_zone_surfaces[i] != NULL){
			SDL_FreeSurface(menu_zone_surfaces[i]);
		}
	}
	if(backup_hw_screen != NULL){
		SDL_FreeSurface(backup_hw_screen);
	}

	SDL_FreeSurface(img_arrow_top);
	SDL_FreeSurface(img_arrow_bottom);

	/// ------ Free Menu memory and reset vars -----
	if(idx_menus){
		free(idx_menus);
	}
	idx_menus=NULL;
	nb_menu_zones = 0;

	return;
}


void MenuMode::draw_progress_bar(SDL_Surface * surface, uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, uint8_t percentage, uint16_t nb_bars){
	/// ------ Init Variables ------
	uint16_t line_width = 1; //px
	uint16_t padding_bars_ratio = 3;
	uint16_t nb_full_bars = 0;

	/// ------ Check values ------
	percentage = (percentage > 100)?100:percentage;
	x = (x > (surface->w-1))?(surface->w-1):x;
	y = (y > surface->h-1)?(surface->h-1):y;
	width = (width < line_width*2+1)?(line_width*2+1):width;
	width = (width > surface->w-x-1)?(surface->w-x-1):width;
	height = (height < line_width*2+1)?(line_width*2+1):height;
	height = (height > surface->h-y-1)?(surface->h-y-1):height;
	uint16_t nb_bars_max = ( width * padding_bars_ratio  /  (line_width*2+1) + 1 ) / (padding_bars_ratio+1);
	nb_bars = (nb_bars > nb_bars_max)?nb_bars_max:nb_bars;
	uint16_t bar_width = (width / nb_bars)*padding_bars_ratio/(padding_bars_ratio+1)+1;
	uint16_t bar_padding_x = bar_width/padding_bars_ratio;
	nb_full_bars = nb_bars*percentage/100;

	/// ------ draw full bars ------
	for (int i = 0; i < nb_full_bars; ++i)
	{
		/// ---- draw one bar ----
		//MENU_DEBUG_PRINTF("Drawing filled bar %d\n", i);
		SDL_Rect rect = {x+ i*(bar_width +bar_padding_x),
				y, bar_width, height};
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, GRAY_MAIN_R, GRAY_MAIN_G, GRAY_MAIN_B));
	}

	/// ------ draw full bars ------
	for (int i = 0; i < (nb_bars-nb_full_bars); ++i)
	{
		/// ---- draw one bar ----
		//MENU_DEBUG_PRINTF("Drawing empty bar %d\n", i);
		SDL_Rect rect = {x+ i*(bar_width +bar_padding_x) + nb_full_bars*(bar_width +bar_padding_x),
				y, bar_width, height};
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, GRAY_MAIN_R, GRAY_MAIN_G, GRAY_MAIN_B));

		SDL_Rect rect2 = {x+ i*(bar_width +bar_padding_x) + line_width + nb_full_bars*(bar_width +bar_padding_x),
				y + line_width, bar_width - line_width*2, height - line_width*2};
		SDL_FillRect(surface, &rect2, SDL_MapRGB(surface->format, WHITE_MAIN_R, WHITE_MAIN_R, WHITE_MAIN_R));
	}


}


void MenuMode::add_menu_zone(ENUM_MENU_TYPE menu_type){
	/// ------ Increase nb of menu zones -------
	nb_menu_zones++;

	/// ------ Realoc idx Menus array -------
	if(!idx_menus){
		idx_menus = (int*) malloc(nb_menu_zones*sizeof(int));
		menu_zone_surfaces = (SDL_Surface**) malloc(nb_menu_zones*sizeof(SDL_Surface*));
	}
	else{
		int *temp = (int*) realloc(idx_menus, nb_menu_zones*sizeof(int));
		idx_menus = temp;
		menu_zone_surfaces = (SDL_Surface**) realloc(menu_zone_surfaces, nb_menu_zones*sizeof(SDL_Surface*));
	}
	idx_menus[nb_menu_zones-1] = menu_type;

	/// ------ Reinit menu surface with height increased -------
	menu_zone_surfaces[nb_menu_zones-1] = IMG_Load(MENU_PNG_BG_PATH);
	if(!menu_zone_surfaces[nb_menu_zones-1]) {
		MENU_ERROR_PRINTF("ERROR IMG_Load: %s\n", IMG_GetError());
	}

	/// --------- Init Common Variables --------
	SDL_Surface *text_surface = NULL;
	SDL_Surface *surface = menu_zone_surfaces[nb_menu_zones-1];
	SDL_Rect text_pos;

	/// --------- Add new zone ---------
	switch(menu_type){
	case MENU_TYPE_VOLUME:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_VOLUME\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "VOLUME", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);

		x_volume_bar = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - width_progress_bar)/2;
		y_volume_bar = surface->h - MENU_ZONE_HEIGHT/2 - height_progress_bar/2 + padding_y_from_center_menu_zone;
		draw_progress_bar(surface, x_volume_bar, y_volume_bar,
				width_progress_bar, height_progress_bar, 0, 100/STEP_CHANGE_VOLUME);
		break;
	case MENU_TYPE_BRIGHTNESS:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_BRIGHTNESS\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "BRIGHTNESS", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);

		x_brightness_bar = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - width_progress_bar)/2;
		y_brightness_bar = surface->h - MENU_ZONE_HEIGHT/2 - height_progress_bar/2 + padding_y_from_center_menu_zone;
		draw_progress_bar(surface, x_brightness_bar, y_brightness_bar,
				width_progress_bar, height_progress_bar, 0, 100/STEP_CHANGE_BRIGHTNESS);
		break;
	case MENU_TYPE_SAVE:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_SAVE\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "SAVE", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone*2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	case MENU_TYPE_LOAD:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_LOAD\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "LOAD", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone*2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	case MENU_TYPE_ASPECT_RATIO:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_ASPECT_RATIO\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "ASPECT RATIO", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	case MENU_TYPE_EXIT:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_EXIT\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "EXIT GAME", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	case MENU_TYPE_USB:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_USB\n");
		/// ------ Text ------
		/*text_surface = TTF_RenderText_Blended(menu_title_font, "USB", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);*/
		break;
	case MENU_TYPE_THEME:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_THEME\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "SET THEME", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 - padding_y_from_center_menu_zone*2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	case MENU_TYPE_POWERDOWN:
		MENU_DEBUG_PRINTF("Init MENU_TYPE_POWERDOWN\n");
		/// ------ Text ------
		text_surface = TTF_RenderText_Blended(menu_title_font, "POWERDOWN", text_color);
		text_pos.x = (surface->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
		text_pos.y = surface->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
		SDL_BlitSurface(text_surface, NULL, surface, &text_pos);
		break;
	default:
		MENU_DEBUG_PRINTF("Warning - In add_menu_zone, unknown MENU_TYPE: %d\n", menu_type);
		break;
	}

	/// ------ Free Surfaces -------
	SDL_FreeSurface(text_surface);
}

void MenuMode::init_menu_zones(){
	/// Init Volume Menu
	add_menu_zone(MENU_TYPE_VOLUME);
	/// Init Brightness Menu
	add_menu_zone(MENU_TYPE_BRIGHTNESS);
	/// Init Save Menu
	//add_menu_zone(MENU_TYPE_SAVE);
	/// Init Load Menu
	//add_menu_zone(MENU_TYPE_LOAD);
	/// Init Aspect Ratio Menu
	//add_menu_zone(MENU_TYPE_ASPECT_RATIO);
	/// Init Exit Menu
	//add_menu_zone(MENU_TYPE_EXIT);
	/// Init USB Menu
	add_menu_zone(MENU_TYPE_USB);
	/// Init Theme Menu
	add_menu_zone(MENU_TYPE_THEME);
	/// Init Powerdown Menu
	add_menu_zone(MENU_TYPE_POWERDOWN);
}


void MenuMode::init_menu_system_values(){
	FILE *fp;
	char res[100];

	/// ------- Get system volume percentage --------
	fp = popen(SHELL_CMD_VOLUME_GET, "r");
	if (fp == NULL) {
		MENU_ERROR_PRINTF("Failed to run command %s\n", SHELL_CMD_VOLUME_GET );
		volume_percentage = 50; ///wrong value: setting default to 50
	}
	else{
		fgets(res, sizeof(res)-1, fp);

		/// Check if Volume is a number (at least the first char)
		if(res[0] < '0' || res[0] > '9'){
			MENU_ERROR_PRINTF("Wrong return value: %s for volume cmd: %s\n",res, SHELL_CMD_VOLUME_GET);
			volume_percentage = 50; ///wrong value: setting default to 50
		}
		else{
			volume_percentage = atoi(res);
			MENU_DEBUG_PRINTF("System volume = %d%%\n", volume_percentage);
		}
	}

	/// ------- Get system brightness percentage -------
	fp = popen(SHELL_CMD_BRIGHTNESS_GET, "r");
	if (fp == NULL) {
		MENU_ERROR_PRINTF("Failed to run command %s\n", SHELL_CMD_BRIGHTNESS_GET );
		brightness_percentage = 50; ///wrong value: setting default to 50
	}
	else{
		fgets(res, sizeof(res)-1, fp);

		/// Check if brightness is a number (at least the first char)
		if(res[0] < '0' || res[0] > '9'){
			MENU_ERROR_PRINTF("Wrong return value: %s for volume cmd: %s\n",res, SHELL_CMD_BRIGHTNESS_GET);
			brightness_percentage = 50; ///wrong value: setting default to 50
		}
		else{
			brightness_percentage = atoi(res);
			MENU_DEBUG_PRINTF("System brightness = %d%%\n", brightness_percentage);
		}
	}

	/// ------- Get USB Value -------
	usb_data_connected = Utils::executeRawPath(SHELL_CMD_USB_DATA_CONNECTED);
	usb_sharing = Utils::executeRawPath(SHELL_CMD_USB_CHECK_IS_SHARING);

	if(usb_sharing && !usb_data_connected){
		MENU_ERROR_PRINTF("WARNING usb_sharing && !usb_data_connected\n");
		usb_sharing = 0;
	}

	if(usb_sharing){

		/// Force USB menu to launch
		for(int cur_idx=0; cur_idx < nb_menu_zones; cur_idx++){
			if(idx_menus[cur_idx] == MENU_TYPE_USB){
				menuItem = cur_idx;
				printf("USB mounted, setting menu item to %d\n", menuItem);
				break;
			}
		}
	}
}

void MenuMode::menu_screen_refresh(int menuItem, int prevItem, int scroll, uint8_t menu_confirmation, uint8_t menu_action){
	/// --------- Vars ---------
	int print_arrows = (scroll || usb_sharing)?0:1;

	/// --------- Clear HW screen ----------
	SDL_Surface * virtual_hw_screen = SDL::getWindow();
	if(SDL_BlitSurface(backup_hw_screen, NULL, virtual_hw_screen, NULL)){
		MENU_ERROR_PRINTF("ERROR Could not Clear virtual_hw_screen: %s\n", SDL_GetError());
	}

	/// --------- Setup Blit Window ----------
	SDL_Rect menu_blit_window;
	menu_blit_window.x = 0;
	menu_blit_window.w = SCREEN_HORIZONTAL_SIZE;

	/// --------- Blit prev menu Zone going away ----------
	menu_blit_window.y = scroll;
	menu_blit_window.h = SCREEN_VERTICAL_SIZE;
	if(SDL_BlitSurface(menu_zone_surfaces[prevItem], &menu_blit_window, virtual_hw_screen, NULL)){
		MENU_ERROR_PRINTF("ERROR Could not Blit surface on virtual_hw_screen: %s\n", SDL_GetError());
	}

	/// --------- Blit new menu Zone going in (only during animations) ----------
	if(scroll>0){
		menu_blit_window.y = SCREEN_VERTICAL_SIZE-scroll;
		menu_blit_window.h = SCREEN_VERTICAL_SIZE;
		if(SDL_BlitSurface(menu_zone_surfaces[menuItem], NULL, virtual_hw_screen, &menu_blit_window)){
			MENU_ERROR_PRINTF("ERROR Could not Blit surface on virtual_hw_screen: %s\n", SDL_GetError());
		}
	}
	else if(scroll<0){
		menu_blit_window.y = SCREEN_VERTICAL_SIZE+scroll;
		menu_blit_window.h = SCREEN_VERTICAL_SIZE;
		if(SDL_BlitSurface(menu_zone_surfaces[menuItem], &menu_blit_window, virtual_hw_screen, NULL)){
			MENU_ERROR_PRINTF("ERROR Could not Blit surface on virtual_hw_screen: %s\n", SDL_GetError());
		}
	}
	/// --------- No Scroll ? Blitting menu-specific info
	else{
		SDL_Surface * text_surface = NULL;
		char text_tmp[100];
		SDL_Rect text_pos;
		char fname[MAXPATHLEN];
		memset(fname, 0, MAXPATHLEN);
		char *curLayoutName;
		bool dots=false;
		int max_chars = 15;

		switch(idx_menus[menuItem]){
		case MENU_TYPE_VOLUME:
			draw_progress_bar(virtual_hw_screen, x_volume_bar, y_volume_bar,
					width_progress_bar, height_progress_bar, volume_percentage, 100/STEP_CHANGE_VOLUME);
			break;

		case MENU_TYPE_BRIGHTNESS:
			draw_progress_bar(virtual_hw_screen, x_volume_bar, y_volume_bar,
					width_progress_bar, height_progress_bar, brightness_percentage, 100/STEP_CHANGE_BRIGHTNESS);
			break;

		case MENU_TYPE_SAVE:
			/// ---- Write slot -----
			sprintf(text_tmp, "IN SLOT   < %d >", savestate_slot+1);
			text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);

			if(menu_action){
				sprintf(text_tmp, "Saving...");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			}
			else{
				if(menu_confirmation){
					sprintf(text_tmp, "Are you sure ?");
					text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				}
				else{
					/// ---- Write current Save state ----
				}
			}
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			break;

		case MENU_TYPE_LOAD:
			/// ---- Write slot -----
			sprintf(text_tmp, "FROM SLOT   < %d >", savestate_slot+1);
			text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);

			if(menu_action){
				sprintf(text_tmp, "Loading...");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			}
			else{
				if(menu_confirmation){
					sprintf(text_tmp, "Are you sure ?");
					text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				}
				else{
					/// ---- Write current Load state ----
				}
			}
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			break;

		case MENU_TYPE_ASPECT_RATIO:
			sprintf(text_tmp, "<   %s   >", aspect_ratio_name[aspect_ratio]);
			text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + padding_y_from_center_menu_zone;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			break;

		case MENU_TYPE_USB:
			/// ---- Write slot -----
			sprintf(text_tmp, "%s USB", usb_sharing?"EJECT":"MOUNT");
			text_surface = TTF_RenderText_Blended(menu_title_font, text_tmp, text_color);
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);

			if(menu_action){
				sprintf(text_tmp, "in progress ...");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
				text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
				SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			}
			else if(menu_confirmation){
				sprintf(text_tmp, "Are you sure ?");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
				text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
				SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			}
			else{
					///Nothing
			}
			break;

		case MENU_TYPE_THEME:
			/// ---- Write current chosen theme -----
			curLayoutName = (char*)Utils::getFileName(config->layouts_.at(indexChooseLayout)).c_str();

			// no more than max_chars chars in name to fit screen
			if(strlen(curLayoutName) > max_chars){
				curLayoutName[max_chars-2] = 0;
				dots = true;
			}
			sprintf(text_tmp, "< %s%s >", curLayoutName, dots?"...":"" );

			text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
			text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
			text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2;
			SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);

			if(menu_action){
				sprintf(text_tmp, "In progress...");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
				text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
				SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			}
			else if(menu_confirmation){
				sprintf(text_tmp, "Are you sure ?");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
				text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
				SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			}
			break;

		case MENU_TYPE_EXIT:
		case MENU_TYPE_POWERDOWN:
			if(menu_action){
				sprintf(text_tmp, "Shutting down...");
				text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
				text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
				text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
				SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
			}
			else{
				if(menu_confirmation){
					sprintf(text_tmp, "Are you sure ?");
					text_surface = TTF_RenderText_Blended(menu_info_font, text_tmp, text_color);
					text_pos.x = (virtual_hw_screen->w - MENU_ZONE_WIDTH)/2 + (MENU_ZONE_WIDTH - text_surface->w)/2;
					text_pos.y = virtual_hw_screen->h - MENU_ZONE_HEIGHT/2 - text_surface->h/2 + 2*padding_y_from_center_menu_zone;
					SDL_BlitSurface(text_surface, NULL, virtual_hw_screen, &text_pos);
				}
			}
			break;
		default:
			break;
		}

		/// ------ Free Surfaces -------
		if(text_surface)
			SDL_FreeSurface(text_surface);
	}

	/// --------- Print arrows --------
	if(print_arrows){
		/// Top arrow
		SDL_Rect pos_arrow_top;
		pos_arrow_top.x = (virtual_hw_screen->w - img_arrow_top->w)/2;
		pos_arrow_top.y = (virtual_hw_screen->h - MENU_BG_SQUREE_HEIGHT)/4 - img_arrow_top->h/2;
		SDL_BlitSurface(img_arrow_top, NULL, virtual_hw_screen, &pos_arrow_top);

		/// Bottom arrow
		SDL_Rect pos_arrow_bottom;
		pos_arrow_bottom.x = (virtual_hw_screen->w - img_arrow_bottom->w)/2;
		pos_arrow_bottom.y = virtual_hw_screen->h -
			(virtual_hw_screen->h - MENU_BG_SQUREE_HEIGHT)/4 - img_arrow_bottom->h/2;
		SDL_BlitSurface(img_arrow_bottom, NULL, virtual_hw_screen, &pos_arrow_bottom);
	}

	/// --------- Flip Screen ----------
	//SDL_Flip(hw_screen);
	SDL::renderAndFlipWindow();
}


int MenuMode::launch( )
{
	MENU_DEBUG_PRINTF("Launch MenuMode\n");

	SDL_Event event;
	uint32_t prev_ms = SDL_GetTicks();
	uint32_t cur_ms = SDL_GetTicks();
	int scroll=0;
	int start_scroll=0;
	uint8_t screen_refresh = 1;
	char shell_cmd[100];
	FILE *fp;
	uint8_t menu_confirmation = 0;
	stop_menu_loop = 0;
	char fname[MAXPATHLEN];
	indexChooseLayout = config->currentLayoutIdx_;
	int returnCode = MENU_RETURN_OK;

	/// ------ Get System values -------
	init_menu_system_values();
	int prevItem=menuItem;

	/// Save prev key repeat params and set new Key repeat
	SDL_GetKeyRepeat(&backup_key_repeat_delay, &backup_key_repeat_interval);
	if(SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL)){
		MENU_ERROR_PRINTF("ERROR with SDL_EnableKeyRepeat: %s\n", SDL_GetError());
	}

	/// Get save slot from game
	//savestate_slot = (savestate_slot%MAX_SAVE_SLOTS); // security

	/// ------ Copy currently displayed screen -------
	SDL_Surface * virtual_hw_screen = SDL::getWindow();
	if(SDL_BlitSurface(virtual_hw_screen, NULL, backup_hw_screen, NULL)){
		MENU_ERROR_PRINTF("ERROR Could not copy virtual_hw_screen: %s\n", SDL_GetError());
	}

	/// -------- Main loop ---------
	while (!stop_menu_loop)
	{
		/// -------- Handle Keyboard Events ---------
		if(!scroll){
			while (SDL_PollEvent(&event))
				switch(event.type)
				{
				case SDL_QUIT:
					stop_menu_loop = 1;
					returnCode = MENU_RETURN_EXIT;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
					case SDLK_b:
						if(menu_confirmation){
							/// ------ Reset menu confirmation ------
							menu_confirmation = 0;
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						/*else{
							stop_menu_loop = 1;
						}*/
						break;

					case SDLK_q:
					case SDLK_ESCAPE:
						/// ------ Check if no action ------
						if(usb_sharing){
							break;
						}

						stop_menu_loop = 1;
						break;

					case SDLK_d:
					case SDLK_DOWN:
						MENU_DEBUG_PRINTF("DOWN\n");
						/// ------ Check if no action ------
						if(usb_sharing){
							break;
						}

						/// ------ Start scrolling to new menu -------
						menuItem++;
						if (menuItem>=nb_menu_zones) menuItem=0;

						/// Skip if usb menu if usb not connected
						if(idx_menus[menuItem] == MENU_TYPE_USB && !usb_data_connected){
							menuItem++;
							if (menuItem>=nb_menu_zones) menuItem=0;
						}

						start_scroll=1;

						/// ------ Reset menu confirmation ------
						menu_confirmation = 0;

						/// ------ Refresh screen ------
						screen_refresh = 1;
						break;

					case SDLK_u:
					case SDLK_UP:
						MENU_DEBUG_PRINTF("UP\n");
						/// ------ Check if no action ------
						if(usb_sharing){
							break;
						}

						/// ------ Start scrolling to new menu -------
						menuItem--;
						if (menuItem<0) menuItem=nb_menu_zones-1;

						/// Skip if usb menu if usb not connected
						if(idx_menus[menuItem] == MENU_TYPE_USB && !usb_data_connected){
							menuItem--;
							if (menuItem<0) menuItem=nb_menu_zones-1;
						}

						start_scroll=-1;

						/// ------ Reset menu confirmation ------
						menu_confirmation = 0;

						/// ------ Refresh screen ------
						screen_refresh = 1;
						break;

					case SDLK_l:
					case SDLK_LEFT:
						//MENU_DEBUG_PRINTF("LEFT\n");
						if(idx_menus[menuItem] == MENU_TYPE_VOLUME){
							MENU_DEBUG_PRINTF("Volume DOWN\n");
							/// ----- Compute new value -----
							volume_percentage = (volume_percentage < STEP_CHANGE_VOLUME)?
									0:(volume_percentage-STEP_CHANGE_VOLUME);

							/// ----- Shell cmd ----
							sprintf(shell_cmd, "%s %d", SHELL_CMD_VOLUME_SET, volume_percentage);
							fp = popen(shell_cmd, "r");
							if (fp == NULL) {
								MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
							}

							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_BRIGHTNESS){
							MENU_DEBUG_PRINTF("Brightness DOWN\n");
							/// ----- Compute new value -----
							brightness_percentage = (brightness_percentage < STEP_CHANGE_BRIGHTNESS)?
									0:(brightness_percentage-STEP_CHANGE_BRIGHTNESS);

							/// ----- Shell cmd ----
							sprintf(shell_cmd, "%s %d", SHELL_CMD_BRIGHTNESS_SET, brightness_percentage);
							fp = popen(shell_cmd, "r");
							if (fp == NULL) {
								MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
							}
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_SAVE){
							MENU_DEBUG_PRINTF("Save Slot DOWN\n");
							savestate_slot = (!savestate_slot)?(MAX_SAVE_SLOTS-1):(savestate_slot-1);
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_LOAD){
							MENU_DEBUG_PRINTF("Load Slot DOWN\n");
							//idx_load_slot = (!idx_load_slot)?(MAX_SAVE_SLOTS-1):(idx_load_slot-1);
							savestate_slot = (!savestate_slot)?(MAX_SAVE_SLOTS-1):(savestate_slot-1);
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_ASPECT_RATIO){
							MENU_DEBUG_PRINTF("Aspect Ratio DOWN\n");
							aspect_ratio = (!aspect_ratio)?(NB_ASPECT_RATIOS_TYPES-1):(aspect_ratio-1);
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_THEME){
							MENU_DEBUG_PRINTF("Theme previous\n");
							indexChooseLayout = (!indexChooseLayout)?(config->layouts_.size()-1):(indexChooseLayout-1);
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						break;

					case SDLK_r:
					case SDLK_RIGHT:
						//MENU_DEBUG_PRINTF("RIGHT\n");
						if(idx_menus[menuItem] == MENU_TYPE_VOLUME){
							MENU_DEBUG_PRINTF("Volume UP\n");
							/// ----- Compute new value -----
							volume_percentage = (volume_percentage > 100 - STEP_CHANGE_VOLUME)?
									100:(volume_percentage+STEP_CHANGE_VOLUME);

							/// ----- Shell cmd ----
							sprintf(shell_cmd, "%s %d", SHELL_CMD_VOLUME_SET, volume_percentage);
							fp = popen(shell_cmd, "r");
							if (fp == NULL) {
								MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
							}
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_BRIGHTNESS){
							MENU_DEBUG_PRINTF("Brightness UP\n");
							/// ----- Compute new value -----
							brightness_percentage = (brightness_percentage > 100 - STEP_CHANGE_BRIGHTNESS)?
									100:(brightness_percentage+STEP_CHANGE_BRIGHTNESS);

							/// ----- Shell cmd ----
							sprintf(shell_cmd, "%s %d", SHELL_CMD_BRIGHTNESS_SET, brightness_percentage);
							fp = popen(shell_cmd, "r");
							if (fp == NULL) {
								MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
							}
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_SAVE){
							MENU_DEBUG_PRINTF("Save Slot UP\n");
							savestate_slot = (savestate_slot+1)%MAX_SAVE_SLOTS;
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_LOAD){
							MENU_DEBUG_PRINTF("Load Slot UP\n");
							//idx_load_slot = (idx_load_slot+1)%MAX_SAVE_SLOTS;
							savestate_slot = (savestate_slot+1)%MAX_SAVE_SLOTS;
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_ASPECT_RATIO){
							MENU_DEBUG_PRINTF("Aspect Ratio UP\n");
							aspect_ratio = (aspect_ratio+1)%NB_ASPECT_RATIOS_TYPES;
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						else if(idx_menus[menuItem] == MENU_TYPE_THEME){
							MENU_DEBUG_PRINTF("Theme previous\n");
							indexChooseLayout = (indexChooseLayout+1)%config->layouts_.size();
							/// ------ Refresh screen ------
							screen_refresh = 1;
						}
						break;

					case SDLK_a:
					case SDLK_RETURN:
						if(idx_menus[menuItem] == MENU_TYPE_SAVE){
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("Saving in slot %d\n", savestate_slot);
								/// ------ Refresh Screen -------
								menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 1);

								/// ------ Save game ------
								stop_menu_loop = 1;
							}
							else{
								MENU_DEBUG_PRINTF("Save game - asking confirmation\n");
								menu_confirmation = 1;
								/// ------ Refresh screen ------
								screen_refresh = 1;
							}
						}
						else if(idx_menus[menuItem] == MENU_TYPE_LOAD){
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("Loading in slot %d\n", savestate_slot);
								/// ------ Refresh Screen -------
								menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 1);

								/// ------ Load game ------
								stop_menu_loop = 1;
							}
							else{
								MENU_DEBUG_PRINTF("Save game - asking confirmation\n");
								menu_confirmation = 1;
								/// ------ Refresh screen ------
								screen_refresh = 1;
							}
						}
						else if(idx_menus[menuItem] == MENU_TYPE_USB){
							MENU_DEBUG_PRINTF("USB %s\n", usb_sharing?"unmount":"mount");
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("%s USB - confirmed\n", usb_sharing?"Unmount":"Mount");
								/// ----- Refresh screen ----
								menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 1);

								/// ----- Shell cmd ----
								/*fp = popen(usb_sharing?SHELL_CMD_USB_UNMOUNT:SHELL_CMD_USB_MOUNT, "r");
								if (fp == NULL) {
									MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
								}
								else{
									usb_sharing = !usb_sharing;
								}*/

								bool res = Utils::executeRawPath(usb_sharing?SHELL_CMD_USB_UNMOUNT:SHELL_CMD_USB_MOUNT);
								if (!res) {
									MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
								}
								else{
									usb_sharing = !usb_sharing;
								}

								/// ------ Refresh screen ------
								menu_confirmation = 0;
								screen_refresh = 1;
							}
							else{
								MENU_DEBUG_PRINTF("%s USB - asking confirmation\n", usb_sharing?"Unmount":"Mount");
								menu_confirmation = 1;
								screen_refresh = 1;
							}
						}
						else if(idx_menus[menuItem] == MENU_TYPE_THEME){
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("Theme change - confirmed\n");

								/// ------ Refresh Screen -------
								menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 1);

								/// ----- Write new theme and restart RetroFe ----
								config->exportCurrentLayout(Utils::combinePath(Configuration::absolutePath, "layout.conf"),
										Utils::getFileName(config->layouts_.at(indexChooseLayout)));
								stop_menu_loop = 1;
								returnCode = MENU_RETURN_EXIT;
							}
							else{
								MENU_DEBUG_PRINTF("Theme change - asking confirmation\n");
								menu_confirmation = 1;
								/// ------ Refresh screen ------
								screen_refresh = 1;
							}
						}
						else if(idx_menus[menuItem] == MENU_TYPE_EXIT){
							MENU_DEBUG_PRINTF("Exit game\n");
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("Exit game - confirmed\n");
								/// ----- The game should be saved here ----

								/// ----- Exit game and back to launcher ----
								stop_menu_loop = 1;
								returnCode = MENU_RETURN_EXIT;
							}
							else{
								MENU_DEBUG_PRINTF("Exit game - asking confirmation\n");
								menu_confirmation = 1;
								/// ------ Refresh screen ------
								screen_refresh = 1;
							}
						}
						else if(idx_menus[menuItem] == MENU_TYPE_POWERDOWN){
							if(menu_confirmation){
								MENU_DEBUG_PRINTF("Powerdown - confirmed\n");

								/// ------ Refresh Screen -------
								menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 1);

								/// ----- Shell cmd ----
								sprintf(shell_cmd, "%s", SHELL_CMD_POWERDOWN);
								fp = popen(shell_cmd, "r");
								if (fp == NULL) {
									MENU_ERROR_PRINTF("Failed to run command %s\n", shell_cmd);
								}

								return MENU_RETURN_EXIT;
							}
							else{
								MENU_DEBUG_PRINTF("Powerdown - asking confirmation\n");
								menu_confirmation = 1;
								/// ------ Refresh screen ------
								screen_refresh = 1;
							}
						}
						break;

					default:
						//MENU_DEBUG_PRINTF("Keydown: %d\n", event.key.keysym.sym);
						break;
					}
					break;
				}
		}

		/// --------- Handle Scroll effect ---------
		if ((scroll>0) || (start_scroll>0)){
			scroll+=MIN(SCROLL_SPEED_PX, MENU_ZONE_HEIGHT-scroll);
			start_scroll = 0;
			screen_refresh = 1;
		}
		else if ((scroll<0) || (start_scroll<0)){
			scroll-=MIN(SCROLL_SPEED_PX, MENU_ZONE_HEIGHT+scroll);
			start_scroll = 0;
			screen_refresh = 1;
		}
		if (scroll>=MENU_ZONE_HEIGHT || scroll<=-MENU_ZONE_HEIGHT) {
			prevItem=menuItem;
			scroll=0;
			screen_refresh = 1;
		}

		/// --------- Handle FPS ---------
		cur_ms = SDL_GetTicks();
		if(cur_ms-prev_ms < 1000/FPS_MENU){
			SDL_Delay(1000/FPS_MENU - (cur_ms-prev_ms));
		}
		prev_ms = SDL_GetTicks();


		/// --------- Refresh screen
		if(screen_refresh){
			menu_screen_refresh(menuItem, prevItem, scroll, menu_confirmation, 0);
		}

		/// --------- reset screen refresh ---------
		screen_refresh = 0;
	}

	/// ------ Reset prev key repeat params -------
	if(SDL_EnableKeyRepeat(backup_key_repeat_delay, backup_key_repeat_interval)){
		MENU_ERROR_PRINTF("ERROR with SDL_EnableKeyRepeat: %s\n", SDL_GetError());
	}
	return returnCode;
}
