#include "simple_logger.h"

#include "player.h"
#include "inventory.h"
#include "elements.h"
#include "hud_menu.h"

typedef struct
{
	Window	*win;		/* the current window */
	Uint8	selected;	/* true if window is currently selected */
	Sint16	health;		/* the amount of health left */
	Sint16	armor;		/* the amount of armor left */
	Sint16	heat;		/* the amount of heat left */
	Uint8	invHealth;	/* inventory number of medicine */
	Uint8	invArmor;	/* inventory number of armor */
	Uint8	invDamage;	/* inventory number of spinach */
	Uint8	invSpeed;	/* inventory number of stimulant */
	Uint8	invHeat;	/* inventory number of touriner */
} HudMenuData;

static int hud_menu_update(Window *win, GFC_List *updates);
static int hud_menu_draw(Window *win);
static int hud_menu_free(Window *win);

Window *hud_menu()
{
	Window *win;
	HudMenuData *data;
	win = window_load("defs/menus/hud_menu.json");
	if (!win)
	{
		slog("failed to load inventory menu");
		return NULL;
	}
	win->update = hud_menu_update;
	win->draw = hud_menu_draw;
	win->free = hud_menu_free;
	data = (HudMenuData*)gfc_allocate_array(sizeof(HudMenuData), 1);
	win->data = data;
	data->win = win;
	data->selected = 1;
	data->health = 0;
	data->armor = 0;
	data->heat = 0;
	data->invHealth = 0;
	data->invArmor = 0;
	data->invDamage = 0;
	data->invSpeed = 0;
	data->invHeat = 0;
	return win;
}

static int hud_menu_update(Window *win, GFC_List *updates)
{
	Element *element;
	HudMenuData *data;
	int i, c;

	if (!win) return 0;
	if (!updates) return 0;
	data = (HudMenuData*)win->data;
	if (!data) return 0;

	c = gfc_list_get_count(win->elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(win->elements, i);
		if (!element) continue;
		if (gfc_strlcmp(element->name, "label_health_number") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->health, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_armor_number") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->armor, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_heat_number") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->heat, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_inv_health") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->invHealth, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_inv_armor") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->invArmor, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_inv_damage") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->invDamage, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_inv_speed") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->invSpeed, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_inv_heat") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->invHeat, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
	}
	return 1;
}

static int hud_menu_draw(Window *win)
{
	HudMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;
	return 1;
}

static int hud_menu_free(Window *win)
{
	HudMenuData *data;

	if ((!win) || (!win->data)) return 0;
	data = (HudMenuData*) win->data;
	free(data);
	return 0;
}

void hud_menu_update_health(Window *win, Sint16 count)
{
	HudMenuData *data;

	if (!win) return;
	data = (HudMenuData*)win->data;
	if (!data) return;

	if (count < 0) data->health = 0;
	else data->health = count;
}

void hud_menu_update_armor(Window *win, Sint16 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	if (count < 0) data->armor = 0;
	else data->armor = count;
}

void hud_menu_update_heat(Window *win, Sint16 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	if (count < 0) data->heat = 0;
	else data->heat = count;
}

void hud_menu_update_inv_health(Window *win, Uint8 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	data->invHealth = count;
}

void hud_menu_update_inv_armor(Window *win, Uint8 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	data->invArmor = count;
}

void hud_menu_update_inv_damage(Window *win, Uint8 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	data->invDamage = count;
}

void hud_menu_update_inv_speed(Window *win, Uint8 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	data->invSpeed = count;
}

void hud_menu_update_inv_heat(Window *win, Uint8 count)
{
	HudMenuData *data;

	if (!win) return;

	data = (HudMenuData*)win->data;
	if (!data) return;

	data->invHeat = count;
}