#include <SDL.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Player.h"
#include "Weapon.h"
#include "Hud.h"
#include "ParticleSystem.h"
#include "Sound.h"
#include "IniFile.h"
#include "SDL_Video.h"

static byte monsterWpInfo[28] = {
	1, 50,
	1, 50,
	3, 20,
	1, 50,
	3, 25,
	1, 50,
	3, 25,
	1, 30,
	1, 50,
	1, 50,
	1, 50,
	1, 50,
	1, 50,
	3, 25 };

static byte wpinfo[72] = { 
	1, 30, 20, 25, 20, 0, 
	1, 50, 0, 15, 0, 0, 
	1, 35, 0, 15, 0, 0, 
	1, 35, 0, 12, 0, 0, 
	3, 15, 0, 15, 0, 0, 
	1, 40, 0, 15, 0, 0, 
	3, 10, 0, 15, 0, 0, 
	1, 50, 0, 15, 0, 0, 
	1, 50, 0, 13, 0, 5, 
	1, 40, 20, 15, 20, 8, 
	1, 40, 20, 15, 20, 8, 
	1, 40, 20, 15, 20, 8 };

// Default weapon stats (hardcoded fallback)
static const int defaultWeaponStats[19][8] = {
	// strMin, strMax, rangeMin, rangeMax, ammoType, ammoUsage, damage, resourceID
	{ 3, 12, 0, 70, 0, 0, 25, 5044 },     // 0: Axe
	{ 1, 2, 0, 100, 0, 1, 204, 5045 },    // 1: Fire Ext
	{ 6, 7, 5, 80, 1, 1, 102, 5046 },     // 2: Pistol
	{ 6, 10, 2, 80, 2, 1, 128, 5047 },    // 3: Shotgun
	{ 3, 6, 3, 90, 1, 3, 102, 5048 },     // 4: Chaingun
	{ 12, 18, 1, 90, 2, 2, 51, 5049 },    // 5: Super Shotgun
	{ 6, 8, 4, 90, 4, 3, 230, 5050 },     // 6: Plasma Gun
	{ 15, 36, 8, 70, 3, 1, 128, 5051 },   // 7: Rocket Launcher
	{ 60, 105, 8, 100, 4, 15, 76, 5052 }, // 8: BFG
	{ 5, 9, 0, 75, 5, 0, 25, 5088 },      // 9: Hell Hound
	{ 8, 12, 0, 75, 5, 0, 25, 5088 },     // 10: Cerberus
	{ 15, 18, 0, 75, 5, 0, 25, 5088 },    // 11: Demon Wolf
	{ 3, 5, 0, 90, 0, 0, 128, 5096 },     // 12: Melee Attack 1
	{ 4, 7, 0, 80, 0, 0, 128, 0 },        // 13: Melee Attack 2
	{ 5, 15, 0, 70, 0, 0, 128, 0 },       // 14: Melee Attack 3
	{ 4, 10, 3, 85, 0, 0, 128, 5072 },    // 15: Imp/Caco/Pain/Vile Missile
	{ 10, 20, 3, 75, 0, 0, 128, 0 },      // 16: Boss Missile
	{ 15, 30, 2, 80, 0, 0, 128, 0 },      // 17: Rocket Missile
	{ 0, 0, 0, 0, 0, 0, 0, 0 }            // 18: Null Attack
};

// Default monster stats (hardcoded fallback)
static const int defaultMonsterStats[14][7] = {
	// type, health, armor, defense, strength, agility, accuracy
	{ 0, 5, 4, 13, 13, 13, 13 },       // 0: Zombie
	{ 1, 7, 3, 12, 14, 12, 12 },       // 1: Hellhound
	{ 2, 10, 5, 14, 7, 13, 12 },       // 2: Troop
	{ 3, 9, 4, 13, 14, 13, 13 },       // 3: Imp
	{ 4, 6, 3, 14, 6, 15, 15 },        // 4: Lost Soul
	{ 5, 10, 5, 14, 16, 12, 13 },      // 5: Pinky
	{ 6, 13, 7, 12, 10, 13, 15 },      // 6: Cacodemon
	{ 7, 15, 8, 14, 12, 12, 12 },      // 7: Pain Elemental
	{ 8, 13, 8, 16, 14, 15, 15 },      // 8: Revenant
	{ 9, 20, 10, 16, 14, 15, 15 },     // 9: Mancubus
	{ 10, 18, 9, 15, 17, 14, 16 },     // 10: Archvile
	{ 11, 25, 15, 16, 15, 15, 15 },    // 11: Baron
	{ 12, 600, 400, 45, 30, 35, 40 },  // 12: Cyberdemon
	{ 13, 400, 250, 30, 35, 40, 55 }   // 13: Kronos
};

// Load weapon stats from INI file or use defaults
static void Combat_loadWeaponStats(Combat_t* combat)
{
	IniFile_t* ini = IniFile_load("data/weapons.ini");
	char sectionName[32];
	
	for (int i = 0; i < 19; i++) {
		SDL_snprintf(sectionName, sizeof(sectionName), "Weapon.%d", i);
		
		int strMin, strMax, rangeMin, rangeMax, ammoType, ammoUsage, damage;
		short resourceID;
		
		if (ini != NULL && IniFile_hasSection(ini, sectionName)) {
			strMin = IniFile_getInt(ini, sectionName, "strMin", defaultWeaponStats[i][0]);
			strMax = IniFile_getInt(ini, sectionName, "strMax", defaultWeaponStats[i][1]);
			rangeMin = IniFile_getInt(ini, sectionName, "rangeMin", defaultWeaponStats[i][2]);
			rangeMax = IniFile_getInt(ini, sectionName, "rangeMax", defaultWeaponStats[i][3]);
			ammoType = IniFile_getInt(ini, sectionName, "ammoType", defaultWeaponStats[i][4]);
			ammoUsage = IniFile_getInt(ini, sectionName, "ammoUsage", defaultWeaponStats[i][5]);
			damage = IniFile_getInt(ini, sectionName, "damage", defaultWeaponStats[i][6]);
			resourceID = (short)IniFile_getInt(ini, sectionName, "resourceID", defaultWeaponStats[i][7]);
		} else {
			strMin = defaultWeaponStats[i][0];
			strMax = defaultWeaponStats[i][1];
			rangeMin = defaultWeaponStats[i][2];
			rangeMax = defaultWeaponStats[i][3];
			ammoType = defaultWeaponStats[i][4];
			ammoUsage = defaultWeaponStats[i][5];
			damage = defaultWeaponStats[i][6];
			resourceID = (short)defaultWeaponStats[i][7];
		}
		
		Weapon_initWeapon(&combat->weaponInfo[i], strMin, strMax, rangeMin, rangeMax, ammoType, ammoUsage, damage, resourceID);
	}
	
	if (ini != NULL) {
		IniFile_free(ini);
		printf("Combat_loadWeaponStats: Loaded weapon stats from INI\n");
	} else {
		printf("Combat_loadWeaponStats: Using default weapon stats\n");
	}
}

// Load monster stats from INI file or use defaults
static void Combat_loadMonsterStats(Combat_t* combat)
{
	IniFile_t* ini = IniFile_load("data/monsters.ini");
	char sectionName[32];
	
	for (int i = 0; i < 14; i++) {
		SDL_snprintf(sectionName, sizeof(sectionName), "Monster.%d", i);
		
		int type, health, armor, defense, strength, agility, accuracy;
		
		if (ini != NULL && IniFile_hasSection(ini, sectionName)) {
			type = i; // Type is always the index
			health = IniFile_getInt(ini, sectionName, "health", defaultMonsterStats[i][1]);
			armor = IniFile_getInt(ini, sectionName, "armor", defaultMonsterStats[i][2]);
			defense = IniFile_getInt(ini, sectionName, "defense", defaultMonsterStats[i][3]);
			strength = IniFile_getInt(ini, sectionName, "strength", defaultMonsterStats[i][4]);
			agility = IniFile_getInt(ini, sectionName, "agility", defaultMonsterStats[i][5]);
			accuracy = IniFile_getInt(ini, sectionName, "accuracy", defaultMonsterStats[i][6]);
		} else {
			type = defaultMonsterStats[i][0];
			health = defaultMonsterStats[i][1];
			armor = defaultMonsterStats[i][2];
			defense = defaultMonsterStats[i][3];
			strength = defaultMonsterStats[i][4];
			agility = defaultMonsterStats[i][5];
			accuracy = defaultMonsterStats[i][6];
		}
		
		CombatEntity_initCombatEntity(&combat->monsters[i], type, health, armor, defense, strength, agility, accuracy);
	}
	
	if (ini != NULL) {
		IniFile_free(ini);
		printf("Combat_loadMonsterStats: Loaded monster stats from INI\n");
	} else {
		printf("Combat_loadMonsterStats: Using default monster stats\n");
	}
}

Combat_t* Combat_init(Combat_t* combat, DoomRPG_t* doomRpg)
{
	printf("Combat_init\n");

	if (combat == NULL)
	{
		combat = SDL_malloc(sizeof(Combat_t));
		if (combat == NULL) {
			return NULL;
		}
	}
	SDL_memset(combat, 0, sizeof(Combat_t));

	SDL_memset(&combat->bMobj, 0, sizeof(CombatEntity_t));
	SDL_memset(&combat->aMobj, 0, sizeof(CombatEntity_t));
	SDL_memset(combat->weaponInfo, 0, (sizeof(Weapon_t) * 19));
	SDL_memset(combat->monsters, 0, (sizeof(CombatEntity_t) * 14));
	combat->doomRpg = doomRpg;
	CombatEntity_initCombatEntity(&combat->aMobj, -1, 0, 0, 1, 1, 1, 1);
	CombatEntity_initCombatEntity(&combat->bMobj, -1, 0, 0, 25, 1, 1, 1);
	
	// Load weapon and monster stats from INI files (with hardcoded fallbacks)
	Combat_loadWeaponStats(combat);
	Combat_loadMonsterStats(combat);
	
	combat->gotCrit = false;
	combat->kronosTeleporterDest = false;

	return combat;
}

void Combat_free(Combat_t* combat, boolean freePtr)
{
	for (int i = 0; i < 14; i++) {
		SDL_memset(&combat->monsters[i], 0, sizeof(combat->monsters[i]));
	}
	SDL_memset(&combat->aMobj, 0, sizeof(combat->aMobj));
	SDL_memset(&combat->bMobj, 0, sizeof(combat->bMobj));
	if (freePtr) {
		SDL_free(combat);
	}
}

int Combat_calcParticleIntensity(Combat_t* combat, int i)
{
	int b = CombatEntity_getMaxHealth(&combat->curTarget->monster->ce);
	if (CombatEntity_getArmor(&combat->curTarget->monster->ce) > 0) {
		b += CombatEntity_getMaxArmor(&combat->curTarget->monster->ce);
	}

	int i2 = ((((combat->damage + combat->armorDamage) << 16) / (b << 8)) * 12288) >> 8;
	int a = ((CombatEntity_getHealth(&combat->curTarget->monster->ce) + CombatEntity_getArmor(&combat->curTarget->monster->ce)) - combat->damage) - combat->armorDamage;
	if (a <= 0) {
		i2 = (i2 * 512) >> 8;
		if (i == 1 && (a <= (-(((CombatEntity_getMaxHealth(&combat->curTarget->monster->ce) << 16) / 637) >> 8)) || combat->curTarget->def->eSubType == 13)) {
			combat->f340c = true;
			combat->curTarget->info |= 0x20000;
		}
	}
	int i4 = i2 + 128;
	if (i4 < 256) {
		i4 = 256;
	}
	return i4 >> 8;
}

int Combat_calcHit(Combat_t* combat, Entity_t* entity, int i)
{
	Player_t* player;
	int weapon;
	
	player = combat->doomRpg->player;
	weapon = player->weapon;

	CombatEntity_setAgility(&combat->aMobj, ((CombatEntity_getAccuracy(&player->ce) << 8) * 204) >> 16);

	if (CombatEntity_calcHit(combat->doomRpg, &combat->doomRpg->player->ce, &combat->weaponInfo[weapon], &combat->aMobj, i) == COMBATENTITY_MISS) {
		return 0;
	}

	if (entity == NULL || !(entity->info & 0x40000)) {
		return 0;
	}

	if (entity->def->eType == 10) { // File
		return (combat->doomRpg->player->weapon == 1) ? 1 : 0;
	}
	else if (entity->def->eSubType == 3) {
		return (combat->doomRpg->player->weapon == 0) ? 1 : 0;
	}
	else {
		return (combat->doomRpg->player->weapon == 1) ? 0 : 1;
	}
}

void Combat_worldDistToTileDist(Combat_t* combat)
{
	DoomCanvas_t* doomCanvas;
	doomCanvas = combat->doomRpg->doomCanvas;

	combat->worldDist = Entity_calcWorldDistance(combat->curTarget, doomCanvas->viewX, doomCanvas->viewY);
	combat->tileDist = -1;
	if (combat->worldDist <= 4096) {
		combat->tileDist = 1;
	}
	else if (combat->worldDist <= 16384) {
		combat->tileDist = 2;
	}
	else if (combat->worldDist <= 36864) {
		combat->tileDist = 3;
	}
	else if (combat->worldDist <= 65536) {
		combat->tileDist = 4;
	}
}

void Combat_drawWeapon(Combat_t* combat, int x, int y)
{
	DoomCanvas_t* doomCanvas;
	DoomRPG_t* doomRpg;
	Player_t* player;
	Render_t* render;
	boolean damageBlend;
	int pX, pY, scale;

	doomRpg = combat->doomRpg;
	doomCanvas = doomRpg->doomCanvas;
	player = doomRpg->player;
	render = doomRpg->render;

	if (player->weapons) {
		boolean z = false;
		if (doomCanvas->state == ST_COMBAT && combat->curAttacker == NULL && combat->stage != ST_MENU) {
			pX = wpinfo[(player->weapon * 6) + FLD_WP_ATKX];
			pY = wpinfo[(player->weapon * 6) + FLD_WP_ATKY];

			if (combat->f336b) {
				if (doomCanvas->time < combat->endFrameTime) {
					z = true;
				}
			}
		}
		else {
			pX = wpinfo[(player->weapon * 6) + FLD_WP_IDLEX];
			pY = wpinfo[(player->weapon * 6) + FLD_WP_IDLEY];
		}

		damageBlend = ((doomRpg->doomCanvas->time < doomRpg->hud->damageTime) && (doomRpg->hud->damageCount > 0)) ? true : false;

		scale = (render->screenWidth << FRACBITS) / 0x8000;

		Render_draw2DSprite(render, 
			240 + player->weapon, 
			z ? 1 : 0, 
			doomCanvas->SCR_CX + ((((((pX + x) - 32) << 8) * scale) + 0xff00) >> FRACBITS),
			render->screenHeight - (((((64 - (pY + y)) << 8) * scale) + 0xff00) >> FRACBITS), 0, damageBlend);
	}
}

void Combat_explodeOnMonster(Combat_t* combat)
{
	Sprite_t* sprite;

	if (combat->hitType != 0 && combat->tileDist != -1) {

		if (combat->curTarget->def->eType == 1) { // Enemy
			Combat_spawnBloodParticles(combat, combat->tileDist, combat->bloodColor, -1);
			if (combat->f340c && (combat->attackerWeaponId != 4 || combat->animLoopCount == 0)) {
				sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
				sprite->info |= 0x10000;
				if (!combat->doomRpg->doomCanvas->slowBlit) {
					DoomCanvas_updateViewTrue(combat->doomRpg->doomCanvas);
				}
			}
		}
		else if (combat->curTarget->def->eType == 12) {
			if (combat->curTarget->def->eSubType == 3) {
				Combat_spawnParticlesJammedDoor(combat, 0xBBBBBB);
			}
			else if (combat->curTarget->def->eSubType == 4 || combat->curTarget->def->eSubType == 2) {
				Combat_spawnParticlesDestructibleObject(combat, combat->tileDist);
			}
		}
		else if (combat->curTarget->def->eType == 10) {
			Combat_spawnParticlesFire(combat, combat->tileDist, 0xCCCC00);
		}
	}
}

void Combat_explodeOnPlayer(Combat_t* combat)
{
	if (combat->curTarget) {
		return;
	}
	if (combat->hitType == 0) {
		return;
	}
	Player_painEvent(combat->doomRpg->player, combat->curAttacker);
}

int Combat_getMonsterWeaponInfo(Combat_t* combat, int indx, int field)
{
	return monsterWpInfo[(indx * 2) + field];
}

void Combat_launchProjectile(Combat_t* combat)
{
	GameSpriteMissile_t* gSprMissile;
	GameSprite_t* gSprite;
	DoomCanvas_t* doomCanvas;
	DoomRPG_t* doomRpg;
	
	Sprite_t* sprite;
	byte renderMode;
	int missileAnim;
	int missileX, missileY, viewX, viewY, mDeltaX, mDeltaY;
	boolean v24;

	doomRpg = combat->doomRpg;
	doomCanvas = doomRpg->doomCanvas;

	renderMode = 0;
	switch (combat->attackerWeaponId) {
		case 6:
			renderMode = 7;
			missileAnim = 200;
			break;
		case 7:
			missileAnim = combat->curAttacker ? 203 : 201;
			break;
		case 8:
			renderMode = 7;
			missileAnim = 202;
			break;
		case 12:
		case 13:
		case 14:
			missileAnim = 0;
			break;
		case 15:
			renderMode = 7;
			missileAnim = (combat->curAttacker->def->eSubType == 10) ? 0 : 204;
			break;
		case 16:
			missileAnim = 205;
			break;
		case 17:
			missileAnim = 206;
			break;
		default:
			missileAnim = 0;
			combat->exploded = true;
			return;
	}

	combat->missileAnim = missileAnim;

	if (combat->curAttacker == NULL) {
		sprite = &doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
		missileX = sprite->x;
		missileY = sprite->y;
	}
	else if (combat->curAttacker->monster->target) {
		sprite = &doomRpg->render->mapSprites[(combat->curAttacker->monster->target->info & 0xFFFF) - 1];
		missileX = sprite->x;
		missileY = sprite->y;
	}
	else {
		missileX = doomCanvas->viewX;
		missileY = doomCanvas->viewY;
	}

	v24 = true;
	if (combat->curAttacker == NULL) {
		viewX = doomCanvas->viewX + doomCanvas->viewStepX;
		viewY = doomCanvas->viewY + doomCanvas->viewStepY;
		mDeltaX = missileX - viewX;
		mDeltaY = missileY - viewY;

		if (mDeltaX < 0) {
			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileY += 25;
				}
				else {
					missileY -= 25;
				}
			}
		}
		else if (mDeltaX > 0) {
			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileY += 25;
				}
				else {
					missileY -= 25;
				}
			}
		}

		if (mDeltaY < 0) {
			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileX += 25;
				}
				else {
					missileX -= 25;
				}
			}
		}
		else if (mDeltaY > 0) {
			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileX += 25;
				}
				else {
					missileX -= 25;
				}
			}
		}
		else {
			v24 = false;
		}
	}
	else {
		viewX = combat->curAttacker->monster->x;
		viewY = combat->curAttacker->monster->y;
		mDeltaY = missileY - viewY;
		mDeltaX = missileX - viewX;

		byte uVar4;

		if ((((combat->curAttacker->def->eSubType == 10) || (combat->attackerWeaponId == 12)) || (combat->attackerWeaponId == 13)) || (combat->attackerWeaponId == 14)) {
			uVar4 = 1;
		}
		if ((((combat->curAttacker->def->eSubType != 10) && (combat->attackerWeaponId != 12)) && (combat->attackerWeaponId != 13)) && (combat->attackerWeaponId != 14)) {
			uVar4 = 0;
		}

		if (mDeltaX < 0) {
			if (uVar4 != 0) {
				viewX = (combat->missileZHeight + missileX) - 1;
			}

			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileY += 25;
				}
				else {
					missileY -= 25;
				}
			}
			else {
				missileX += combat->missileZHeight;
			}
		}
		else if (mDeltaX > 0) {
			if (uVar4 != 0) {
				viewX = (missileX - combat->missileZHeight) - 1;
			}

			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileY += 25;
				}
				else {
					missileY -= 25;
				}
			}
			else {
				missileX -= combat->missileZHeight;
			}
		}

		if (mDeltaY < 0) {
			if (uVar4 != 0) {
				viewY = (missileY + combat->missileZHeight) - 1;
			}

			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileX += 25;
				}
				else {
					missileX -= 25;
				}
			}
			else {
				missileY += combat->missileZHeight;
			}
		}
		else if (mDeltaY > 0) {
			if (uVar4 != 0) {
				viewY = (missileY - combat->missileZHeight) - 1;
			}

			if (combat->hitType == 0) {
				if ((DoomRPG_randNextByte(&doomRpg->random) & 1)) {
					missileX += 25;
				}
				else {
					missileX -= 25;
				}
			}
			else {
				missileY -= combat->missileZHeight;
			}
		}
		else {
			v24 = false;
		}
	}

	gSprMissile = &combat->activeMissiles[combat->numActiveMissiles++];
	gSprite = Game_gsprite_alloc(combat->doomRpg->game, combat->missileAnim, 0, viewX, viewY);
	gSprMissile->gsMissile = gSprite;
	if (combat->missileAnim == 0) {
		gSprite->sprite->info |= 0x10000;
	}

	gSprMissile->gsMissile->sprite->renderMode = renderMode;
	gSprMissile->posX = missileX;
	gSprMissile->posY = missileY;
	gSprMissile->unk0 = v24;

	combat->exploded = false;
}

int Combat_monsterSeq(Combat_t* combat)
{
	DoomCanvas_t* doomCanvas;
	Sprite_t* sprite;
	Player_t* player;
	Hud_t* hud;
	char* msgBuff;
	int snd;

	doomCanvas = combat->doomRpg->doomCanvas;
	player = combat->doomRpg->player;
	hud = combat->doomRpg->hud;

	switch (combat->stage) {
		case 0:
			combat->f336b = false;
			combat->stage = 1;
			combat->animEndTime = doomCanvas->time + 250;
			combat->frameTime = 0;
			combat->totalDamage = 0;
			combat->totalArmorDamage = 0;
			combat->hitType = 0;
			combat->f342d = 0;
			combat->gotCrit = false;
			msgBuff = Hud_getMessageBuffer(hud);
			SDL_snprintf(msgBuff, MS_PER_CHAR, (combat->attackerWeaponId == 18) ? "%s casts raise%c" : "%s attacks%c", combat->curAttacker->def->name, 0x7f);
			Hud_finishMessageBuffer(hud);
			break;
		case 1:
			if ((combat->nextStageTime != 0) && (combat->numActiveMissiles == 0)) {
				if (doomCanvas->time > combat->animEndTime) {
					combat->doomRpg->render->mapSprites[(combat->curAttacker->info & 0xFFFF) - 1].info &= 0xffffe1ff;
					if (doomCanvas->time > combat->nextStageTime) {
						combat->stage = 2;
						combat->nextStageTime = 0;
					}
				}
				return false;
			}

			if (combat->frameTime == 0) {

				if (doomCanvas->time <= combat->animEndTime) {
					Combat_updateProjectile(combat);
					return false;
				}

				if (combat->animLoopCount <= 0 && !combat->f336b) {
					Combat_updateProjectile(combat);
					return 0;
				}

				combat->f336b = !combat->f336b;

				sprite = &combat->doomRpg->render->mapSprites[(combat->curAttacker->info & 0xFFFF) - 1];

				if (combat->f336b) {
					sprite->info = (sprite->info & 0xffffe1ff) | (combat->attackFrame << 9);
					combat->frameTime = doomCanvas->time + 150;

					if (combat->curTarget == NULL) {

						combat->hitType = CombatEntity_calcHit(combat->doomRpg, 
							&combat->curAttacker->monster->ce,
							combat->weaponInfo + combat->attackerWeaponId,
							&combat->doomRpg->player->ce, 0);

						if (combat->hitType) {
							if (combat->hitType == 2 && combat->animLoopCount == monsterWpInfo[(combat->curAttacker->def->eSubType * 2) + 0]) {
								combat->gotCrit = true;
							}

							CombatEntity_calcDamage(combat->doomRpg, 
								&combat->curAttacker->monster->ce,
								combat->weaponInfo + combat->attackerWeaponId, &combat->doomRpg->player->ce,
								combat->gotCrit ? 512 : 256, 0, &combat->damage, &combat->armorDamage);
						}
						else {
							combat->armorDamage = 0;
							combat->damage = 0;
						}
					}
					else {
						combat->hitType = 1;
						combat->damage = 1;
						combat->armorDamage = 0;
					}
					combat->totalDamage = combat->totalDamage + combat->damage;
					combat->totalArmorDamage = combat->totalArmorDamage + combat->armorDamage;
					--combat->animLoopCount;
				}
				else {
					sprite->info &= 0xffffe1ff;
				}

				DoomCanvas_updateViewTrue(doomCanvas);
				combat->animEndTime = doomCanvas->time + ((10 * monsterWpInfo[(combat->curAttacker->def->eSubType * 2) + 1]));
			}

			if (combat->frameTime != 0 && doomCanvas->time > combat->frameTime) {
				if (combat->attackerWeaponId == 18) {
					Sound_playSound(combat->doomRpg->sound, 5124, 0, 3);
				}
				else {
					// Attack Sound
					snd = EntityMonster_getSoundID(combat->curAttacker->monster, (combat->curAttacker->info & 0x800000) ? 5:4);
					if (snd != 0) {
						Sound_playSound(combat->doomRpg->sound, snd, 0, 3);
					}
				}

				Combat_launchProjectile(combat);
				combat->frameTime = 0;

				Combat_updateProjectile(combat);
				return false;
			}
			else {
				Combat_updateProjectile(combat);
				return false;
			}

			break;
		case 2:
			combat->f336b = false;
			combat->curAttacker->info &= 0xfffeffff;

			if (combat->curTarget == NULL) {
				if (combat->totalDamage + combat->totalArmorDamage == 0) {
					Hud_addMessage(hud, "Dodged!");
				}
				else {
					Player_pain(player, combat->totalDamage, combat->totalArmorDamage);
				}
			}
			else if (combat->attackerWeaponId == 18) {
				sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
				if (Game_findMapEntityXYFlag(combat->doomRpg->game, sprite->x, sprite->y, 16263) == NULL) {
					sprite->info &= 0xfeffe1ff;
					combat->curTarget->info |= 0x40000;
					combat->curTarget->info &= 0xfdffffff;
					CombatEntity_setHealth(&combat->curTarget->monster->ce, CombatEntity_getMaxHealth(&combat->curTarget->monster->ce));
					CombatEntity_setArmor(&combat->curTarget->monster->ce, CombatEntity_getMaxArmor(&combat->curTarget->monster->ce));
					combat->curTarget->monster->nextAttacker = NULL;
					Game_activate(combat->doomRpg->game, combat->curTarget);
					Game_linkEntity(combat->doomRpg->game, combat->curTarget, combat->curTarget->linkIndex % 32, combat->curTarget->linkIndex / 32);
					
					msgBuff = Hud_getMessageBuffer(hud);
					SDL_snprintf(msgBuff, MS_PER_CHAR, "%s is revived!", combat->curTarget->def->name);
					Hud_finishMessageBuffer(hud);

					DoomCanvas_checkFacingEntity(doomCanvas);
				}
				else {
					Hud_addMessage(hud, "Raise failed!");
				}
			}
			else {
				Entity_died(combat->curTarget);
			}

			return true;
			break;
	}

	return false;
}

void Combat_performAttack(Combat_t* combat, Entity_t* attacker, Entity_t* target)
{
	combat->curAttacker = attacker;
	combat->curTarget = target;

	if (combat->curAttacker == NULL) {
		combat->attackerWeaponId = combat->doomRpg->player->weapon;
		combat->missileZHeight = 48;
	}
	else {
		if (combat->curAttacker->info & 0x1000000) {
			combat->attackerWeaponId = 18;
			combat->attackFrame = 5;
		}
		else if (combat->curAttacker->info & 0x800000) {
			combat->attackerWeaponId = monsterAttacks[(combat->curAttacker->def->eSubType * 2) + 1];
			combat->attackFrame = 5;
		}
		else {
			combat->attackerWeaponId = monsterAttacks[(combat->curAttacker->def->eSubType * 2)];
			combat->attackFrame = 1;
		}
		combat->missileZHeight = 48;

		if (combat->curAttacker->def->eSubType == 10) {
			combat->missileZHeight = 30;
		}
	}

	combat->stage = 0;
	combat->nextStageTime = 0;
	combat->animEndTime = 0;
	combat->f336b = 0;

	if (combat->curAttacker) {
		combat->animLoopCount = monsterWpInfo[(combat->curAttacker->def->eSubType * 2) + MONSTER_WP_FIELD_NUMSHOTS];
	}
	else {
		combat->animLoopCount = wpinfo[(combat->attackerWeaponId * 6) + FLD_WP_NUMSHOTS];
	}

	combat->doomRpg->hud->msgCount = 0;
	DoomCanvas_setState(combat->doomRpg->doomCanvas, ST_COMBAT);
}

int teleportDestXY[8] = {0, 1, 0, -1, 1, 0, -1, 0};
boolean Combat_playerSeq(Combat_t* combat)
{
	DoomCanvas_t* doomCanvas;
	Sprite_t* sprite;
	Player_t* player;
	Weapon_t* wpn;
	Hud_t* hud;
	char* msgBuff;
	int msgLen, snd;
	int armor, dmgArmor;
	int teleportDestXYtmp[8], teleportOrder[4], tmp , indx1, indx2, i, x, y, viewX, viewY;

	doomCanvas = combat->doomRpg->doomCanvas;
	player = combat->doomRpg->player;
	hud = combat->doomRpg->hud;

	if (combat->stage == 0) {
		combat->f336b = false;
		combat->f340c = false;
		combat->stage = 1;
		combat->animEndTime = doomCanvas->time + 250;
		combat->frameTime = 0;
		combat->totalDamage = 0;
		combat->totalArmorDamage = 0;
		combat->hitType = 0;
		combat->f342d = 0;
		combat->gotCrit = false;
		combat->numActiveMissiles = 0;
		combat->endFrameTime = 0;
		combat->kronosTeleporter = 0;

		if (!doomCanvas->slowBlit) {
			DoomCanvas_updateViewTrue(doomCanvas);
		}

		Combat_worldDistToTileDist(combat);

		wpn = &combat->weaponInfo[combat->attackerWeaponId];
		if (combat->curTarget->def->eType == 1) {

			msgBuff = Hud_getMessageBuffer(hud);
			if (wpn->ammoUsage) {
				byte b = player->ammo[wpn->ammoType];
				if (b < wpn->ammoUsage) {
					SDL_snprintf(msgBuff, MS_PER_CHAR, "Attacking%c %s", 0x7f, "(Last shot!)");
				}
				else if (b < wpn->ammoUsage * 2) {
					SDL_snprintf(msgBuff, MS_PER_CHAR, "Attacking%c %s", 0x7f, "(1 shot left!)");
				}
				else if (b < wpn->ammoUsage * 3) {
					SDL_snprintf(msgBuff, MS_PER_CHAR, "Attacking%c %s", 0x7f, "(2 shots left!)");
				}
				else if (b < wpn->ammoUsage * 4) {
					SDL_snprintf(msgBuff, MS_PER_CHAR, "Attacking%c %s", 0x7f, "(3 shots left!)");
				}
			}
			Hud_finishMessageBuffer(hud);

			if (combat->curTarget->def->eSubType == 6 && combat->curTarget->def->parm == 467) {
				combat->bloodColor = 0x0000BB;
			}
			else if (combat->curTarget->def->eSubType == 11 && combat->curTarget->def->parm == 467) {
				combat->bloodColor = 0x00C000;
			}
			else {
				combat->bloodColor = 0xBB0000;
			}
		}
		else if (wpn->ammoUsage) {
			byte b = player->ammo[wpn->ammoType];
			if (b < wpn->ammoUsage) {
				Hud_addMessage(hud, "Last shot!");
			}
			else if (b < wpn->ammoUsage * 2) {
				Hud_addMessage(hud, "1 shot left!");
			}
			else if (b < wpn->ammoUsage * 3) {
				Hud_addMessage(hud, "2 shots left!");
			}
			else if (b < wpn->ammoUsage * 4) {
				Hud_addMessage(hud, "3 shots left!");
			}
		}
	}
	else if (combat->stage == 1) {

		if (combat->nextStageTime != 0) {
			if (doomCanvas->time > combat->animEndTime) {
				if (combat->f336b) {
					if (!doomCanvas->slowBlit) {
						DoomCanvas_updateViewTrue(doomCanvas);
					}
					combat->f336b = false;
				}
				if (doomCanvas->time > combat->nextStageTime) {
					combat->stage = 2; // combat->nextStage;
					combat->nextStageTime = 0;
				}
			}
			return false;
		}

		if (combat->frameTime == 0) {

			if (doomCanvas->time <= combat->animEndTime) {
				Combat_updateProjectile(combat);
				return false;
			}

			if (combat->animLoopCount <= 0 && !combat->f336b) {
				Combat_updateProjectile(combat);
				return false;
			}

			combat->f336b = !combat->f336b;

			if (!doomCanvas->slowBlit || (combat->f336b && (combat->animLoopCount == wpinfo[(combat->attackerWeaponId * 6) + 0]))) {
				DoomCanvas_updateViewTrue(doomCanvas);
			}

			if (combat->f336b) {
				combat->frameTime = doomCanvas->time + 150;
				combat->endFrameTime = doomCanvas->time + (10 * wpinfo[(combat->attackerWeaponId * 6) + 1]);
				combat->armorDamage = 0;
				combat->damage = 0;

				int n = (player->berserkerTics != 0) ? 768 : 256;
				if (combat->curTarget->def->eType == 1) { // Enemy
					combat->hitType = CombatEntity_calcHit(combat->doomRpg, &player->ce, &combat->weaponInfo[combat->attackerWeaponId], &combat->curTarget->monster->ce, combat->worldDist);

					if (combat->hitType != 0) {
						if (combat->hitType == 2 && combat->animLoopCount == wpinfo[combat->attackerWeaponId * 6 + 0]) {
							combat->gotCrit = true;
						}

						CombatEntity_calcDamage(combat->doomRpg, &player->ce,
							&combat->weaponInfo[combat->attackerWeaponId],
							&combat->curTarget->monster->ce, combat->gotCrit ? (n * 512 >> 8) : n, combat->worldDist,
							&combat->damage, &combat->armorDamage);

						armor = CombatEntity_getArmor(&combat->curTarget->monster->ce);
						dmgArmor = combat->damage;
						if (combat->armorDamage > armor) {
							dmgArmor += (combat->armorDamage - armor);
						}

						if (CombatEntity_getHealth(&combat->curTarget->monster->ce) - dmgArmor > 0) {
							sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
							sprite->info = sprite->info & 0xffffe1ffU | 0xc00;
							combat->curTarget->monster->animFrameTime = doomCanvas->time + 250;
						}

						if (combat->curTarget->def->eSubType == 13) {// Kronos
							if (combat->kronosTeleporterDest == false) {
								combat->kronosTeleporter = 1;
							}
						}
					}

					if (combat->curTarget->def->eSubType == 13) { // Kronos
						if (combat->hitType &&
							combat->animLoopCount == wpinfo[(combat->attackerWeaponId * 6) + 0] &&
							(combat->kronosTeleporter || DoomRPG_randNextByte(&combat->doomRpg->random) < 96))
						{
							SDL_memmove(teleportDestXYtmp, teleportDestXY, sizeof(teleportDestXY));

							teleportOrder[0] = 0;
							teleportOrder[1] = 1;
							teleportOrder[2] = 2;
							teleportOrder[3] = 3;
							for (i = 0; i < 4; ++i)
							{
								indx1 = DoomRPG_randNextByte(&combat->doomRpg->random) & 3;
								indx2 = DoomRPG_randNextByte(&combat->doomRpg->random) & 3;
								if (indx1 != indx2)
								{
									tmp = teleportOrder[indx2];
									teleportOrder[indx2] = teleportOrder[indx1];
									teleportOrder[indx1] = tmp;
								}
							}

							viewX = doomCanvas->viewX;
							viewY = doomCanvas->viewY;
							sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];

							i = 0;
							do {

								x = viewX + (teleportDestXYtmp[teleportOrder[i] * 2 + 0] << 6);
								y = viewY + (teleportDestXYtmp[teleportOrder[i] * 2 + 1] << 6);
								if (Game_findMapEntityXYFlag(combat->doomRpg->game, x, y, 0xF387) == NULL) {
									Game_gsprite_allocAnim(combat->doomRpg->game, 2, sprite->x, sprite->y);
									Game_unlinkEntity(combat->doomRpg->game, sprite->ent);
									sprite->ent->monster->x = sprite->x = x;
									sprite->ent->monster->y = sprite->y = y;
									Game_linkEntity(combat->doomRpg->game, sprite->ent, x >> 6, y >> 6);
									Render_relinkSprite(combat->doomRpg->render, sprite);
									combat->kronosTeleporter = true;
									combat->kronosTeleporterDest = true;
									combat->hitType = 0;
									combat->armorDamage = 0;
									combat->damage = 0;
									combat->gotCrit = false;
									break;
								}

							} while (++i < 4);
						}

						printf("KT: %d KTD: %d\n", combat->kronosTeleporter, combat->kronosTeleporterDest);
					}
				}
				else {
					combat->hitType = Combat_calcHit(combat, combat->curTarget, combat->worldDist);
					if (combat->curTarget->def->eSubType == 4) {
						CombatEntity_calcDamage(combat->doomRpg, &player->ce,
							&combat->weaponInfo[combat->attackerWeaponId],
							&combat->bMobj, combat->gotCrit ? (n * 512 >> 8) : n, combat->worldDist,
							&combat->damage, &combat->armorDamage);
					}
					else {
						CombatEntity_setDefense(&combat->aMobj, (CombatEntity_getStrength(&combat->doomRpg->player->ce) << 8) * 176 >> 16);

						CombatEntity_calcDamage(combat->doomRpg, &player->ce,
							&combat->weaponInfo[combat->attackerWeaponId],
							&combat->aMobj, combat->gotCrit ? (n * 512 >> 8) : n, combat->worldDist,
							&combat->damage, &combat->armorDamage);
					}
				}
				combat->totalDamage += combat->damage;
				combat->totalArmorDamage += combat->armorDamage;
				--combat->animLoopCount;

				combat->animEndTime = doomCanvas->time + (10 * wpinfo[(combat->attackerWeaponId * 6) + 1]);
			}
			else {
				combat->animEndTime = doomCanvas->time + (5 * wpinfo[(combat->attackerWeaponId * 6) + 1]);
			}
		}

		if (combat->frameTime != 0 && doomCanvas->time > combat->frameTime) {

			if (combat->attackerWeaponId == 8) {
				snd = 5052;
			}
			else if ((combat->attackerWeaponId == 0) && (combat->curTarget->def->eType == 1))
			{
				snd = 5136;
			}
			else {
				snd = (combat->attackerWeaponId + 5044);
			}
			Sound_playSound(combat->doomRpg->sound, snd, 0, 5);


			if (combat->kronosTeleporter == 0) {
				Combat_launchProjectile(combat);
			}

			if (combat->attackerWeaponId == 4 && combat->f340c && combat->curTarget->def->eType == 1 && combat->animLoopCount == 0) {
				Combat_spawnBloodParticles(combat, combat->tileDist, combat->bloodColor, -1);
				sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
				sprite->info |= 0x10000;
				DoomCanvas_updateViewTrue(doomCanvas);
			}

			combat->frameTime = 0;

			Combat_updateProjectile(combat);
			return false;
		}
		else {
			Combat_updateProjectile(combat);
			return false;
		}
	}
	else if (combat->stage == 2) {
		combat->f336b = false;
		if (combat->nextStageTime == 0) {

			DoomCanvas_updateViewTrue(doomCanvas);
			if (combat->totalDamage + combat->totalArmorDamage == 0) {
				if (combat->curTarget->def->eType == 1 && player->weapon != 1) {
					if (combat->kronosTeleporter != 0) {
						Hud_addMessage(hud, "Kronos Teleported!");
					}
					else {
						Hud_addMessage(hud, "Missed!");
					}
				}
				else {
					Hud_addMessage(hud, "No effect!");
				}
			}
			else {
				if (combat->curTarget->def->eType == 1) {
					msgBuff = Hud_getMessageBuffer(hud);
					SDL_snprintf(msgBuff, MS_PER_CHAR, "%s%d damage!", combat->gotCrit ? "Crit! " : "", combat->totalDamage + combat->totalArmorDamage);

					Entity_pain(combat->curTarget, combat->totalDamage, combat->totalArmorDamage);

					if (CombatEntity_getHealth(&combat->curTarget->monster->ce) <= 0) {
						msgLen = SDL_strlen(msgBuff);
						SDL_snprintf(msgBuff + msgLen, MS_PER_CHAR - msgLen, " %s died!", combat->curTarget->def->name);
						Hud_finishMessageBuffer(hud);
						Entity_died(combat->curTarget);
					}
					else {
						Hud_finishMessageBuffer(hud);

						// Pain Sound 
						snd = EntityMonster_getSoundRnd(combat->curTarget->monster, 6);
						if (snd != 0) {
							Sound_playSound(combat->doomRpg->sound, snd, 0, 2);
						}
					}
				}
				else if (combat->hitType != 0) {
					if (combat->curTarget->def->eSubType == 4) {
						msgBuff = Hud_getMessageBuffer(hud);
						int n3 = ((combat->curTarget->info & 0x400000) != 0x0) ? 1 : 0;

						if (combat->doomRpg->game->powerCouplingHealth[n3] > combat->totalDamage + combat->totalArmorDamage) {
							SDL_snprintf(msgBuff, MS_PER_CHAR, "Hit %s for %d damage!", combat->curTarget->def->name, combat->totalDamage + combat->totalArmorDamage);
						}
						else {
							SDL_snprintf(msgBuff, MS_PER_CHAR, "%s destroyed!", combat->curTarget->def->name);
						}

						Hud_finishMessageBuffer(hud);

						combat->doomRpg->game->powerCouplingHealth[n3] -= combat->totalDamage + combat->totalArmorDamage;
						if (combat->doomRpg->game->powerCouplingHealth[n3] <= 0) {
							Entity_died(combat->curTarget);
						}
					}
					else {
						Entity_died(combat->curTarget);
					}
				}
				else if (combat->attackerWeaponId == 1) {
					Hud_addMessage(hud, "No effect!");
				}
				else {
					Hud_addMessage(hud, "Missed!");
				}

				if (combat->hitType != 0 && (combat->curTarget->info & 0x200000) == 0x0) {
					sprite = &combat->doomRpg->render->mapSprites[(combat->curTarget->info & 0xFFFF) - 1];
					if (combat->attackerWeaponId == 7) {
						Game_radiusHurtEntities(combat->doomRpg->game, sprite->x, sprite->y, combat->totalDamage / 2, combat->totalArmorDamage / 2, false, combat->curAttacker == NULL);
					}
					else if (combat->attackerWeaponId == 8) {
						Game_radiusHurtEntities(combat->doomRpg->game, sprite->x, sprite->y, combat->totalDamage / 2, combat->totalArmorDamage / 2, true, combat->curAttacker == NULL);
					}
				}
			}
			if (combat->curTarget->def->eType != 1 || CombatEntity_getHealth(&combat->curTarget->monster->ce) <= 0) {
				combat->f339c = doomCanvas->time + 1000;
				return true;
			}
			combat->nextStageTime = doomCanvas->time + 1000;
		}
		else if (doomCanvas->time > combat->nextStageTime) {
			combat->nextStageTime = 0;
			return true;
		}
	}

	return false;
}

void Combat_spawnBloodParticles(Combat_t* combat, int i, int i2, int i3)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;

	if (combat->curTarget) {
		if (combat->curTarget->monster) {
			combat->curTarget->monster->damageBlendTime = combat->doomRpg->doomCanvas->time + 100;
		}
	}

	if (i3 == -1) {
		if ((combat->curTarget == NULL) || (combat->attackerWeaponId != 1)) {
			i3 = Combat_calcParticleIntensity(combat, i);
		}
		else if (combat->curTarget->def->eSubType == 4) {
			Combat_spawnParticlesFire(combat, i, 0xCCCCCC);
			return;
		}
		else {
			return;
		}
	}
	particleSystem->minVelX = -150;
	particleSystem->maxVelX = 100;
	if (combat->f340c) {
		particleSystem->minVelY = -175;
		particleSystem->maxVelY = -75;
	}
	else {
		particleSystem->minVelY = -160;
		particleSystem->maxVelY = -60;
	}

	particleSystem->minStartX = -6;
	particleSystem->maxStartX = 6;
	particleSystem->minStartY = -9;
	particleSystem->maxStartY = 11;
	particleSystem->gravity = 10;
	particleSystem->minSize = 1;
	particleSystem->maxSize = 4;
	particleSystem->color = i2;
	particleSystem->dispX = 0;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->screenRect.w;
	particleSystem->f547c = 0;
	particleSystem->dispHeight = combat->doomRpg->doomCanvas->screenRect.h;
	particleSystem->f543a = 0;

	ParticleSystem_spawnParticles(particleSystem, i3, i, combat->f340c && (combat->attackerWeaponId != 4 || combat->animLoopCount == 1));
}

void Combat_spawnParticlesDestructibleObject(Combat_t* combat, int i)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;
	particleSystem->maxVelX = 75;
	particleSystem->minVelY = -150;
	particleSystem->minVelX = -75;
	particleSystem->minStartX = -6;
	particleSystem->maxStartX = 6;
	particleSystem->minStartY = 9;
	particleSystem->maxStartY = 10;
	particleSystem->maxVelY = -75;
	particleSystem->color = 0xBBBBBB;
	particleSystem->gravity = 12;
	particleSystem->maxSize = 4;
	particleSystem->minSize = 1;
	particleSystem->dispX = 0;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->displayRect.w;
	particleSystem->dispHeight = (combat->doomRpg->doomCanvas->displayRect.h - combat->doomRpg->hud->statusBarHeight);
	particleSystem->f547c = 0;
	particleSystem->f543a = 1;
	ParticleSystem_spawnParticles(particleSystem, 15, i, 0);
}

void Combat_spawnDogBloodParticles(Combat_t* combat)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;
	particleSystem->minVelX = -150;
	particleSystem->maxVelX = 75;
	particleSystem->minVelY = -100;
	particleSystem->maxVelY = -50;
	particleSystem->minStartX = 10;
	particleSystem->maxStartX = 30;
	particleSystem->minStartY = 15;
	particleSystem->maxStartY = 35;
	particleSystem->color = 0x880000;
	particleSystem->gravity = 8;
	particleSystem->minSize = 1;
	particleSystem->maxSize = 4;
	particleSystem->dispX = 0;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->screenRect.w;
	particleSystem->dispHeight = combat->doomRpg->doomCanvas->screenRect.h;
	particleSystem->f547c = 0;
	particleSystem->f543a = 0;
	ParticleSystem_spawnParticles(particleSystem, 15, 1, 0);
}

void Combat_spawnCaptureDogParticles(Combat_t* combat, int color)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;
	particleSystem->maxStartX = 44;
	particleSystem->minStartX = -44;
	particleSystem->color = color;
	particleSystem->maxStartY = 44;
	particleSystem->minSize = 1;
	particleSystem->gravity = 0;
	particleSystem->maxSize = 4;
	particleSystem->minStartY = -44;
	particleSystem->dispX = 0;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->screenRect.w;
	particleSystem->dispHeight = combat->doomRpg->doomCanvas->screenRect.h;
	particleSystem->f547c = 6;
	particleSystem->centreY = 15;
	particleSystem->centreX = 0;
	particleSystem->f543a = 0;
	ParticleSystem_spawnParticles(particleSystem, 45, 1, 0);
}

void Combat_spawnParticlesFire(Combat_t* combat, int i, int color)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;
	particleSystem->minVelX = -85;
	particleSystem->maxVelX = 85;
	particleSystem->minVelY = -160;
	particleSystem->maxVelY = -100;
	particleSystem->minStartX = -16;
	particleSystem->maxStartX = 16;
	particleSystem->minStartY = -9;
	particleSystem->color = color;
	particleSystem->gravity = 1;
	particleSystem->maxStartY = 11;
	particleSystem->minSize = 1;
	particleSystem->maxSize = 4;
	particleSystem->dispX = 0;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->displayRect.w;
	particleSystem->f547c = 0;
	particleSystem->dispHeight = (combat->doomRpg->doomCanvas->displayRect.w - combat->doomRpg->hud->statusBarHeight);
	particleSystem->f543a = 0;
	ParticleSystem_spawnParticles(particleSystem, 15, i, 0);
}

void Combat_spawnParticlesJammedDoor(Combat_t* combat, int color)
{
	ParticleSystem_t* particleSystem;

	particleSystem = combat->doomRpg->particleSystem;
	particleSystem->minVelX = -150;
	particleSystem->maxVelX = 100;
	particleSystem->minVelY = -200;
	particleSystem->maxVelY = -100;
	particleSystem->minStartX = -6;
	particleSystem->maxStartX = 6;
	particleSystem->minStartY = -6;
	particleSystem->color = color;
	particleSystem->gravity = 10;
	particleSystem->minSize = 1;
	particleSystem->maxSize = 4;
	particleSystem->dispX = 0;
	particleSystem->maxStartY = 6;
	particleSystem->dispY = combat->doomRpg->hud->statusTopBarHeight;
	particleSystem->dispWidth = combat->doomRpg->doomCanvas->displayRect.w;
	particleSystem->f547c = 0;
	particleSystem->dispHeight = (combat->doomRpg->doomCanvas->displayRect.h - combat->doomRpg->hud->statusBarHeight);
	particleSystem->f543a = 0;
	ParticleSystem_spawnParticles(particleSystem, 15, 1, 0);
}

boolean Combat_runFrame(Combat_t* combat)
{
	return !(combat->curAttacker == NULL ? Combat_playerSeq(combat) : Combat_monsterSeq(combat));
}

void Combat_updateProjectile(Combat_t* combat)
{
	GameSpriteMissile_t* gSpriteMissile;
	GameSprite_t* gSprite;
	Sprite_t* sprite;
	int info, i, j, x, y, posX, posY, speed, spX, spY;
	byte renderMode;

	if (combat->numActiveMissiles > 0) {

		for (i = 0; i < combat->numActiveMissiles; ++i) {
			gSpriteMissile = &combat->activeMissiles[i];
			sprite = gSpriteMissile->gsMissile->sprite;
			info = sprite->info & 0x1fff;
			//printf("info %d\n", info);

			switch (info)
			{
			case 201: // Missile Rocket
				speed = 8;
				break;
			case 202: // Missile BFG
				speed = 4;
				break;
			case 203:
				speed = 8;
				break;
			default: // Missile Plasma
				speed = 16; 
				break;
			}

			spX = speed;
			spY = speed;

			if (gSpriteMissile->unk0) {
				spX >>= 2;
			}
			else {
				spY >>= 2;
			}

			x = sprite->x;
			posX = gSpriteMissile->posX;

			if (x < posX) {
				sprite->x = x + spX;
				if (sprite->x > posX) {
					sprite->x = posX;
				}
			}
			else if (x > posX) {
				sprite->x = x - spX;
				if (sprite->x < posX) {
					sprite->x = posX;
				}
			}

			y = sprite->y;
			posY = gSpriteMissile->posY;

			if (y < posY) {
				sprite->y = y + spY;
				if (posY < sprite->y) {
					sprite->y = posY;
				}
			}
			else if (posY < y) {
				sprite->y = y - spY;
				if (sprite->y < posY) {
					sprite->y = posY;
				}
			}

			if ((sprite->x == posX) && (sprite->y == posY))
			{
				renderMode = 0;
				switch (combat->attackerWeaponId) {
					case 6:
						renderMode = 7;
						combat->missileAnim = 5;
						break;
					case 7:
						combat->missileAnim = 1;
						break;
					case 8:
						renderMode = 7;
						combat->missileAnim = 4;
						break;
					case 12:
						if (combat->curAttacker->def->eSubType == 8) {
							combat->missileAnim = 12;
						}
						else {
							combat->missileAnim = 10;
						}
						break;
					case 13:
						combat->missileAnim = 11;
						break;
					case 14:
						combat->missileAnim = 12;
						break;
					case 15:
						renderMode = 7;
						if ((combat->curAttacker == NULL) || (combat->curAttacker->def->eSubType != 10)) {
							combat->missileAnim = 6;
						}
						else {
							combat->missileAnim = 9;
						}
						break;
					case 16:
						combat->missileAnim = 7;
						break;
					case 17:
						renderMode = 7;
						combat->missileAnim = 8;
						break;
					default:
						combat->missileAnim = 0;
						break;
				}

				gSpriteMissile->gsMissile->flags = 0;
				sprite->info |= 0x10000;

				if (combat->totalDamage + combat->totalArmorDamage != 0) {
					gSprite = Game_gsprite_allocAnim(combat->doomRpg->game, combat->missileAnim, sprite->x, sprite->y);
					gSprite->flags |= 4;
					gSprite->sprite->renderMode = renderMode;
				}

				for (j = i + 1; j < combat->numActiveMissiles; ++j) {
					combat->activeMissiles[j - 1].gsMissile = combat->activeMissiles[j].gsMissile;
					combat->activeMissiles[j - 1].posX = combat->activeMissiles[j].posX;
					combat->activeMissiles[j - 1].posY = combat->activeMissiles[j].posY;
					combat->activeMissiles[j - 1].unk0 = combat->activeMissiles[j].unk0;
				}

				--combat->numActiveMissiles;
				combat->exploded = true;
			}
			else {
				Render_relinkSprite(combat->doomRpg->render, sprite);
			}
		}
	}

	if(combat->exploded) {
		if (combat->curAttacker == NULL) {
			Combat_explodeOnMonster(combat);
		}
		else {
			Combat_explodeOnPlayer(combat);
		}
		combat->f342d = 1;
		combat->exploded = 0;
	}

	if (combat->numActiveMissiles == 0) {
		if (combat->hitType == 0) {
			combat->f342d = 1;
		}
		else if (combat->f342d == 0) {
			return;
		}

		if (combat->animLoopCount == 0 && !combat->f336b) {
			combat->nextStageTime = combat->doomRpg->doomCanvas->time;
		}
	}
}