#include "lightning-laser.h"

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/infclass/classes/infcplayerclass.h>
#include <game/server/infclass/entities/infccharacter.h>
#include <game/server/infclass/infcgamecontroller.h>

#include <algorithm>

int CLightningLaser::EntityId = CGameWorld::ENTTYPE_LIGHTNING_LASER;

static const vec2 DamageForce(0, 0);

CLightningLaser::CLightningLaser(CGameContext *pGameContext, const vec2 &Pos, vec2 Direction, float StartEnergy, int Owner)
	: CInfCEntity(pGameContext, EntityId, Pos, Owner)
{
	m_Direction = Direction;
	m_Energy = StartEnergy;

	for(LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		SnapItem.SnapID = Server()->SnapNewID();
	}

	GameWorld()->InsertEntity(this);
}

CLightningLaser::~CLightningLaser()
{
	for(const LaserSnapItem &SnapItem : m_LasersForSnap)
	{
		Server()->SnapFreeID(SnapItem.SnapID);
	}
}

void CLightningLaser::Tick()
{
	if(m_Links.IsEmpty())
	{
		GenerateThePath();
	}
	else
	{
		UpdateThePath();
	}

	if(m_Links.IsEmpty())
	{
		Reset();
	}
	PrepareSnapItems();
}

void CLightningLaser::Snap(int SnappingClient)
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

void CLightningLaser::AddSnapItem(const vec2 &From, const vec2 &To, int SnapTick)
{
	m_LasersForSnap[m_ActiveSnapItems].From.x = From.x;
	m_LasersForSnap[m_ActiveSnapItems].From.y = From.y;
	m_LasersForSnap[m_ActiveSnapItems].To.x = To.x;
	m_LasersForSnap[m_ActiveSnapItems].To.y = To.y;
	m_LasersForSnap[m_ActiveSnapItems].StartTick = SnapTick;
	++m_ActiveSnapItems;
}

void CLightningLaser::PrepareSnapItems()
{
	m_ActiveSnapItems = 0;

	int Tick = Server()->Tick();
	vec2 PreviousPoint = GetPos();
	for(int i = 0; i < m_Links.Size(); ++i)
	{
		AddSnapItem(PreviousPoint, m_Links[i].Endpoint, m_Links[i].StartTick); // Bold for now
		PreviousPoint = m_Links[i].Endpoint;
	}
}

static bool OnlyLinkableCharacterFilter(const CCharacter *pCharacter)
{
	const CInfClassCharacter *pInfCharacter = CInfClassCharacter::fromCharacter(pCharacter);
	return pInfCharacter->GetClass()->CanBeLinked();
}

void CLightningLaser::GenerateThePath()
{
	int Tick = Server()->Tick();
	const int BaseDamage = Config()->m_InfLightningLaserBaseDamage;
	const int MaxBounceDistance = Config()->m_InfLightningLaserBounceMaxDistance;

	m_Links.Clear();

	vec2 To = GetPos() + m_Direction * m_Energy;

	// Limit the 'To' value.
	GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To);

	vec2 At;
	CInfClassCharacter *pOwnerChar = GameController()->GetCharacter(GetOwner());
	CInfClassCharacter *pHit = static_cast<CInfClassCharacter*>(GameWorld()->IntersectCharacter(m_Pos, To, 0.f, At, OnlyLinkableCharacterFilter));

	if(!pHit)
	{
		m_Links.Add(Link(To, Tick));
		return;
	}

	int Damage = BaseDamage;

	ClientsArray LinkedClientIDs;

	vec2 LastLinkedPoint = At;

	m_Links.Add(Link(pHit->GetPos(), Tick));
	LinkedClientIDs.Add(pHit->GetCID());
	pHit->TakeDamage(DamageForce, Damage, GetOwner(), WEAPON_SHOTGUN, TAKEDAMAGEMODE_NOINFECTION);
	Damage -= 1;

	while(Damage > 0)
	{
		ClientsArray ClientsInRange;
		GetSortedTargetsInRange(LastLinkedPoint, MaxBounceDistance, LinkedClientIDs, &ClientsInRange);

		// 1. Find the nearest
		// 2. Try to hit.
		// 3. If can't hit - go on the list
		// 3. If no hits - abort
		pHit = nullptr;
		for(int i = 0; i < ClientsInRange.Size(); ++i)
		{
			const int ClientID = ClientsInRange.At(i);

			CInfClassCharacter *pCharacterNearby = GameController()->GetCharacter(ClientID);
			if(!pCharacterNearby->GetClass()->CanBeLinked())
			{
				continue;
			}

			bool Intersected = GameServer()->Collision()->IntersectLine(LastLinkedPoint, pCharacterNearby->GetPos(), 0x0, &To);
			if(Intersected)
			{
				// Unable to hit through the walls
				continue;
			}

			pHit = pCharacterNearby;
			m_Links.Add(Link(pHit->GetPos(), Tick));
			LinkedClientIDs.Add(pHit->GetCID());
			pHit->TakeDamage(DamageForce, Damage, GetOwner(), WEAPON_SHOTGUN, TAKEDAMAGEMODE_NOINFECTION);
			Damage -= 1;
			break;
		}
		if(!pHit)
		{
			break;
		}
	}
}

void CLightningLaser::UpdateThePath()
{
	if(!m_Links.IsEmpty())
	{
		if(m_Links.At(0).StartTick < Server()->Tick() - 5)
		{
			m_Links.Clear();
		}
	}
}

struct DistanceItem
{
	DistanceItem() = default;
	DistanceItem(int C, float D)
		: ClientID(C)
		, Distance(D)
	{
	}

	int ClientID;
	float Distance;
};

bool DistanceItemComparator(const DistanceItem &DistanceItem1, const DistanceItem &DistanceItem2)
{
	return DistanceItem1.Distance < DistanceItem2.Distance;
}

void CLightningLaser::GetSortedTargetsInRange(const vec2 &Center, const float Radius, const ClientsArray &SkipList, ClientsArray *pOutput)
{
	array_on_stack<DistanceItem, MAX_CLIENTS> Distances;

	for(int ClientID = 0; ClientID < MAX_CLIENTS; ++ClientID)
	{
		if(SkipList.Contains(ClientID))
			continue;

		const CCharacter *pChar = GameController()->GetCharacter(ClientID);
		if(!pChar)
			continue;

		const vec2 &CharPos = pChar->GetPos();
		const float Distance = distance(CharPos, Center);
		if(Distance > Radius)
			continue;

		Distances.Add(DistanceItem(ClientID, Distance));

		std::sort(Distances.begin(), Distances.end(), &DistanceItemComparator);
	}

	pOutput->Clear();
	for(const DistanceItem &DistanceItem : Distances)
	{
		pOutput->Add(DistanceItem.ClientID);
	}
}
