/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "voltage-box.h"

#include <game/server/gamecontext.h>

int CVoltageBox::EntityId = CGameWorld::ENTTYPE_VOLTAGE_BOX;
static constexpr int BoxProximityRadius = 24;

CVoltageBox::CVoltageBox(CGameContext *pGameContext, vec2 CenterPos, int Owner)
	: CInfCEntity(pGameContext, EntityId, CenterPos, Owner, BoxProximityRadius)
{
	GameWorld()->InsertEntity(this);
	
	for(int i=0; i<EDGES; i++)
	{
		m_ExtraIDs[i] = Server()->SnapNewID();
	}
}

CVoltageBox::~CVoltageBox()
{
	for(int i=0; i<EDGES; i++)
	{
		Server()->SnapFreeID(m_ExtraIDs[i]);
	}
}

void CVoltageBox::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	static const vec2 Vertices[EDGES] =
	{
		vec2(-BoxProximityRadius, -BoxProximityRadius),
		vec2( BoxProximityRadius, -BoxProximityRadius),
		vec2( BoxProximityRadius,  BoxProximityRadius),
		vec2(-BoxProximityRadius,  BoxProximityRadius),
	};

	for(int i=0; i<EDGES; i++)
	{
		const bool Last = i == EDGES - 1;
		vec2 PartPosStart = Last ? m_Pos + Vertices[0] : m_Pos + Vertices[i + 1];
		vec2 PartPosEnd = m_Pos + Vertices[i];

		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
			NETOBJTYPE_LASER, m_ExtraIDs[i], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_X = PartPosEnd.x;
		pObj->m_Y = PartPosEnd.y;
		pObj->m_FromX = PartPosStart.x;
		pObj->m_FromY = PartPosStart.y;
		pObj->m_StartTick = Server()->Tick()-4;
	}

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
		NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));

	if(!pObj)
		return;

	pObj->m_X = m_Pos.x;
	pObj->m_Y = m_Pos.y;
	pObj->m_FromX = m_Pos.x;
	pObj->m_FromY = m_Pos.y;
	pObj->m_StartTick = Server()->Tick();
}
