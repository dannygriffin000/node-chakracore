//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "CommonDefines.h"
#include "RecyclerWaitReason.h"
#include "Common/Tick.h"
#include "AllocatorTelemetryStats.h"
#include "HeapBucketStats.h"

#ifdef ENABLE_BASIC_TELEMETRY
#include "Windows.h"
#endif

namespace Memory
{
    class Recycler;
    class IdleDecommitPageAllocator;
    class RecyclerTelemetryInfo;

    // struct that defines an interface with host on how we can get key data stats
    class RecyclerTelemetryHostInterface
    {
    public:
        virtual LPFILETIME GetLastScriptExecutionEndTime() const = 0;
        virtual bool TransmitTelemetry(RecyclerTelemetryInfo& rti) = 0;
        virtual bool TransmitTelemetryError(const RecyclerTelemetryInfo& rti, const char* msg) = 0;
        virtual bool IsThreadBound() const = 0;
        virtual DWORD GetCurrentScriptThreadID() const = 0;
    };

#ifdef ENABLE_BASIC_TELEMETRY

    /**
     *  struct with all data we want to capture for a specific GC pass.
     *
     *  This forms a linked list, one for each per-stats pass.
     *  The last element in the list points to the head instead of nullptr.
     */
    struct RecyclerTelemetryGCPassStats
    {
        FILETIME passStartTimeFileTime;
        Js::Tick passStartTimeTick;
        Js::Tick passEndTimeTick;
        Js::TickDelta startPassProcessingElapsedTime;
        Js::TickDelta endPassProcessingElapsedTime;
        Js::TickDelta computeBucketStatsElapsedTime;
        FILETIME lastScriptExecutionEndTime;
        RecyclerTelemetryGCPassStats* next;
        Js::TickDelta uiThreadBlockedTimes[static_cast<size_t>(RecyclerWaitReason::Other) + 1];
        bool isInScript;
        bool isScriptActive;
        bool isGCPassActive;

        size_t processAllocaterUsedBytes_start;
        size_t processAllocaterUsedBytes_end;
        size_t processCommittedBytes_start;
        size_t processCommittedBytes_end;

        HeapBucketStats bucketStats;

        AllocatorSizes threadPageAllocator_start;
        AllocatorSizes threadPageAllocator_end;
        AllocatorSizes recyclerLeafPageAllocator_start;
        AllocatorSizes recyclerLeafPageAllocator_end;
        AllocatorSizes recyclerLargeBlockPageAllocator_start;
        AllocatorSizes recyclerLargeBlockPageAllocator_end;

#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE
        AllocatorSizes recyclerWithBarrierPageAllocator_start;
        AllocatorSizes recyclerWithBarrierPageAllocator_end;
#endif
    };

    /**
     *
     */
    class RecyclerTelemetryInfo
    {
    public:
        RecyclerTelemetryInfo(Recycler* recycler, RecyclerTelemetryHostInterface* hostInterface);
        ~RecyclerTelemetryInfo();

        void StartPass();
        void EndPass();
        void IncrementUserThreadBlockedCount(Js::TickDelta waitTime, RecyclerWaitReason source);
        
        inline const Js::Tick& GetRecyclerStartTime() const { return this->recyclerStartTime;  }
        inline const RecyclerTelemetryGCPassStats* GetLastPassStats() const { return this->lastPassStats; }
        inline const Js::Tick& GetLastTransmitTime() const { return this->lastTransmitTime; }
        inline const uint16 GetPassCount() const { return this->passCount; }
        const GUID& GetRecyclerID() const;
        bool GetIsConcurrentEnabled() const;
        bool ShouldCaptureRecyclerTelemetry() const;
        bool IsOnScriptThread() const;

        AllocatorDecommitStats* GetThreadPageAllocator_decommitStats() { return &this->threadPageAllocator_decommitStats; }
        AllocatorDecommitStats* GetRecyclerLeafPageAllocator_decommitStats() { return &this->recyclerLeafPageAllocator_decommitStats; }
        AllocatorDecommitStats* GetRecyclerLargeBlockPageAllocator_decommitStats() { return &this->recyclerLargeBlockPageAllocator_decommitStats; }
#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE
        AllocatorDecommitStats* GetRecyclerWithBarrierPageAllocator_decommitStats() { return &this->recyclerWithBarrierPageAllocator_decommitStats; }
#endif

    private:
        Recycler* recycler;
        DWORD mainThreadID;
        RecyclerTelemetryHostInterface * hostInterface;
        Js::Tick recyclerStartTime;

        // TODO: update list below to SList.  Need to ensure we have same allocation semantics (specifically heap allocs, no exceptions on failure)
        RecyclerTelemetryGCPassStats* lastPassStats;
        Js::Tick lastTransmitTime;
        uint16 passCount;
        bool abortTelemetryCapture;

        AllocatorDecommitStats threadPageAllocator_decommitStats;
        AllocatorDecommitStats recyclerLeafPageAllocator_decommitStats;
        AllocatorDecommitStats recyclerLargeBlockPageAllocator_decommitStats;
#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE
        AllocatorDecommitStats recyclerWithBarrierPageAllocator_decommitStats;
#endif

        bool ShouldTransmit();
        void FreeGCPassStats();
        void Reset();
        void FillInSizeData(IdleDecommitPageAllocator* allocator, AllocatorSizes* sizes) const;

        static size_t GetProcessCommittedBytes();
    };
#else
    class RecyclerTelemetryInfo
    {
    };
#endif // #ifdef ENABLE_BASIC_TELEMETRY

}

