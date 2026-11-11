#include "simple_logger.h"
#include "simple_json.h"

#include "elements.h"
#include "windows.h"

typedef struct
{
	Window  *window_list;
	Uint32	window_max;
} WindowManager;

static WindowManager _window_manager = {0};

static void window_system_close();
static void window_update(Window *win);
static void window_draw(Window *win);

void window_system_init(Uint32 max)
{
	if (_window_manager.window_list)
	{
		slog("window manager is aleady active");
		return;
	}
	if (!max)
	{
		slog("cannot initialize window system with zero windows");
		return;
	}
	_window_manager.window_list = gfc_allocate_array(sizeof(Window), max);
	if (!_window_manager.window_list)
	{
		slog("failed to allocate global window list");
		return;
	}
	_window_manager.window_max = max;
	atexit(window_system_close);
	slog("window system initialized");
}

void window_system_close()
{
	window_clear_all(NULL);
	if (_window_manager.window_list) free(_window_manager.window_list);
	memset(&_window_manager, 0, sizeof(WindowManager));
	slog("window system closed");
}

void window_clear_all(Window *ignore)
{
	int i;
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (&_window_manager.window_list[i] == ignore) continue;
		if (!_window_manager.window_list[i]._inuse) continue;
		window_free(&_window_manager.window_list[i]);
	}
}

Window *window_new()
{
	int i;
	if (!_window_manager.window_list)
	{
		slog("window system has not been initialized!");
		return NULL;
	}
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (_window_manager.window_list[i]._inuse) continue;
		memset(&_window_manager.window_list[i], 0, sizeof(Window));

		// set defaults
		_window_manager.window_list[i]._inuse = 1;
		_window_manager.window_list[i].hidden = 0;
		return &_window_manager.window_list[i];
	}
	slog("no more available windows");
	return NULL;
}

void window_free(Window *win)
{
	if (!win) return;
	win->_inuse = 0; // save this spot for future windows
	gf2d_sprite_free(win->background);
	gf2d_sprite_free(win->border);
	element_list_free(win->elements);

	// anything else we allocate for our window would get cleaned up here
	if (win->free) win->free(win);
}

void window_update(Window* win)
{
	int i, c;
	GFC_List *updates;
	Element *element;

	if (!win) return;

	updates = gfc_list_new();
	c = gfc_list_get_count(win->elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(win->elements, i);
		if (!element) continue;
		element_update_state(element);
		if (element->state == ES_ACTIVE) gfc_list_append(updates, element);
	}
	if (win->update) win->update(win, updates);
	gfc_list_delete(updates);
}

void window_system_update()
{
	int i;
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if ((!_window_manager.window_list[i]._inuse) || (_window_manager.window_list[i].hidden)) continue;
		window_update(&_window_manager.window_list[i]);
	}
}

void window_draw(Window* win)
{
	GFC_Vector2D position, bgScale, borderScale;
	GFC_Color bgColor, borderColor;

	if (!win) return;
	if (!win->background) return;
	if (!win->border) return;

	position = gfc_vector2d(win->size.x, win->size.y);
	bgScale = gfc_vector2d(win->size.w / win->background->frameWidth, win->size.h / win->background->frameHeight);
	borderScale = gfc_vector2d(win->size.w / win->border->frameWidth, win->size.h / win->border->frameHeight);
	bgColor = win->bgColor;
	borderColor = win->borderColor;

	gf2d_sprite_draw(win->background, 
		position, 
		&bgScale, // scale background to match size
		NULL,
		NULL,
		NULL,
		&bgColor,
		NULL,
		0);

	gf2d_sprite_draw(win->border, 
		position,
		&borderScale, // scale border to match size
		NULL,
		NULL,
		NULL,
		&borderColor,
		NULL,
		0);

	if (win->draw) win->draw(win);
	element_list_draw(win->elements);
}

void window_system_draw()
{
	int i;
	for (i = _window_manager.window_max - 1; i >= 0; i--)
	{
		if ((!_window_manager.window_list[i]._inuse) || (_window_manager.window_list[i].hidden)) continue;
		window_draw(&_window_manager.window_list[i]);
	}
}

Window *window_load(const char *filename)
{
	Window *win, *parent;
	SJson *file, *window, *elementList, *array;
	const char *name, *bgName, *borderName, *pname;
	int frameW, frameH;
	GFC_List *elements;
	Sprite *background, *border;
	GFC_Rect size, canvas;
	GFC_Vector4D sizeDims, canvasDims;
	GFC_Color bgColor, borderColor;

	if (!filename)
	{
		slog("no file name provided for window load");
		return NULL;
	}
	file = sj_load(filename);
	if (!file)
	{
		slog("failed to load window file '%s'", filename);
		return NULL;
	}
	window = sj_object_get_value(file, "window");
	if (!window)
	{
		sj_free(file);
		slog("missing window object in file '%s'", filename);
		return NULL;
	}

	win = window_new();
	if (!win)
	{
		sj_free(file);
		return NULL;
	}

	name = sj_object_get_value_as_string(window, "name"); 
	if (!name)
	{
		window_free(win);
		sj_free(file);
		slog("missing window name object in file '%s'", filename);
		return NULL;
	}

	elementList = sj_object_get_value(window, "elements");
	if (!elementList)
	{
		window_free(win);
		sj_free(file);
		slog("missing window elements object in file '%s'", filename);
		return NULL;
	}
	elements = element_list_load(elementList, win);
	if (!elements)
	{
		window_free(win);
		sj_free(file);
		slog("failed to get window elements in file '%s'");
		return NULL;
	}

	array = sj_object_get_value(window, "background");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window background object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 3)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window background parameters in file '%s'", filename);
		return NULL;
	}
	bgName = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!bgName ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frameW) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frameH))
	{
		window_free(win);
		sj_free(file);
		slog("one or more background parameters are invalid in file '%s'", filename);
		return NULL;
	}
	background = gf2d_sprite_load(bgName, frameW, frameH, 1);

	array = sj_object_get_value(window, "border");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window border object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 3)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window border parameters in file '%s'", filename);
		return NULL;
	}
	borderName = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!borderName ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frameW) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frameH))
	{
		window_free(win);
		sj_free(file);
		slog("one or more border parameters are invalid in file '%s'", filename);
		return NULL;
	}
	border = gf2d_sprite_load(borderName, frameW, frameH, 1);

	array = sj_object_get_value(window, "size");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window size object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window size dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &sizeDims.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &sizeDims.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &sizeDims.z) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &sizeDims.w))
	{
		window_free(win);
		sj_free(file);
		slog("one or more size dimensions are invalid in file '%s'", filename);
		return NULL;
	}
	size = gfc_rect(sizeDims.x, sizeDims.y, sizeDims.z, sizeDims.w);

	array = sj_object_get_value(window, "canvas");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window canvas object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window canvas dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &canvasDims.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &canvasDims.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &canvasDims.z) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &canvasDims.w))
	{
		window_free(win);
		sj_free(file);
		slog("one or more canvas dimensions are invalid in file '%s'", filename);
		return NULL;
	}
	canvas = gfc_rect(canvasDims.x, canvasDims.y, canvasDims.z, canvasDims.w);

	array = sj_object_get_value(window, "bg_color");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window bg_color object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window bg_color dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &bgColor.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &bgColor.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &bgColor.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &bgColor.a))
	{
		window_free(win);
		sj_free(file);
		slog("one or more bg_color parameters are invalid in file '%s'", filename);
		return NULL;
	}
	bgColor.ct = CT_RGBA8;

	array = sj_object_get_value(window, "border_color");
	if (!array)
	{
		window_free(win);
		sj_free(file);
		slog("missing window border_color object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		window_free(win);
		sj_free(file);
		slog("missing or extra window border_color dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &borderColor.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &borderColor.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &borderColor.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &borderColor.a))
	{
		window_free(win);
		sj_free(file);
		slog("one or more bg_color parameters are invalid in file '%s'", filename);
		return NULL;
	}
	borderColor.ct = CT_RGBA8;

	pname = sj_object_get_value_as_string(window, "parent");
	if (!pname) parent = NULL;
	else parent = window_find_by_name(pname);
	gfc_line_cpy(win->name, name);
	win->background = background;
	win->border = border;
	win->elements = elements;
	win->size = size;
	win->bgColor = bgColor;
	win->borderColor = borderColor;
	win->canvas = canvas;
	win->parent = parent;
	if (parent) parent->child = win;
	sj_free(file);
	return win;
}

Window *window_find_by_name(const char *name)
{
	int i;

	if (!name)
	{
		slog("failed to get window name");
		return NULL;
	}
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (!_window_manager.window_list[i]._inuse) continue;
		if (gfc_strlcmp(_window_manager.window_list[i].name, name) == 0) return &_window_manager.window_list[i];
	}
	return NULL;
}