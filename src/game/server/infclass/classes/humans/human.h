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

	void HandleBonusZone(int ZoneValue) override;
	void HandleDamageZone(int ZoneValue) override;

	void OnCharacterTick() override;
	void OnCharacterSpawned() override;
	void OnCharacterDeath(int Killer, int Weapon) override;
	void OnThisKilledAnotherCharacter() override;

	void OnSlimeEffect(int Owner) override;

	static bool SetupSkin(int PlayerClass, CTeeInfo *output);

protected:
	void CheckSuperWeaponAccess();
	void MaybeGiveStunGrenades();

	void GiveClassAttributes() override;
	void SetupSkin(CTeeInfo *output) override;

	int m_BonusTick = 0;
};

#endif // GAME_SERVER_INFCLASS_CLASSES_HUMAN_H
