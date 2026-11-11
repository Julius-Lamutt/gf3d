#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "draw_2d.h"

typedef struct
{
    Sprite *rectangle;			/**<sprite for unit rectangle*/
    Sprite *rectangleOutline;	/**<model for unit rectangle outline*/
} Draw2dData;

Draw2dData draw_2d_data = {0};

static void draw_2d_close();

void draw_2d_init()
{
    draw_2d_data.rectangle = gf2d_sprite_load("models/primitives/flatwhite.png", 32, 32, 1);
    draw_2d_data.rectangleOutline = gf2d_sprite_load("images/default_border.png", 400, 400, 1);
    atexit(draw_2d_close);
}

static void draw_2d_close()
{
    if (draw_2d_data.rectangle) gf2d_sprite_free(draw_2d_data.rectangle);
    if (draw_2d_data.rectangleOutline) gf2d_sprite_free(draw_2d_data.rectangleOutline);
}

void draw_2d_rect(GFC_Rect rect, GFC_Color color)
{
	GFC_Vector2D position, scale;

	if (!draw_2d_data.rectangle) return;

	position = gfc_vector2d(rect.x, rect.y);
	scale = gfc_vector2d(rect.w / draw_2d_data.rectangle->frameWidth, rect.h / draw_2d_data.rectangle->frameHeight);

	gf2d_sprite_draw(draw_2d_data.rectangle, 
		position, 
		&scale, // scale background to match size
		NULL,
		NULL,
		NULL,
		&color,
		NULL,
		0);
}

void draw_2d_rect_outline(GFC_Rect rect, GFC_Color color)
{
	GFC_Vector2D position, scale;

	if (!draw_2d_data.rectangleOutline) return;

	position = gfc_vector2d(rect.x, rect.y);
	scale = gfc_vector2d(rect.w / draw_2d_data.rectangleOutline->frameWidth, rect.h / draw_2d_data.rectangleOutline->frameHeight);

	gf2d_sprite_draw(draw_2d_data.rectangleOutline, 
		position, 
		&scale, // scale background to match size
		NULL,
		NULL,
		NULL,
		&color,
		NULL,
		0);
}

