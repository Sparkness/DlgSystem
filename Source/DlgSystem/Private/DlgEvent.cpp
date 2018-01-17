// Fill out your copyright notice in the Description page of Project Settings.
#include "DlgEvent.h"
#include "DlgDialogueParticipant.h"

void FDlgEvent::Call(UObject* TargetParticipant) const
{
	if (TargetParticipant == nullptr)
		return;

	switch (EventType)
	{
	case EDlgEventType::DlgEventEvent:
		IDlgDialogueParticipant::Execute_OnDialogueEvent(TargetParticipant, EventName);
		break;
	case EDlgEventType::DlgEventModifyInt:
		IDlgDialogueParticipant::Execute_ModifyIntValue(TargetParticipant, EventName, bDelta, IntValue);
		break;
	case EDlgEventType::DlgEventModifyFloat:
		IDlgDialogueParticipant::Execute_ModifyFloatValue(TargetParticipant, EventName, bDelta, FloatValue);
		break;
	case EDlgEventType::DlgEventModifyBool:
		IDlgDialogueParticipant::Execute_ModifyBoolValue(TargetParticipant, EventName, bValue);
		break;
	case EDlgEventType::DlgEventModifyName:
		IDlgDialogueParticipant::Execute_ModifyNameValue(TargetParticipant, EventName, NameValue);
		break;
	default:
		checkNoEntry();
	}
}

FArchive& operator<<(FArchive &Ar, FDlgEvent& DlgEvent)
{
	Ar << DlgEvent.ParticipantName;
	Ar << DlgEvent.EventName;
	Ar << DlgEvent.IntValue;
	Ar << DlgEvent.FloatValue;
	Ar << DlgEvent.NameValue;
	Ar << DlgEvent.bDelta;
	Ar << DlgEvent.bValue;
	Ar << DlgEvent.EventType;
	return Ar;
}
