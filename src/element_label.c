#include "simple_logger.h"

#include "element_label.h"

LabelElement *element_label_load(SJson *windel)
{
	LabelElement *label;
	const char *text, *font;
	SJson *array;
	GFC_Color color;
	int style, wrap;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}
	label = gfc_allocate_array(sizeof(LabelElement), 1);
	if (!label)
	{
		slog("failed to allocate label element");
		return NULL;
	}

	text = sj_object_get_value_as_string(windel, "text");
	if (!text)
	{
		free(label);
		slog("failed to find text object for label element");
		return NULL;
	}

	font = sj_object_get_value_as_string(windel, "style");
	if (!font)
	{
		free(label);
		slog("failed to find style object for label element");
		return NULL;
	}
	if (gfc_strlcmp(font, "large") == 0) style = FT_Large;
	else if (gfc_strlcmp(font, "normal") == 0) style = FT_Normal;
	else if (gfc_strlcmp(font, "small") == 0) style = FT_Small;
	else if (gfc_strlcmp(font, "h1") == 0) style = FT_H1;
	else if (gfc_strlcmp(font, "h2") == 0) style = FT_H2;
	else if (gfc_strlcmp(font, "h3") == 0) style = FT_H3;
	else if (gfc_strlcmp(font, "h4") == 0) style = FT_H4;
	else if (gfc_strlcmp(font, "h5") == 0) style = FT_H5;
	else if (gfc_strlcmp(font, "h6") == 0) style = FT_H6;
	else
	{
		free(label);
		slog("invalid style object for label element");
		return NULL;
	}

	array = sj_object_get_value(windel, "text_color");
	if (sj_array_get_count(array) != 4)
	{
		free(label);
		slog("missing or extra color parameters for label element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &color.a))
	{
		free(label);
		slog("one or more color parameters are invalid for label element");
		return NULL;
	}
	color.ct = CT_RGBA8;

	if (!sj_object_get_value_as_int(windel, "wrap", &wrap))
	{
		free(label);
		slog("failed to find wrap object for label element");
		return NULL;
	}

	gfc_line_cpy(label->text, text);
	label->style = style;
	label->color = color;
	label->wrap = wrap;
	return label;
}

void element_label_draw(LabelElement *label, GFC_Rect bounds)
{
	if (!label) return;
	if (label->wrap)
	{
		gf2d_font_draw_text_wrap(label->text, bounds, label->color, label->style);
	}
	else
	{
		gf2d_font_draw_line_tag(label->text, label->style, label->color, gfc_vector2d(bounds.x, bounds.y));
	}
}

void element_label_free(LabelElement *label)
{
	if (!label) return;
	free(label);
}