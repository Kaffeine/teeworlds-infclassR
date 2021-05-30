#include "human.h"

#include <engine/shared/config.h>
#include <game/server/classes.h>
#include <game/server/gamecontext.h>
#include <game/server/infclass/entities/infccharacter.h>
#include <game/server/infclass/entities/superweapon-indicator.h>
#include <game/server/infclass/infcgamecontroller.h>
#include <game/server/infclass/infcplayer.h>
#include <game/server/teeinfo.h>

MACRO_ALLOC_POOL_ID_IMPL(CInfClassHuman, MAX_CLIENTS)

CInfClassHuman::CInfClassHuman(CInfClassPlayer *pPlayer)
	: CInfClassPlayerClass(pPlayer)
{
}

void CInfClassHuman::OnCharacterTick()
{
	if(PlayerClass() == PLAYERCLASS_SNIPER && m_pCharacter->m_PositionLocked)
	{
		if(m_pCharacter->m_Input.m_Jump && !m_pCharacter->m_PrevInput.m_Jump)
		{
			m_pCharacter->m_PositionLocked = false;
		}
	}
}

void CInfClassHuman::OnCharacterDeath(int Killer, int Weapon)
{
	CInfClassPlayerClass::OnCharacterDeath(Killer, Weapon);

	CInfClassPlayer* pKillerPlayer = GameController()->GetPlayer(Killer);
	m_pPlayer->Infect(pKillerPlayer);
}

void CInfClassHuman::OnThisKilledAnotherCharacter()
{
	// set attacker's face to happy (taunt!)
	m_pCharacter->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
	CheckSuperWeaponAccess();

	if(m_pCharacter->GetPlayerClass() == PLAYERCLASS_MERCENARY)
	{
		m_pCharacter->GiveWeapon(WEAPON_LASER, m_pCharacter->m_aWeapons[WEAPON_LASER].m_Ammo + 3);
	}
}

void CInfClassHuman::GiveClassAttributes()
{
	CInfClassPlayerClass::GiveClassAttributes();

	switch(PlayerClass())
	{
		case PLAYERCLASS_ENGINEER:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_LASER);
			break;
		case PLAYERCLASS_SOLDIER:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_GRENADE);
			break;
		case PLAYERCLASS_MERCENARY:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			if(!GameServer()->m_FunRound)
				m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_GUN);
			break;
		case PLAYERCLASS_SNIPER:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_LASER);
			break;
		case PLAYERCLASS_SCIENTIST:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_LASER);
			break;
		case PLAYERCLASS_BIOLOGIST:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->GiveWeapon(WEAPON_SHOTGUN, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_SHOTGUN);
			break;
		case PLAYERCLASS_LOOPER:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_LASER);
			break;
		case PLAYERCLASS_MEDIC:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_SHOTGUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_SHOTGUN);
			break;
		case PLAYERCLASS_HERO:
			if(GameController()->AreTurretsEnabled())
				m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_SHOTGUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_GRENADE);
			break;
		case PLAYERCLASS_NINJA:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_GRENADE, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_HAMMER);
			break;
		case PLAYERCLASS_NONE:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_HAMMER);
			break;
	}
}

bool CInfClassHuman::SetupSkin(int PlayerClass, CTeeInfo *output)
{
	switch(PlayerClass)
	{
		case PLAYERCLASS_ENGINEER:
			output->m_UseCustomColor = 0;
			output->SetSkinName("limekitty");
			break;
		case PLAYERCLASS_SOLDIER:
			output->SetSkinName("brownbear");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_SNIPER:
			output->SetSkinName("warpaint");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_MERCENARY:
			output->SetSkinName("bluestripe");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_SCIENTIST:
			output->SetSkinName("toptri");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_BIOLOGIST:
			output->SetSkinName("twintri");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_LOOPER:
			output->SetSkinName("bluekitty");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 255;
			output->m_ColorFeet = 0;
			break;
		case PLAYERCLASS_MEDIC:
			output->SetSkinName("twinbop");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_HERO:
			output->SetSkinName("redstripe");
			output->m_UseCustomColor = 0;
			break;
		case PLAYERCLASS_NINJA:
			output->SetSkinName("default");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 255;
			output->m_ColorFeet = 0;
			break;
		default:
			output->SetSkinName("default");
			output->m_UseCustomColor = 0;
			return false;
	}

	return true;
}

void CInfClassHuman::CheckSuperWeaponAccess()
{
	// check kills of player
	int kills = m_pPlayer->GetNumberKills();

	//Only scientists can receive white holes
	if(PlayerClass() == PLAYERCLASS_SCIENTIST)
	{
		if (!m_pCharacter->m_HasWhiteHole) // Can't receive a white hole while having one available
		{
			// enable white hole probabilities
			if (kills > Config()->m_InfWhiteHoleMinimalKills)
			{
				if (random_int(0,100) < Config()->m_InfWhiteHoleProbability)
				{
					//Scientist-laser.cpp will make it unavailable after usage and reset player kills

					//create an indicator object
					if (m_pCharacter->m_HasIndicator == false) {
						m_pCharacter->m_HasIndicator = true;
						GameServer()->SendChatTarget_Localization(m_pPlayer->GetCID(), CHATCATEGORY_SCORE, _("white hole found, adjusting scientific parameters..."), NULL);
						new CSuperWeaponIndicator(GameServer(), GetPos(), m_pPlayer->GetCID());
					}
				}
			}
		}
	}

	if(PlayerClass() == PLAYERCLASS_LOOPER)
	{
		MaybeGiveStunGrenades();
	}

	if(PlayerClass() == PLAYERCLASS_SOLDIER)
	{
		MaybeGiveStunGrenades();
	}
}

void CInfClassHuman::MaybeGiveStunGrenades()
{
	if(m_pCharacter->m_HasStunGrenade)
		return;

	if(m_pPlayer->GetNumberKills() > Config()->m_InfStunGrenadeMinimalKills)
	{
		if(random_int(0,100) < Config()->m_InfStunGrenadeProbability)
		{
				//grenade launcher usage will make it unavailable and reset player kills

				m_pCharacter->m_HasStunGrenade = true;
				GameServer()->SendChatTarget_Localization(m_pPlayer->GetCID(), CHATCATEGORY_SCORE, _("stun grenades found..."), NULL);
		}
	}
}

void CInfClassHuman::SetupSkin(CTeeInfo *output)
{
	SetupSkin(PlayerClass(), output);
}

void CInfClassHuman::OnSlimeEffect(int Owner)
{
	int Count = Config()->m_InfSlimePoisonDuration;
	Poison(Count, Owner);
}
