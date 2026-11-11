#include "simple_logger.h"

#include "element_button.h"

ButtonElement *element_button_load(SJson *windel)
{
	ButtonElement *button;
	SJson *buttonWindel, *array;
	int label, actor, type;
	GFC_Color highColor, pressColor;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}
	button = gfc_allocate_array(sizeof(ButtonElement), 1);
	if (!button)
	{
		slog("failed to allocate button element");
		return NULL;
	}

	buttonWindel = sj_object_get_value(windel, "label");
	if (!buttonWindel)
	{
		label = 0;
		button->label = NULL;
	}
	else
	{
		label = 1;
		button->label = element_label_load(buttonWindel);
		if (!button->label)
		{
			element_button_free(button);
			slog("failed to create a label for button element");
			return NULL;
		}
	}

	buttonWindel = sj_object_get_value(windel, "actor");
	if (!buttonWindel)
	{
		actor = 0;
		button->actor = NULL;
	}
	else
	{
		actor = 1;
		button->actor = element_actor_load(buttonWindel);
		if (!button->actor)
		{
			element_label_free(button->label);
			element_button_free(button);
			slog("failed to create an actor for button element");
			return NULL;
		}
	}

	if (label && actor) type = BT_BOTH;
	else if (label) type = BT_LABEL;
	else if (actor) type = BT_ACTOR;
	else type = BT_HIDDEN;

	array = sj_object_get_value(windel, "high_color");
	if (sj_array_get_count(array) != 4)
	{
		element_button_free(button);
		slog("missing or extra high_color parameters for button element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &highColor.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &highColor.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &highColor.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &highColor.a))
	{
		element_button_free(button);
		slog("one or more high_color parameters are invalid for button element");
		return NULL;
	}
	highColor.ct = CT_RGBA8;

	array = sj_object_get_value(windel, "press_color");
	if (sj_array_get_count(array) != 4)
	{
		element_button_free(button);
		slog("missing or extra press_color parameters for button element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &pressColor.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &pressColor.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &pressColor.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &pressColor.a))
	{
		element_button_free(button);
		slog("one or more press_color parameters are invalid for button element");
		return NULL;
	}
	pressColor.ct = CT_RGBA8;

	button->type = type;
	button->highColor = highColor;
	button->pressColor = pressColor;
	return button;
}

void element_button_draw(ButtonElement *button, Element *element)
{
	GFC_Rect textBounds;

	if (!button) return;
	if (element->state == ES_IDLE) draw_2d_rect(element->bounds, element->color);
	else if (element->state == ES_HIGHLIGHT) draw_2d_rect(element->bounds, button->highColor);
	else if (element->state == ES_ACTIVE) draw_2d_rect(element->bounds, button->pressColor);

	if (button->actor) element_actor_draw(button->actor, element->bounds);
	if (button->label)
	{
		textBounds.x = element->bounds.x + (0.1 * element->bounds.w);
		textBounds.y = element->bounds.y;
		textBounds.w = 0.8 * element->bounds.w;
		textBounds.h = element->bounds.h;
		element_label_draw(button->label, textBounds);
	}
}

void element_button_free(ButtonElement *button)
{
	if (!button) return;
	if (button->label) element_label_free(button->label);
	if (button->actor) element_actor_free(button->actor);
	free(button);
}