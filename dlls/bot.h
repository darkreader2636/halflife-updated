// bot.h==========================================================
// botmans PGBot (http://planethalflife.com/botman)===============
// Bot Tutorial by BigGuy=========================================

#ifndef BOT_H
#define BOT_H

#define LADDER_UP 1
#define LADDER_DOWN 2

#define WANDER_LEFT 1
#define WANDER_RIGHT 2

void BotDebug(char* msg);
// always have some type of bug reporter

class CBot : public CBasePlayer
// player inherits many functions from player class
{
public:
	char message[256];
	// this is for error messages
	void EXPORT Spawn(void);
	// called when bot begins (spawns)
	void EXPORT Killed(entvars_t* pevAttacker, int iGib);
	// called when bot is killed
	void EXPORT BotThink(void);
	// bots need to think!
	void EXPORT PlayerDeathThink(void);
	// even when theyre dead!
	virtual bool IsNetClient(void) { return false; }
	// bots cant recieve NET messages
	int BloodColor() { return BLOOD_COLOR_RED; }
	// bots bleed too
	void BotOnLadder(void);	  // This handles the bot going up or down.
	void BotFindItem(void);	  // function that bots use to find items.
	Vector v_prev_origin;	  // previous location
	float f_max_speed;		  // last sv_maxspeed setting
	float f_speed_check_time; // check sv_maxspeed every so often
	float f_move_speed;		  // speed at which the bot will move
	int ladder_dir;			  // direction travelling on ladder (up or down)
	int wander_dir;			  // randomly wander left or right
	float f_pause_time;		  // timeout for periods when the bot pause
	float f_find_item;		  // timeout for not looking for items
};
extern int f_Observer;
void BotCreate(void);
#endif // BOT_H