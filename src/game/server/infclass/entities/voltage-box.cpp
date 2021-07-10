/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "voltage-box.h"

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/infclass/infcgamecontroller.h>

#include "infccharacter.h"

int CVoltageBox::EntityId = CGameWorld::ENTTYPE_VOLTAGE_BOX;
static constexpr int BoxProximityRadius = 24;
static constexpr int BasicDamage = 10;

static constexpr int InvalidClientID = -1;

CVoltageBox::CVoltageBox(CGameContext *pGameContext, vec2 CenterPos, int Owner)
	: CInfCEntity(pGameContext, EntityId, CenterPos, Owner, BoxProximityRadius)
{
	GameWorld()->InsertEntity(this);

	m_Charges = Config()->m_InfVoltageBoxCharges;
	for(int &LinkClientID : m_LinkedClientIDs)
	{
		LinkClientID = InvalidClientID;
	}
	for(LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		SnapItem.SnapID = Server()->SnapNewID();
	}

	m_DischargeTick = -1;

	AddLink(Owner);
}

CVoltageBox::~CVoltageBox()
{
	for(const LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		Server()->SnapFreeID(SnapItem.SnapID);
	}
}

void CVoltageBox::AddLink(int ClientID)
{
	for(int LinkedClient : m_LinkedClientIDs)
	{
		if(LinkedClient == InvalidClientID)
		{
			break;
		}
		if(LinkedClient == ClientID)
		{
			// Already linked
			return;
		}
	}

	if (m_LinksCount + 1 >= MaxLinks)
	{
		// TODO: Warning
		return;
	}
	m_LinkedClientIDs[m_LinksCount] = ClientID;
	m_LinksCount++;
}

void CVoltageBox::RemoveLink(int ClientID)
{
	for(int i = 0; i < m_LinksCount; ++i)
	{
		if(m_LinkedClientIDs[i] == ClientID)
		{
			if(i + 1 < m_LinksCount)
			{
				// This is not the last linked client. Swap.
				m_LinkedClientIDs[i] = m_LinkedClientIDs[m_LinksCount - 1];
			}

			m_LinkedClientIDs[m_LinksCount - 1] = InvalidClientID;
			--m_LinksCount;
		}
	}
}

void CVoltageBox::Discharge()
{
	m_Charges--;
	m_DischargeTick = Server()->Tick();

	GameServer()->CreateSound(m_Pos, SOUND_LASER_FIRE);

	// 1. Freeze the chars
	// 2. move bold lasers along the links
	// 3. Deal the damage
	// 4. Kill the links endpoints (except the owner)
}

void CVoltageBox::FinalDischarge()
{
	Discharge();
	m_Charges = 0;
}

void CVoltageBox::Tick()
{
	if(m_DischargeTick >= 0)
	{
		UpdateDischarge();
	}
	else if(m_Charges <= 0)
	{
		Reset();
	}
	else
	{
		UpdateLinks();
	}

	BakeSnapItems();
}

void CVoltageBox::TickPaused()
{
	if(m_DischargeTick)
	{
		++m_DischargeTick;
	}
}

void CVoltageBox::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	for(int i=0; i < m_ActiveSnapItems; i++)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
			NETOBJTYPE_LASER, m_LasersForSnap[i].SnapID, sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_X = m_LasersForSnap[i].To.x;
		pObj->m_Y = m_LasersForSnap[i].To.y;
		pObj->m_FromX = m_LasersForSnap[i].From.x;
		pObj->m_FromY = m_LasersForSnap[i].From.y;
		pObj->m_StartTick = m_LasersForSnap[i].StartTick;
	}
}

void CVoltageBox::BakeSnapItems()
{
	int CurrentSnapItemN = 0;

	static const vec2 Vertices[BoxEdges] =
	{
		vec2(-BoxProximityRadius, -BoxProximityRadius),
		vec2( BoxProximityRadius, -BoxProximityRadius),
		vec2( BoxProximityRadius,  BoxProximityRadius),
		vec2(-BoxProximityRadius,  BoxProximityRadius),
	};

	for(int i=0; i<BoxEdges; i++)
	{
		const bool Last = i == BoxEdges - 1;
		vec2 PartPosStart = Last ? m_Pos + Vertices[0] : m_Pos + Vertices[i + 1];
		vec2 PartPosEnd = m_Pos + Vertices[i];

		m_LasersForSnap[CurrentSnapItemN].From.x = PartPosStart.x;
		m_LasersForSnap[CurrentSnapItemN].From.y = PartPosStart.y;
		m_LasersForSnap[CurrentSnapItemN].To.x = PartPosEnd.x;
		m_LasersForSnap[CurrentSnapItemN].To.y = PartPosEnd.y;
		m_LasersForSnap[CurrentSnapItemN].StartTick = Server()->Tick() - 4;
		++CurrentSnapItemN;
	}

	m_LasersForSnap[CurrentSnapItemN].From.x = m_Pos.x;
	m_LasersForSnap[CurrentSnapItemN].From.y = m_Pos.y;
	m_LasersForSnap[CurrentSnapItemN].To.x = m_Pos.x;
	m_LasersForSnap[CurrentSnapItemN].To.y = m_Pos.y;
	m_LasersForSnap[CurrentSnapItemN].StartTick = Server()->Tick();
	++CurrentSnapItemN;

	// Snap links
	for(int i = 0; i < m_LinksCount; ++i)
	{
		int ClientID = m_LinkedClientIDs[i];

		CCharacter *pCharacter = GameController()->GetCharacter(ClientID);
		if(!pCharacter)
		{
			continue;
		}
		const vec2 CharPos = pCharacter->GetPos();

		float MaxLength = Config()->m_InfVoltageBoxLinkLength;
		float Distance = distance(CharPos, GetPos());
		if (Distance > MaxLength)
		{
			Distance = MaxLength;
		}
		m_LasersForSnap[CurrentSnapItemN].From.x = m_Pos.x;
		m_LasersForSnap[CurrentSnapItemN].From.y = m_Pos.y;
		m_LasersForSnap[CurrentSnapItemN].To.x = CharPos.x;
		m_LasersForSnap[CurrentSnapItemN].To.y = CharPos.y;
		m_LasersForSnap[CurrentSnapItemN].StartTick = GetStartTickForDistance(Distance / MaxLength);
		++CurrentSnapItemN;
	}

	m_ActiveSnapItems = CurrentSnapItemN;
}

void CVoltageBox::UpdateLinks()
{
	for(int i = 0; i < m_LinksCount; ++i)
	{
		int ClientID = m_LinkedClientIDs[i];

		CCharacter *pCharacter = GameController()->GetCharacter(ClientID);
		if(!pCharacter || !pCharacter->IsAlive())
		{
			RemoveLink(ClientID);
			continue;
		}
		const vec2 CharPos = pCharacter->GetPos();
		float Distance = distance(CharPos, GetPos());
		float MaxLength = Config()->m_InfVoltageBoxLinkLength;

		if(Distance > MaxLength)
		{
			if(ClientID == GetOwner())
			{
				FinalDischarge();
			}
			else
			{
				Discharge();
			}
			return;
		}
	}
}

void CVoltageBox::UpdateDischarge()
{
	// 1. Freeze the chars on links
	// 2. move bold lasers along the links
	// 3. Deal the damage
	// 4. Kill the links endpoints (except the owner)

	int CurrentTick = Server()->Tick();
	if(CurrentTick >= m_DischargeTick + Server()->TickSpeed() * Config()->m_InfVoltageBoxDischargeDuration / 1000)
	{
		// Discharge is over.
		for(int i = 0; i < m_LinksCount; ++i)
		{
			int ClientID = m_LinkedClientIDs[i];
			CCharacter *pCharacter = GameController()->GetCharacter(ClientID);
			if(!pCharacter || pCharacter->IsHuman())
			{
				continue;
			}
			// static const vec2 Force(0, 0);
			// pCharacter->TakeDamage(Force, BasicDamage, GetOwner(), WEAPON_LASER, TAKEDAMAGEMODE_NOINFECTION);
			pCharacter->Die(GetOwner(), WEAPON_LASER);
		}

		m_DischargeTick = -1;
	}
}

int CVoltageBox::GetStartTickForDistance(float Progress)
{
	int Tick = Server()->Tick();
	int Correction = 0;

	Correction = 0;
	if(Progress > 0.2)
	{
		Correction = 1;
	}
	if(Progress > 0.4)
	{
		Correction = 2;
	}
	if(Progress > 0.6)
	{
		Correction = 3;
	}
	if(Progress > 0.8)
	{
		Correction = 4;
	}
	if(Progress > 0.9)
	{
		Correction = 5;
	}

	return Tick - Correction;
}
