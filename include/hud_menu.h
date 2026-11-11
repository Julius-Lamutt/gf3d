#ifndef __HUD_MENU_H__
#define __HUD_MENU_H__

#include "windows.h"

/*
* @brief load the hud menu
* @return NULL on error, a pointer to the hud menu window otherwise
*/
Window *hud_menu();

/*
* @brief update the health display
* @param win the window to update
* @param count the amount of health
*/
void hud_menu_update_health(Window *win, Sint16 count);

/*
* @brief update the armor display
* @param win the window to update
* @param count the amount of armor
*/
void hud_menu_update_armor(Window *win, Sint16 count);

/*
* @brief update the heat display
* @param win the window to update
* @param count the amount of heat
*/
void hud_menu_update_heat(Window *win, Sint16 count);

/*
* @brief update the medicine inventory count
* @param win the window to update inventory data for
* @param count the new invnetory count
*/
void hud_menu_update_inv_health(Window *win, Uint8 count);

/*
* @brief update the armor inventory count
* @param win the window to update inventory data for
* @param count the new invnetory count
*/
void hud_menu_update_inv_armor(Window *win, Uint8 count);

/*
* @brief update the spinach inventory count
* @param win the window to update inventory data for
* @param count the new invnetory count
*/
void hud_menu_update_inv_damage(Window *win, Uint8 count);

/*
* @brief update the stimulant inventory count
* @param win the window to update inventory data for
* @param count the new inventory count
*/
void hud_menu_update_inv_speed(Window *win, Uint8 count);

/*
* @brief update the touriner inventory count
* @param win the window to update inventory data for
* @param count the new inventory count
*/
void hud_menu_update_inv_heat(Window *win, Uint8 count);

#endif
