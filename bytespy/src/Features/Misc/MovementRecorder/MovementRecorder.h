#pragma once
#include "../../../SDK/SDK.h"

struct RecordedFrame_t {
	Vector   vPosition;
	QAngle   viewAngles;
	float    forwardMove;
	float    sideMove;
	float    upMove;
	bool     jumped;
	bool     ducked;
	uint32_t  buttons;
	float    timeStamp;
	int iRelativeTick;
	int ereretick = 0;
};

class CTraceFilterSkipLocal : public ITraceFilter
{
public:
	CTraceFilterSkipLocal(CBaseEntity* pEntityToSkip) : m_pEntityToSkip(pEntityToSkip) {}

	bool ShouldHitEntity(IHandleEntity* pEntity, int contentsMask) override
	{
		return pEntity != m_pEntityToSkip;
	}

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

private:
	CBaseEntity* m_pEntityToSkip;
};


class CMovementRecorder
{
public:
	std::vector<RecordedFrame_t> m_vFrames;
	Vector   m_vStartPosition;
	float m_flRecordStart = 0.f;
	float m_flPlaybackStart = 0.f;
	bool m_bIsRecording = false;
	bool m_bIsPlaying = false;
	int m_iPlaybackFrame = 0;
	int m_iPlaybackTick = 0;
	int m_iRecordedTickCounter = 0; // starts at 0
	bool m_bHomingToStart = false;

	void CreateMove(CUserCmd* pCmd);
	void EngineVGui();
	void StartRecording();
	void StopRecording();
	void StartPlayback(CUserCmd* pCmd);
	void StopPlayback();
};

ADD_FEATURE(CMovementRecorder, MovementRecorder)
