#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_mesh.h"

#include "windows.h"
#include "main_menu.h"
#include "hud_menu.h"

#include "items.h"
#include "model.h"
#include "draw_2d.h"
#include "draw_3d.h"
#include "level.h"
#include "entity.h"
#include "player.h"
#include "item_pickup.h"
#include "monster.h"
#include "ai.h"

extern int __DEBUG;
extern int __COLLISION;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

int main(int argc,char *argv[])
{
    // local variables
    Window *hud;
    Entity *player, *health, *armor, *damage, *speed, *heat, *brawler, *brute, *grabber, *shooter, *dodger;
    Level *level;
    GFC_Rect rect = gfc_rect(10, 10, 50, 50);

    // initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");

    // gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    gfc_audio_init(200, 1, 0);

    // gf3d init
    gf3d_mesh_init(2000);
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);

    // resource manager init
    draw_2d_init();
    draw_3d_init();
    window_system_init(64);
    entity_system_init(1024);
    items_init("defs/items.json");
    
    // game init
    srand(SDL_GetTicks());
    slog_sync();
    gf2d_mouse_load("actors/mouse.actor");

    // menu
    hud = hud_menu();

    // player
    player = player_spawn(gfc_vector3d(0, -90, 10));

    // item pickups
    health = item_pickup_spawn("health", gfc_vector3d(-12, -80, 10));
    armor = item_pickup_spawn("armor", gfc_vector3d(-6, -80, 10));
    damage = item_pickup_spawn("damage", gfc_vector3d(0, -80, 10));
    speed = item_pickup_spawn("speed", gfc_vector3d(6, -80, 10));
    heat = item_pickup_spawn("heat", gfc_vector3d(12, -80, 10));

    // monsters
    brawler = monster_spawn("brawler", gfc_vector3d(-0, -50, 10));

    // ai init
    ai_init();
    ai_set_player(player);

    // level
    level = level_load("levels/test_level.json");
    entity_system_set_level(level);

    // main game loop    
    while(!_done)
    {
        // UI updates
        window_system_update();
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        // entity updates
        entity_system_think();
        entity_system_update();

        //camera updates
        gf3d_vgraphics_render_start();
            
            //3D draws
            level_draw(level);
            entity_system_draw();

            //2D draws
            window_system_draw();
            gf2d_mouse_draw();

        gf3d_vgraphics_render_end();

        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc, char *argv[])
{
    int a;

    for (a = 1; a < argc; a++)
    {
        if (strcmp(argv[a], "--debug") == 0) __DEBUG = 1;
        else if (strcmp(argv[a], "--collision") == 0) __COLLISION = 1;
    }
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
