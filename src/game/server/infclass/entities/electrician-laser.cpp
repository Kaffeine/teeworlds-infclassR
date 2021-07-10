#include "electrician-laser.h"

#include <engine/shared/config.h>

#include <game/server/infclass/entities/infccharacter.h>
#include <game/server/infclass/entities/voltage-box.h>
#include <game/server/infclass/infcgamecontroller.h>

CElectricianLaser::CElectricianLaser(CGameContext *pGameContext, vec2 Pos, vec2 Direction, float StartEnergy, int Owner)
	: CInfClassLaser(pGameContext, Pos, Direction, StartEnergy, Owner, 0, CGameWorld::ENTTYPE_LASER)
{
	GameWorld()->InsertEntity(this);
	DoBounce();
}

void CElectricianLaser::Tick()
{
	if(m_Bounces > 1)
	{
		Reset();
		return;
	}

	CInfClassCharacter *pOwnerChar = GameController()->GetCharacter(GetOwner());

	CVoltageBox *pBox = pOwnerChar->GetVoltageBox();
	if(!pBox || distance(GetPos(), pBox->GetPos()) >= Config()->m_InfVoltageBoxLinkLength)
	{
		Reset();
		return;
	}

	CInfClassLaser::Tick();
}

bool CElectricianLaser::HitCharacter(vec2 From, vec2 To)
{
	vec2 At;
	CInfClassCharacter *pOwnerChar = GameController()->GetCharacter(GetOwner());
	CInfClassCharacter *pHit = static_cast<CInfClassCharacter*>(GameWorld()->IntersectCharacter(m_Pos, To, 0.f, At, pOwnerChar));

	if(!pHit)
	{
		return false;
	}

	if(pHit->IsHuman())
	{
		// TODO: Check this
		return false;
	}

	m_From = From;
	m_Pos = At;
	m_Energy = -1;

	CVoltageBox *pBox = pOwnerChar->GetVoltageBox();
	if(!pBox)
	{
		Reset();
		return true;
	}

	pBox->AddLink(pHit->GetCID());

	return true;
}
