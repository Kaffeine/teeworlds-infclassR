#include "human.h"

#include <engine/shared/config.h>
#include <game/server/classes.h>
#include <game/server/gamecontext.h>
#include <game/server/infclass/entities/infccharacter.h>
#include <game/server/infclass/entities/voltage-box.h>
#include <game/server/infclass/infcgamecontroller.h>
#include <game/server/infclass/infcplayer.h>
#include <game/server/teeinfo.h>

MACRO_ALLOC_POOL_ID_IMPL(CInfClassHuman, MAX_CLIENTS)

CInfClassHuman::CInfClassHuman(CInfClassPlayer *pPlayer)
	: CInfClassPlayerClass(pPlayer)
{
}

void CInfClassHuman::OnCharacterPreCoreTick()
{
	if(PlayerClass() == PLAYERCLASS_SNIPER && m_pCharacter->m_PositionLocked)
	{
		if(m_pCharacter->m_Input.m_Jump && !m_pCharacter->m_PrevInput.m_Jump)
		{
			m_pCharacter->UnlockPosition();
		}
	}
}

void CInfClassHuman::OnCharacterTick()
{
	if(PlayerClass() == PLAYERCLASS_ELECTRICIAN)
	{
		if(m_pCharacter->m_ActiveWeapon == WEAPON_LASER)
		{
			CVoltageBox *pBox = m_pCharacter->GetVoltageBox();
			if(!pBox)
			{
				GameServer()->SendBroadcast_Localization(GetPlayer()->GetCID(), BROADCAST_PRIORITY_WEAPONSTATE, 60, "No VoltageBox to make a link");
			}
		}
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
		case PLAYERCLASS_ELECTRICIAN:
			m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
			m_pCharacter->GiveWeapon(WEAPON_GUN, -1);
			m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
			m_pCharacter->SetActiveWeapon(WEAPON_HAMMER);
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
		case PLAYERCLASS_ELECTRICIAN:
			output->SetSkinName("cammo");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 8716159;
			output->m_ColorFeet = 0;
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

void CInfClassHuman::RegisterHelpPages(int PlayerClass)
{
	const char *pClassName = CInfClassGameController::GetClassName(PLAYERCLASS_ELECTRICIAN);
	// AddPage(pClassName, CinfClassHuman::ElectricianHelpPage)
	// page:
	// electrician
	// callback:
	// CinfClassHuman::ElectricianHelpPage
}

void CInfClassHuman::SetupSkin(CTeeInfo *output)
{
	SetupSkin(PlayerClass(), output);
}

void CInfClassHuman::DestroyChildEntities()
{
	if(PlayerClass() == PLAYERCLASS_ELECTRICIAN)
	{
		CVoltageBox *pBox = m_pCharacter->GetVoltageBox();
		if(pBox)
		{
			pBox->Reset();
		}
	}
}

void CInfClassHuman::OnSlimeEffect(int Owner)
{
	int Count = Config()->m_InfSlimePoisonDuration;
	Poison(Count, Owner);
}
