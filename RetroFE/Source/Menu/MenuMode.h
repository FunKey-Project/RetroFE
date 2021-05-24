#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include "../Database/Configuration.h"

typedef enum{
    MENU_TYPE_VOLUME,
    MENU_TYPE_BRIGHTNESS,
    MENU_TYPE_SAVE,
    MENU_TYPE_LOAD,
    MENU_TYPE_ASPECT_RATIO,
    MENU_TYPE_USB,
    MENU_TYPE_THEME,
    MENU_TYPE_LAUNCHER,
    MENU_TYPE_EXIT,
    MENU_TYPE_POWERDOWN,
    NB_MENU_TYPES,
} ENUM_MENU_TYPE;

typedef enum{
    MENU_RETURN_OK,
    MENU_RETURN_EXIT,
    NB_MENU_RETURN_CODES,
} ENUM_MENU_RETURN_CODES;


///------ Definition of the different aspect ratios
#define ASPECT_RATIOS \
    X(ASPECT_RATIOS_TYPE_MANUAL, "MANUAL ZOOM") \
    X(ASPECT_RATIOS_TYPE_STRECHED, "STRECHED") \
    X(ASPECT_RATIOS_TYPE_CROPPED, "CROPPED") \
    X(ASPECT_RATIOS_TYPE_SCALED, "SCALED") \
    X(NB_ASPECT_RATIOS_TYPES, "")

////------ Enumeration of the different aspect ratios ------
#undef X
#define X(a, b) a,
typedef enum {ASPECT_RATIOS} ENUM_ASPECT_RATIOS_TYPES;

////------ Defines to be shared -------
#define STEP_CHANGE_VOLUME          10
#define STEP_CHANGE_BRIGHTNESS      10

////------ Menu commands -------
#define SHELL_CMD_VOLUME_GET                "volume get"
#define SHELL_CMD_VOLUME_SET                "volume set"
#define SHELL_CMD_BRIGHTNESS_GET            "brightness get"
#define SHELL_CMD_BRIGHTNESS_SET            "brightness set"
#define SHELL_CMD_USB_DATA_CONNECTED        "is_usb_data_connected"
#define SHELL_CMD_USB_MOUNT                 "share start"
#define SHELL_CMD_USB_UNMOUNT               "share stop"
#define SHELL_CMD_USB_CHECK_IS_SHARING      "share is_sharing"
#define SHELL_CMD_POWERDOWN                 "shutdown_funkey"
#define SHELL_CMD_SCHEDULE_POWERDOWN        "sched_shutdown"
#define SHELL_CMD_CANCEL_SCHED_POWERDOWN    "cancel_sched_powerdown"
#define SHELL_CMD_FRONTEND_SET_GMENU2X      "frontend set gmenu2x"
#define SHELL_CMD_FRONTEND_SET_RETROFE      "frontend set retrofe"

class MenuMode
{

public:
    //MenuMode();
    static void     init(Configuration &c);
    static void     end();
    static int      launch( );
    static void     stop( );

    /*static SDL_Surface * draw_screen;

        static int backup_key_repeat_delay, backup_key_repeat_interval;
        static SDL_Surface * backup_hw_screen;

        static TTF_Font *menu_title_font;
        static TTF_Font *menu_info_font;
        static TTF_Font *menu_small_info_font;
        static SDL_Surface ** menu_zone_surfaces;
        static int * idx_menus;
        static int nb_menu_zones;

        static int stop_menu_loop;

        static SDL_Color text_color;
        static int padding_y_from_center_menu_zone;
        static uint16_t width_progress_bar;
        static uint16_t height_progress_bar;
        static uint16_t x_volume_bar;
        static uint16_t y_volume_bar;
        static uint16_t x_brightness_bar;
        static uint16_t y_brightness_bar;

        static int volume_percentage;
        static int brightness_percentage;

        static const char *aspect_ratio_name[];
        static int aspect_ratio;
        static int aspect_ratio_factor_percent;
        static int aspect_ratio_factor_step;

        static int savestate_slot;*/

private:
    static void draw_progress_bar(SDL_Surface * surface, uint16_t x, uint16_t y, uint16_t width,
            uint16_t height, uint8_t percentage, uint16_t nb_bars);
    static void add_menu_zone(ENUM_MENU_TYPE menu_type);
    static void init_menu_zones();
    static void init_menu_system_values();
    static void menu_screen_refresh(int menuItem, int prevItem, int scroll, uint8_t menu_confirmation, uint8_t menu_action);

    //static SDL_Surface * hw_screen;
    //static SDL_Surface * virtual_hw_screen; // this one is not rotated
    static SDL_Surface * draw_screen;

    static int backup_key_repeat_delay;
	static int backup_key_repeat_interval;
    static SDL_Surface * backup_hw_screen;

    static TTF_Font *menu_title_font;
    static TTF_Font *menu_info_font;
    static TTF_Font *menu_small_info_font;
    static SDL_Surface ** menu_zone_surfaces;
    static int * idx_menus;
    static int nb_menu_zones;
    static int menuItem;

    static int stop_menu_loop;

    static SDL_Color text_color;
    static int padding_y_from_center_menu_zone;
    static uint16_t width_progress_bar;
    static uint16_t height_progress_bar;
    static uint16_t x_volume_bar;
    static uint16_t y_volume_bar;
    static uint16_t x_brightness_bar;
    static uint16_t y_brightness_bar;

    static int volume_percentage;
    static int brightness_percentage;

    static const char *aspect_ratio_name[];
    static int aspect_ratio;
    static int aspect_ratio_factor_percent;
    static int aspect_ratio_factor_step;

    static int savestate_slot;

    static Configuration *config;
    static int indexChooseLayout;
};
