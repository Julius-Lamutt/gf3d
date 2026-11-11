#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "draw_2d.h"

void draw_2d_rect(GFC_Rect rect, GFC_Color color)
{
    GFC_Color tempColor;
    Uint32 drawColor;
    SDL_Rect drawRect;
    SDL_Surface *surface;
    Sprite *sprite;

    tempColor = gfc_color_to_int8(color);
    drawColor = SDL_MapRGBA(SDL_PIXELFORMAT_RGBA32, tempColor.r, tempColor.g, tempColor.b, tempColor.a);

    drawRect = gfc_rect_to_sdl_rect(rect);
    surface = SDL_CreateRGBSurface(0, drawRect.w, drawRect.h, 32, 0, 0, 0, 0);
    if (!SDL_FillRect(surface, &drawRect, &drawColor)) return;

    sprite = gf2d_sprite_from_surface(surface, drawRect.w, drawRect.h, 1);
    if (!sprite) return;

    gf2d_sprite_draw(sprite,
        gfc_vector2d(drawRect.x, drawRect.y), 
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    );
}

void draw_2d_rect_outline(GFC_Rect rect, GFC_Color color)
{
    GFC_Color tempColor;
    Uint32 drawColor;
    SDL_Rect drawRect;
    SDL_Surface *surface;
    Sprite *sprite;

    tempColor = gfc_color_to_int8(color);
    drawColor = SDL_MapRGBA(SDL_PIXELFORMAT_RGBA32, tempColor.r, tempColor.g, tempColor.b, tempColor.a);

    drawRect = gfc_rect_to_sdl_rect(rect);
    surface = SDL_CreateRGBSurface(0, drawRect.w, drawRect.h, 32, 0, 0, 0, 0);
    if (!SDL_FillRect(surface, &drawRect, &drawColor)) return;

    sprite = gf2d_sprite_from_surface(surface, drawRect.w, drawRect.h, 1);
    if (!sprite) return;

    gf2d_sprite_draw(sprite,
        gfc_vector2d(drawRect.x, drawRect.y), 
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    );
}

