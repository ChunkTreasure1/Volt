#pragma once

#include "PerfTimer.h"

#include <Recast.h>

/// Recast build settings.
struct RecastBuildSettings
{
	// Cell size in world units
	float cellSize = 30.f;
	// Cell height in world units
	float cellHeight = 1.f;
	// Max agents in the world
	int maxAgents = 100;
	// Agent height in world units
	float agentHeight = 200.f;
	// Agent radius in world units
	float agentRadius = 60.f;
	// Agent max climb in world units
	float agentMaxClimb = 90.f;
	// Agent max slope in degrees
	float agentMaxSlope = 45.f;
	// Region minimum size in voxels.
	// regionMinSize = sqrt(regionMinArea)
	float regionMinSize = 8.f;
	// Region merge size in voxels.
	// regionMergeSize = sqrt(regionMergeArea)
	float regionMergeSize = 20.f;
	// Edge max length in world units
	float edgeMaxLen = 12.f;
	// Edge max error in voxels
	float edgeMaxError = 1.3f;
	float vertsPerPoly = 6.f;
	// Detail sample distance in voxels
	float detailSampleDist = 6.f;
	// Detail sample max error in voxel heights.
	float detailSampleMaxError = 1.f;
	// Partition type, see SamplePartitionType
	int partitionType = 0;
	// Size of the tiles in voxels
	float tileSize = 48.f;
	// Build using tile cache
	bool useTileCache = false;
	bool useAutoBaking = true;
};

/// Recast build context.
class RecastBuildContext : public rcContext
{
	TimeVal m_startTime[RC_MAX_TIMERS];
	TimeVal m_accTime[RC_MAX_TIMERS];

	static const int MAX_MESSAGES = 1000;
	const char* m_messages[MAX_MESSAGES];
	int m_messageCount;
	static const int TEXT_POOL_SIZE = 8000;
	char m_textPool[TEXT_POOL_SIZE];
	int m_textPoolSize;

public:
	RecastBuildContext();

	/// Dumps the log to stdout.
	void dumpLog(const char* format, ...);
	/// Returns number of log messages.
	int getLogCount() const;
	/// Returns log message text.
	const char* getLogText(const int i) const;

protected:
	/// Virtual functions for custom implementations.
	///@{
	virtual void doResetLog();
	virtual void doLog(const rcLogCategory category, const char* msg, const int len);
	virtual void doResetTimers();
	virtual void doStartTimer(const rcTimerLabel label);
	virtual void doStopTimer(const rcTimerLabel label);
	virtual int doGetAccumulatedTime(const rcTimerLabel label) const;
	///@}
};
