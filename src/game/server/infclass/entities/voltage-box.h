/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_INFCLASS_ENTITIES_VOLTAGE_BOX_H
#define GAME_SERVER_INFCLASS_ENTITIES_VOLTAGE_BOX_H

#include "infcentity.h"

class CVoltageBox : public CInfCEntity
{
public:
	static int EntityId;
	static const int MaxLinks = 8;

	CVoltageBox(CGameContext *pGameContext, vec2 CenterPos, int Owner);
	~CVoltageBox() override;

	void AddLink(int ClientID);
	void RemoveLink(int ClientID);
	void Discharge();
	void FinalDischarge();

	void Tick() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;

protected:
	void BakeSnapItems();
	void UpdateLinks();
	void UpdateDischarge();

	int GetStartTickForDistance(float Progress);

	enum
	{
		BoxEdges = 4,
	};

	struct LaserSnapItem
	{
		ivec2 From;
		ivec2 To;
		int StartTick;
		int SnapID;
	};
	static const int MaxSnapItems = BoxEdges + 1 + MaxLinks * 2;

	LaserSnapItem m_LasersForSnap[MaxSnapItems];
	int m_ActiveSnapItems = 0;

	int m_Charges = 0;
	int m_LinksCount = 0;
	int m_LinkedClientIDs[MaxLinks];
	int m_DischargeTick = 0;
};

#endif // GAME_SERVER_INFCLASS_ENTITIES_VOLTAGE_BOX_H
