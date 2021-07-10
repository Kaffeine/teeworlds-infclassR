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
static const vec2 DamageForce(0, 0);

CVoltageBox::CVoltageBox(CGameContext *pGameContext, vec2 CenterPos, int Owner)
	: CInfCEntity(pGameContext, EntityId, CenterPos, Owner, BoxProximityRadius)
{
	GameWorld()->InsertEntity(this);

	m_Charges = Config()->m_InfVoltageBoxCharges;

	for(LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		SnapItem.SnapID = Server()->SnapNewID();
	}

	for(int &IndicatorID : m_OwnerRadiusIndicatorsIDs)
	{
		IndicatorID = Server()->SnapNewID();
	}

	ClearLinks();
	AddLink(Owner);
}

CVoltageBox::~CVoltageBox()
{
	for(const LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		Server()->SnapFreeID(SnapItem.SnapID);
	}

	for(int IndicatorID : m_OwnerRadiusIndicatorsIDs)
	{
		Server()->SnapFreeID(IndicatorID);
	}
}

void CVoltageBox::AddLink(int ClientID)
{
	const CEntity *pCharacter = GameController()->GetCharacter(ClientID);
	if(!pCharacter)
	{
		// TODO: Warning Invalid ClientID
		return;
	}

	for(int i = 0; i < m_LinksCount; ++i)
	{
		if(m_Links[i].ClientID == ClientID)
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

	m_Links[m_LinksCount].ClientID = ClientID;
	m_Links[m_LinksCount].Endpoint = pCharacter->GetPos();
	m_LinksCount++;
}

void CVoltageBox::RemoveLink(int ClientID)
{
	for(int i = 0; i < m_LinksCount; ++i)
	{
		if(m_Links[i].ClientID != ClientID)
			continue;

		if(i + 1 < m_LinksCount)
		{
			// This is not the last linked client. Swap.
			m_Links[i] = m_Links[m_LinksCount - 1];
		}

		m_Links[m_LinksCount - 1].ClientID = InvalidClientID;
		--m_LinksCount;
	}
}

void CVoltageBox::ClearLinks()
{
	for(Link &Link : m_Links)
	{
		Link.ClientID = InvalidClientID;
	}
	m_LinksCount = 0;
}

void CVoltageBox::ScheduleDischarge(DISCHARGE_TYPE Type)
{
	// The same type, do nothing
	if(m_ScheduledDischarge == Type)
		return;

	// If the final was scheduled then it's too late to do anything
	if(m_ScheduledDischarge == DISCHARGE_TYPE_FINAL)
		return;

	// If we need to set the final then set the final
	if(Type == DISCHARGE_TYPE_FINAL)
	{
		m_ScheduledDischarge = Type;
		return;
	}

	// We have a free discharge. Ignore NORMAL or further FREE discharging calls.
	if(m_ScheduledDischarge == DISCHARGE_TYPE_FREE)
		return;

	// If was NORMAL then it's OK to set it to NORMAL or FREE.
	m_ScheduledDischarge = Type;
}

void CVoltageBox::Tick()
{
	if(m_Charges <= 0)
	{
		Reset();
	}

	UpdateLinks();
	PrepareSnapItems();

	if(m_ScheduledDischarge != DISCHARGE_TYPE_INVALID)
	{
		DoDischarge();
	}
}

void CVoltageBox::TickPaused()
{
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

	if(SnappingClient == GetOwner())
	{
		SnapDrawRadiusIndicator();
	}
}

void CVoltageBox::SnapDrawRadiusIndicator()
{
	float Radius = Config()->m_InfVoltageBoxLinkLength;
	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/20.0f;
	AngleStart = AngleStart*2.0f;
	static const float AngleStep = 2.0f * pi / IndicatorItems;

	for(int i = 0; i < IndicatorItems; i++)
	{
		float Angle = AngleStart + AngleStep * i;
		vec2 PosStart = GetPos() + vec2(Radius * cos(Angle), Radius * sin(Angle));

		int ItemID = m_OwnerRadiusIndicatorsIDs[i];
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, ItemID, sizeof(CNetObj_Projectile)));
		if(pObj)
		{
			pObj->m_X = PosStart.x;
			pObj->m_Y = PosStart.y;
			pObj->m_VelX = 0;
			pObj->m_VelY = 0;
			pObj->m_StartTick = Server()->Tick();
			pObj->m_Type = WEAPON_HAMMER;
		}
	}
}

void CVoltageBox::AddSnapItem(const vec2 &From, const vec2 &To, int SnapTick)
{
	m_LasersForSnap[m_ActiveSnapItems].From.x = From.x;
	m_LasersForSnap[m_ActiveSnapItems].From.y = From.y;
	m_LasersForSnap[m_ActiveSnapItems].To.x = To.x;
	m_LasersForSnap[m_ActiveSnapItems].To.y = To.y;
	m_LasersForSnap[m_ActiveSnapItems].StartTick = SnapTick;
	++m_ActiveSnapItems;
}

void CVoltageBox::PrepareSnapItems()
{
	m_ActiveSnapItems = 0;
	PrepareTheBoxSnapItems();
	PrepareTheLinksSnapItems();
}

void CVoltageBox::PrepareTheBoxSnapItems()
{
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

		AddSnapItem(PartPosStart, PartPosEnd, Server()->Tick() - 4);
	}

	AddSnapItem(m_Pos, m_Pos, Server()->Tick());
}

void CVoltageBox::PrepareTheLinksSnapItems()
{
	const float MaxLength = Config()->m_InfVoltageBoxLinkLength;

	static const float DischargeHighlightLength = 32;

	// Snap links
	for(int i = 0; i < m_LinksCount; ++i)
	{
		const vec2 &Endpoint = m_Links[i].Endpoint;

		float Distance = distance(Endpoint, GetPos());
		if (Distance > MaxLength)
		{
			Distance = MaxLength;
		}

		AddSnapItem(GetPos(), Endpoint, GetStartTickForDistance(0.3 + Distance / MaxLength));
	}
}

void CVoltageBox::UpdateLinks()
{
	const float MaxLength = Config()->m_InfVoltageBoxLinkLength;

	for(int i = 0; i < m_LinksCount; ++i)
	{
		int ClientID = m_Links[i].ClientID;

		CInfClassCharacter *pCharacter = GameController()->GetCharacter(ClientID);
		if(!pCharacter)
		{
			ScheduleDischarge(DISCHARGE_TYPE_FREE);
			continue;
		}

		if(!pCharacter->IsAlive())
		{
			// This will be a free discharge (because the owner left the area)
			ScheduleDischarge(DISCHARGE_TYPE_FREE);
			// Continue to update the other links position
			continue;
		}

		vec2 NewPos = pCharacter->GetPos();
		float Distance = distance(m_Links[i].Endpoint, GetPos());

		if(Distance > MaxLength)
		{
			if(ClientID == GetOwner())
			{
				ScheduleDischarge(DISCHARGE_TYPE_FINAL);
			}
			else
			{
				ScheduleDischarge(DISCHARGE_TYPE_NORMAL);
			}

			// Continue to update the other links position
			continue;
		}

		m_Links[i].Endpoint = NewPos;
	}

	// Reveal the ghosts
	for(CInfClassCharacter *p = (CInfClassCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CInfClassCharacter *)p->TypeNext())
	{
		if(p->GetPlayerClass() == PLAYERCLASS_GHOST)
		{
			for(int i = 0; i < m_LinksCount; ++i)
			{
				const vec2 IntersectPos = closest_point_on_line(GetPos(), m_Links[i].Endpoint, p->GetPos());
				float Len = distance(p->GetPos(), IntersectPos);
				if(Len < p->GetProximityRadius())
				{
					p->MakeVisible();
				}
			}
		}
	}
}

void CVoltageBox::DoDischarge()
{
	// Find other players on the links
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		CInfClassCharacter *p = GameController()->GetCharacter(i);
		if(!p || !p->IsAlive())
			continue;

		if(p->IsHuman())
			continue;

		for(int i = 0; i < m_LinksCount; ++i)
		{
			const vec2 &LinkEndpoint = m_Links[i].Endpoint;
			const vec2 IntersectPos = closest_point_on_line(GetPos(), LinkEndpoint, p->GetPos());
			float Len = distance(p->GetPos(), IntersectPos);
			if(Len < p->GetProximityRadius())
			{
				p->TakeDamage(DamageForce, BasicDamage, GetOwner(), WEAPON_LASER, TAKEDAMAGEMODE_NOINFECTION);
			}
		}
	}

	// Kill the linked zombies
	for(int i = 0; i < m_LinksCount; ++i)
	{
		int ClientID = m_Links[i].ClientID;
		CInfClassCharacter *pCharacter = GameController()->GetCharacter(ClientID);
		if(!pCharacter)
		{
			GameServer()->CreateSound(m_Links[i].Endpoint, SOUND_LASER_BOUNCE);
			continue;
		}

		GameServer()->CreateSound(pCharacter->GetPos(), SOUND_LASER_FIRE);

		if(!pCharacter->IsHuman())
		{
			pCharacter->Die(GetOwner(), WEAPON_LASER);
		}
	}

	switch(m_ScheduledDischarge)
	{
		case DISCHARGE_TYPE_INVALID:
			// TODO: Warning
			break;
		case DISCHARGE_TYPE_NORMAL:
			--m_Charges;
			break;
		case DISCHARGE_TYPE_FINAL:
			m_Charges = 0;
			break;
		case DISCHARGE_TYPE_FREE:
			break;
	}

	ClearLinks();

	if(m_Charges > 0)
	{
		AddLink(GetOwner());
	}

	m_ScheduledDischarge = DISCHARGE_TYPE_INVALID;
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
