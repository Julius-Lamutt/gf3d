#ifndef __AI_H__
#define __AI_H__

#include "gfc_vector.h"

#include "entity.h"

typedef enum
{
	AINA_NONE,
	AINA_MOVE,
	AINA_SHIFT,
	AINA_ATTACK
} AINextAction;

typedef enum
{
	AIMS_IDLE,
	AIMS_WALK,
	AIMS_RUN,
	AIMS_MAX
} AIMoveState;

typedef struct
{
	Uint32		nextAction;		/**<what should the monster ai do next*/
	Uint32		moveState;		/**<how should the monster ai move*/
	Uint32		lastAttack;		/**<time since last attack (NO SPAMMING ALLOWED)*/
	Uint8		rangeAttack;	/**<determines whether ai has access to long range attack*/
} MonsterAI;

/*
* @brief Initialize the level ai system.
* Note: This must be initialized before loading a world for monsters to work!!
*/
void ai_init();

/*
* @brief Cleanup any information for the level ai system.
* Note: This must be called every time a player leaves a loaded level!!
*/
void ai_cleanup();

/*
* @brief update the level ai
*/
void ai_update();

/*
* @brief get the current ai alert state
* @return the ai alert state
*/
Uint32 ai_get_alert_state();

/*
* @brief set the player for the ai system
* @param player the player entity
*/
void ai_set_player(Entity *player);

/*
* @brief deal damage to the player
* @param damage the number of damage to deal
*/
void ai_damage_player(Uint16 damage);

/*
* @brief stop player movement for a brief period
*/
void ai_freeze_player();

/*
* @brief get the player pos from the ai system
* @return the player's position, or {0, 0, 0} if player cannot be found
*/
GFC_Vector3D ai_get_player_pos();

/*
* @brief update the monster ai
* @param ai the ai to update
* @param position the position of the monster
*/
void ai_update_monster(MonsterAI *ai, GFC_Vector3D position);

/*
* @brief set the ai move state
* @param ai the ai to set the move state for
* @param moveState: the new move state for the ai
*/
void ai_set_move_state(MonsterAI *ai, Uint32 moveState);

#endif

