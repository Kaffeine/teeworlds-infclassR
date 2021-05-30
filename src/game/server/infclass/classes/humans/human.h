#ifndef GAME_SERVER_INFCLASS_CLASSES_HUMAN_H
#define GAME_SERVER_INFCLASS_CLASSES_HUMAN_H

#include "../infcplayerclass.h"

#include <game/server/alloc.h>

class CInfClassHuman : public CInfClassPlayerClass
{
	MACRO_ALLOC_POOL_ID()

public:
	explicit CInfClassHuman(CInfClassPlayer *pPlayer);

	bool IsHuman() const final { return true; }

	void OnCharacterTick() override;
	void OnCharacterDeath(int Killer, int Weapon) override;
	void OnThisKilledAnotherCharacter() override;

	void OnSlimeEffect(int Owner) override;

	static bool SetupSkin(int PlayerClass, CTeeInfo *output);

protected:
	void CheckSuperWeaponAccess();
	void MaybeGiveStunGrenades();

	void GiveClassAttributes() override;
	void SetupSkin(CTeeInfo *output) override;
};

#endif // GAME_SERVER_INFCLASS_CLASSES_HUMAN_H
