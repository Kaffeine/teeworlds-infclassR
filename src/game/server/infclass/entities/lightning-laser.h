#ifndef GAME_SERVER_ENTITIES_LIGHTNING_LASER_H
#define GAME_SERVER_ENTITIES_LIGHTNING_LASER_H

#include "infcentity.h"

#include "base/tl/array_on_stack.h"

class CLightningLaser : public CInfCEntity
{
public:
	static int EntityId;

	static const int MaxTargets = 20;
	CLightningLaser(CGameContext *pGameContext, const vec2 &Pos, vec2 Direction, float StartEnergy, int Owner);
	~CLightningLaser() override;

	void Tick() override;
	void Snap(int SnappingClient) override;

protected:
	using ClientsArray = array_on_stack<int, MaxTargets>;
	void AddSnapItem(const vec2 &From, const vec2 &To, int SnapTick);
	void PrepareSnapItems();

	void GenerateThePath();
	void UpdateThePath();
	void GetSortedTargetsInRange(const vec2 &Center, const float Radius, const ClientsArray &SkipList, ClientsArray *pOutput);

	struct LaserSnapItem
	{
		ivec2 From;
		ivec2 To;
		int StartTick;
		int SnapID;
	};

	struct Link
	{
		Link() = default;
		Link(const vec2 &Pos, int Tick)
			: Endpoint(Pos)
			, StartTick(Tick)
		{
		}

		vec2 Endpoint;
		int StartTick;
	};

	static const int MaxSnapItems = MaxTargets + 1;
	LaserSnapItem m_LasersForSnap[MaxSnapItems];
	int m_ActiveSnapItems = 0;

	array_on_stack<Link, MaxTargets + 1> m_Links;
	vec2 m_Direction;
	float m_Energy = 0;

};

#endif // GAME_SERVER_ENTITIES_LIGHTNING_LASER_H
