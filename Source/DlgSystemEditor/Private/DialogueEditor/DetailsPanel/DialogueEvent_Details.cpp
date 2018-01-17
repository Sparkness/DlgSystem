// Fill out your copyright notice in the Description page of Project Settings.
#include "DialogueEvent_Details.h"

#include "IDetailPropertyRow.h"
#include "PropertyEditing.h"
#include "IPropertyUtilities.h"

#include "DlgNode.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "STextPropertyPickList.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEvent_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEventCustomization
void FDialogueEvent_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = DetailsPanel::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();

	// Cache the Property Handle for the EventType
	ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, ParticipantName));
	EventTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, EventType));
	check(ParticipantNamePropertyHandle.IsValid());
	check(EventTypePropertyHandle.IsValid());

	// Register handler for event type change
	EventTypePropertyHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &Self::OnEventTypeChanged, true));

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueEvent_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Common ParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShareable(new FTextPropertyPickList_CustomRowHelper(DetailWidgetRow, ParticipantNamePropertyHandle));
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		->Update();
	}

	// EventType
	StructBuilder.AddProperty(EventTypePropertyHandle.ToSharedRef());

	// EventName
	{
		const TSharedPtr<IPropertyHandle> EventNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, EventName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EventNameSearchKey", "Event Name"));

		EventNamePropertyRow = MakeShareable(new FTextPropertyPickList_CustomRowHelper(DetailWidgetRow, EventNamePropertyHandle));
		EventNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(STextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesEventNames)
				.OnTextCommitted(this, &Self::HandleTextCommitted)
				.HasContextCheckbox(true)
				.IsContextCheckBoxChecked(false)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueEventNames)
		);
		EventNamePropertyRow->Update();
	}

	// IntValue
	{
		IntValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, IntValue)).ToSharedRef());
		IntValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetIntValueVisibility));
	}

	// FloatValue
	{
		FloatValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, FloatValue)).ToSharedRef());
		FloatValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetFloatValueVisibility));
	}

	// NameValue
	{
		NameValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, NameValue)).ToSharedRef());
		NameValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetNameValueVisibility));
	}

	// bDelta
	{
		BoolDeltaPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bDelta)).ToSharedRef());
		BoolDeltaPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolDeltaVisibility));
	}

	// bValue
	{
		BoolValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bValue)).ToSharedRef());
		BoolValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolValueVisibility));
	}

	// Cache the initial event type
	OnEventTypeChanged(false);
}

void FDialogueEvent_Details::OnEventTypeChanged(bool bForceRefresh)
{
	// Update to the new type
	uint8 value;
	verify(EventTypePropertyHandle->GetValue(value) == FPropertyAccess::Success);
	EventType = static_cast<EDlgEventType>(value);

	// Update the display name/tooltips
	FText EventNameDisplayName = LOCTEXT("EventNameDisplayName", "Variable Name");
	FText EventNameToolTip = LOCTEXT("EventNameToolTip", "Name of the relevant variable");
	if (EventType == EDlgEventType::DlgEventEvent)
	{
		EventNameDisplayName = LOCTEXT("DlgEvent_EventNameDisplayName", "Event Name");
		EventNameToolTip = LOCTEXT("DlgEvent_EventNameToolTip", "Name of the relevant event");
	}

	EventNamePropertyRow->SetDisplayName(EventNameDisplayName)
		->SetToolTip(EventNameToolTip)
		->Update();

	// Refresh the view, without this some names/tooltips won't get refreshed
	if (bForceRefresh && PropertyUtils.IsValid())
		PropertyUtils->ForceRefresh();
}

#undef LOCTEXT_NAMESPACE
