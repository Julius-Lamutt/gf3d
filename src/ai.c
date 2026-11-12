#include <string.h>

#include "simple_logger.h"

#include "entity.h"
#include "player.h"
#include "ai.h"

static void ai_close();
static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector3D position, float *distance);
static void ai_monster_chase(MonsterAI *ai, GFC_Vector3D position);

typedef enum
{
	AIAS_IDLE,
	AIAS_ALERT
} AIAlertStatus;

typedef struct
{
	Uint32			alertStatus;	/**<current level AI alert status*/
	Uint32			lastAlert;		/**<last time a monster saw the player*/
	GFC_Vector3D	lastPosition;	/**<last known position of the player*/
	Entity			*player;		/**<pointer to player information*/
	Uint16			sightDistance;	/**<max distance monsters can see from*/
	float			sightCone;		/**<how big monsters' sight cones are (from 0-1 where 0 is big and 1 is small)*/
} LevelAI;

static LevelAI level_ai = {0};

void ai_init()
{
	SJson *json, *ajson;
	int sightDistance;
	float sightCone;

	json = sj_load("defs/ai.json");
	if (!json)
	{
		slog("failed to load ai config file");
		return;
	}
	ajson = sj_object_get_value(json, "ai");
	if (!ajson)
	{
		sj_free(json);
		slog("failed to load ai object for ai system");
		return;
	}
	if (!sj_object_get_value_as_int(ajson, "sight_distance", &sightDistance))
	{
		sj_free(json);
		slog("failed to load sight_distance object for ai system");
		return;
	}
	if (!sj_object_get_value_as_float(ajson, "sight_cone", &sightCone))
	{
		sj_free(json);
		slog("failed to load sight_cone object for ai system");
		return;
	}
	sj_free(json);
	level_ai.sightDistance = sightDistance;
	level_ai.sightCone = sightCone;

	// set defaults
	level_ai.alertStatus = AIAS_IDLE;
	level_ai.lastAlert = 0;
	level_ai.lastPosition = gfc_vector3d(0, 0, 0);
	
	atexit(ai_close);
	slog("ai system initialized");
}

static void ai_close()
{
	memset(&level_ai, 0, sizeof(LevelAI));
	slog("ai system closed");
}

void ai_cleanup()
{
	memset(&level_ai, 0, sizeof(LevelAI));
	slog("ai system cleaned up");
}

void ai_update()
{
	if (level_ai.alertStatus == AIAS_ALERT)
	{
		if (level_ai.lastAlert && SDL_GetTicks() - level_ai.lastAlert > 2000)
		{
			level_ai.alertStatus = AIAS_IDLE;
		}
	}
}

Uint32 ai_get_alert_state()
{
	return level_ai.alertStatus;
}

void ai_set_player(Entity *player)
{
	if (!player) return;

	level_ai.player = player;
}

void ai_damage_player(Uint16 damage)
{
	if (!level_ai.player) return;

	player_damage(level_ai.player, damage);
}

GFC_Vector3D ai_get_player_pos()
{
	if (!level_ai.player) return gfc_vector3d(0, 0, 0);

	return level_ai.player->position;
}

void ai_update_monster(MonsterAI *ai, GFC_Vector3D position)
{
	float distance;

	if (!ai) return;

	// don't do anything during an attack
	if (SDL_GetTicks() - ai->lastAttack < 1750)
	{
		ai->nextAction = AINA_NONE;
		return;
	}

	// me see player, me attack player
	if (ai_monster_can_see(ai, position, &distance))
	{
		if (level_ai.alertStatus != AIAS_ALERT) level_ai.alertStatus = AIAS_ALERT;

		level_ai.lastAlert = SDL_GetTicks();

		level_ai.lastPosition = ai_get_player_pos();
		if (!(level_ai.lastPosition.x == 0 && level_ai.lastPosition.y == 0 && level_ai.lastPosition.z == 0))
		{
			if (ai->rangeAttack || distance <= 5)
			{
				ai->nextAction = AINA_ATTACK;
				ai->lastAttack = SDL_GetTicks();
			}
		}
	}
	// me can't reach player, me chase player
	else if (level_ai.alertStatus == AIAS_ALERT)
	{
		ai->nextAction = AINA_MOVE;
		ai_monster_chase(ai, position);
	}
	// me lose player, me calm down and pretend I saw nothing...
	else if (level_ai.alertStatus == AIAS_IDLE)
	{
		ai->nextAction = AINA_NONE;
	}
}

static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector3D position, float *distance)
{
	if (distance) *distance = gfc_vector3d_magnitude_between(position, ai_get_player_pos());
	return gfc_vector3d_distance_between_less_than(position, ai_get_player_pos(), level_ai.sightDistance);
}

static void ai_monster_chase(MonsterAI *ai, GFC_Vector3D position)
{
	if (!ai) return;

	// TODO make ai move in player's direction when in range
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->moveState = move_state;
	else ai->moveState = AIMS_IDLE;
}
