#include "simple_logger.h"

#include "gfc_config.h"
#include "gfc_config_def.h"
#include "gfc_actions.h"
#include "gfc_audio.h"

#include "ai.h"
#include "monster.h"

typedef struct
{
	MonsterAI			*ai;			/**<monster ai*/
	Sint16				health;			/**<monster health*/
	Uint16				damage;			/**<monster damage*/
	Uint8				walkSpeed;		/**<walk speed*/
	Uint8				runSpeed;		/**<run speed*/
	Uint8				dodgeLeft;		/**<whether to dodge left or right*/
} MonsterData;

static int monster_think(Entity *self);
static int monster_update(Entity *self);
static int monster_draw(Entity *self);
static void monster_free(Entity *self);
static void monster_move(Entity *self);
static void monster_punch(Entity *self);
static void monster_grab(Entity *self);
static void monster_shoot(Entity *self);
static void monster_dodge(Entity *self);
static void monster_die(Entity *self);
static void monster_handle_touches(Entity *self);

Entity *monster_spawn(const char *monsterName, GFC_Vector3D position)
{
	int i, c;
	SJson *json, *monsterList, *monster;
	Entity *self;
	MonsterData *data;
	MonsterAI *ai;
	Model *model;
	GFC_Box bounds;
	GFC_Color color;
	const char *str1, *str2;

	if (!monsterName) return NULL;

	json = sj_load("defs/entities/monsters.json");
	if (!json)
	{
		slog("failed to load item file");
		return NULL;
	}
	monsterList = sj_object_get_value(json, "monster");
	if (!monsterList)
	{
		slog("cannot load monster names from item pickup file");
		sj_free(json);
		return NULL;
	}
	c = sj_array_get_count(monsterList);
	for (i = 0; i < c; i++)
	{
		monster = sj_array_get_nth(monsterList, i);
		if (!monster) continue;
		str1 = sj_object_get_string(monster, "name");
		if (gfc_strlcmp(str1, monsterName) == 0) break;
	}
	if (i == c)
	{
		slog("failed to find monster '%s' in monster file", monsterName);
		sj_free(json);
		return NULL;
	}
	str2 = sj_object_get_string(monster, "model");
	if (!str2)
	{
		slog("failed to locate model for monster '%s'", monsterName);
		sj_free(json);
		return NULL;
	}
	model = model_load(str2);
	if (!model)
	{
		slog("failed to load model for monster '%s'", monsterName);
		sj_free(json);
		return NULL;
	}
	bounds = gfc_box_from_config(sj_object_get_value(monster, "bounds"));
	if (bounds.w == 0 && bounds.h == 0 && bounds.d == 0)
	{
		slog("failed to load bounds for monster '%s'", monsterName);
		sj_free(json);
		return NULL;
	}
	if (!sj_object_get_color_value(monster, "color", &color)) color = GFC_COLOR_WHITE;

	self = entity_new();
	if (!self)
	{
		slog("failed to allocate memory for monster entity");
		sj_free(json);
		return NULL;
	}
	gfc_line_cpy(self->name, str1);
	self->layer = EL_MONSTER;
	self->mask = MONSTER_MASK;
	self->model = model;
	self->position = position;
	self->bounds = bounds;
	self->scale = gfc_vector3d(2, 2, 2);

	self->bounds.w = self->bounds.w * self->scale.x;
	self->bounds.h = self->bounds.h * self->scale.y;
	self->bounds.d = self->bounds.d * self->scale.z;

	self->bounds.x = position.x - 0.5 * self->bounds.w;
	self->bounds.y = position.y - 0.5 * self->bounds.h;
	self->bounds.z = position.z - 0.5 * self->bounds.d;

	self->color = color;

	self->think = monster_think;
	self->update = monster_update;
	self->draw = monster_draw;
	self->free = monster_free;

	ai = gfc_allocate_array(sizeof(MonsterAI), 1);
	if (!ai)
	{
		slog("failed to allocate AI for monster");
		entity_free(self);
		return NULL;
	}
	ai->moveState = AIMS_IDLE;
	ai->dir = gfc_vector3d(0, 0, 0);
	if (gfc_strlcmp(self->name, "shooter") == 0) ai->rangeAttack = 1;
	else ai->rangeAttack = 0;

	self->data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (self->data)
	{
		data = (MonsterData*)self->data;
		data->health = 100;
		data->damage = 20;
		data->walkSpeed = 0.2;
		data->runSpeed = 0.5;
		data->dodgeLeft = 0;
		data->ai = ai;
		if (gfc_strlcmp(self->name, "brute") == 0) data->damage *= 3;
	}
	else
	{
		slog("failed to allocate monster data for monster");
		entity_free(self);
		free(ai);
		return NULL;
	}

	return self;
}

static int monster_think(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return 0;
	data = (MonsterData*) self->data;

	if (!data->ai) return 0;
	ai = (MonsterAI*) data->ai;

	self->velocity.x = 0;

	// decide what to do next
	if (ai->nextAction == AINA_ATTACK)
	{
		if (gfc_strlcmp(self->name, "grabber") == 0) monster_grab(self);
		else if (gfc_strlcmp(self->name, "shooter") == 0) monster_shoot(self);
		else if (gfc_strlcmp(self->name, "dodger") == 0) monster_dodge(self);
		else monster_punch(self);
	}
	else if (ai->nextAction == AINA_MOVE)
	{
		if (gfc_strlcmp(self->name, "shooter") != 0) monster_move(self);
	}

	// gravity
	self->velocity.z = -0.125;

	// collision detection
	if (!entity_move_check(self)) self->velocity = gfc_vector3d(0, 0, 0);

	monster_handle_touches(self);

	return 1;
}

static int monster_update(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return 0;
	data = (MonsterData*) self->data;

	if (!data->ai) return 0;
	ai = (MonsterAI*) data->ai;

	// kill monster if it is out of health
	if (data->health <= 0)
	{
		monster_die(self);
		return 1;
	}

	// update position
	gfc_vector3d_add(self->position, self->position, self->velocity);

	// update bounding box
	self->bounds.x = self->position.x - 0.5 * self->bounds.w;
	self->bounds.y = self->position.y - 0.5 * self->bounds.h;
	self->bounds.z = self->position.z - 0.5 * self->bounds.d;

	// update monster ai
	ai_update_monster(data->ai, self->position);

	return 1;
}

static int monster_draw(Entity *self)
{
	if (!self) return 0;
	return 0;
}

static void monster_free(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (data->ai)
	{
		ai = (MonsterAI*) data->ai;
		free(ai);
	}
	free(data);
}

static void monster_move(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	switch (ai->moveState)
	{
	case AIMS_IDLE:
		return;

	case AIMS_WALK:
		/*
		self->velocity = ai->dir;
		gfc_vector3d_scale(self->velocity, self->velocity, data->walkSpeed);*/
		break;

	case AIMS_RUN:
		break;
	}
}

static void monster_punch(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;
	GFC_Sound *sound;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	sound = gfc_sound_load("defs/audio/punch.mp3", 1, 1);
	gfc_sound_play(sound, 0, 30, -1);
	gfc_sound_free(sound);

	ai_damage_player(data->damage);
}

static void monster_grab(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;
	GFC_Sound *sound;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	sound = gfc_sound_load("defs/audio/grab.mp3", 1, 1);
	gfc_sound_play(sound, 0, 100, -1);
	gfc_sound_free(sound);

	ai_freeze_player();
}

static void monster_shoot(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;
	GFC_Sound *sound;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	sound = gfc_sound_load("defs/audio/gunshot.wav", 1, 1);
	gfc_sound_play(sound, 0, 20, -1);
	gfc_sound_free(sound);
	ai_damage_player(data->damage);
}

static void monster_dodge(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	if (data->dodgeLeft)
	{
		self->velocity.x = -5;
		data->dodgeLeft = 0;
	}
	else
	{
		self->velocity.x = 5;
		data->dodgeLeft = 1;
	}
}

static void monster_die(Entity *self)
{
	GFC_Sound *sound;

	if (!self) return;
	sound = gfc_sound_load("defs/audio/monster_die.wav", 1, 1);
	gfc_sound_play(sound, 0, 30, -1);
	gfc_sound_free(sound);
	entity_free(self);
}

static void monster_handle_touches(Entity *self)
{
	int i, c;
	Entity *other;
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	entity_get_touches(self); // get entities touched this frame

	c = gfc_list_get_count(self->touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->touches, i);
		if (!other) continue;

		if (other->layer == EL_PLAYER)
		{
			self->velocity.x = 0;
			self->velocity.y = 0;
		}
		else if (other->layer == EL_MONSTER)
		{
			self->velocity.x = 0;
			self->velocity.y = 0;
		}
		else if (other->layer == EL_PROJECTILE)
		{
			continue;
		}
		else if (other->layer == EL_WORLD)
		{
			continue;
		}
	}
}

void monster_damage(Entity *self, Uint32 damage)
{
	MonsterData *data;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	data->health -= damage;
}