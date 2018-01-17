// Fill out your copyright notice in the Description page of Project Settings.
#include "DlgMemory.h"

void DlgMemory::SetEntry(const FGuid& DlgGuid, const FDlgHistory& History)
{
	FDlgHistory* OldEntry = HistoryMap.Find(DlgGuid);

	if (OldEntry == nullptr)
	{
		HistoryMap.Add(DlgGuid, History);
	}
	else
	{
		*OldEntry = History;
	}
}

void DlgMemory::SetNodeVisited(const FGuid& DlgGuid, const int32 NodeIndex)
{
	FDlgHistory* History = HistoryMap.Find(DlgGuid);
	// Add it if it does not exist already
	if (History == nullptr)
	{
		History = &HistoryMap.Add(DlgGuid);
	}

	History->VisitedNodeIndices.Add(NodeIndex);
}


bool DlgMemory::IsNodeVisited(const FGuid& DlgGuid, const int32 NodeIndex) const
{
	const FDlgHistory* History = HistoryMap.Find(DlgGuid);
	if (History == nullptr)
	{
		return false;
	}

	const int32* FoundIndex = History->VisitedNodeIndices.Find(NodeIndex);
	return FoundIndex != nullptr;
}


void DlgMemory::Serialize(FArchive& Ar)
{
	Ar << HistoryMap;
}

