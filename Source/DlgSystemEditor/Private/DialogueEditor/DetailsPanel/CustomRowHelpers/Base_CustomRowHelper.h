// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "EditorStyle.h"
#include "SSearchBox.h"
#include "DetailWidgetRow.h"

class FDetailWidgetRow;

/**
 * Base class for all Custom Row Helpers.
 * Makes sure that the ToolTips and Display Names are in sync with the PropertyHandle and the SWidget that is represented by.
 */
class FBase_CustomRowHelper
{
	typedef FBase_CustomRowHelper Self;
public:
	FBase_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, TSharedPtr<IPropertyHandle> InPropertyHandle);
	virtual ~FBase_CustomRowHelper() {}

	/** Sets the localized display name of the property. */
	Self* SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return this;
	}

	/** Sets the localized tooltip of the property. */
	Self* SetToolTip(const FText& InToolTip)
	{
		ToolTip = InToolTip;
		return this;
	}

	/** Sets the visibility of this property. */
	Self* SetVisibility(const TAttribute<EVisibility>& InVisibility)
	{
		DetailWidgetRow->Visibility(InVisibility);
		return this;
	}

	/** Update the full property row. */
	void Update();

protected:
	/** Internal update method that must be implemented. Called after Update finishes. */
	virtual void UpdateInternal() = 0;

protected:
	/** The custom widget row this represents */
	FDetailWidgetRow* DetailWidgetRow = nullptr;

	/** The Property handle of what this row represents */
	TSharedPtr<IPropertyHandle> PropertyHandle;

	/** The NameContent Widget. */
	TSharedPtr<SWidget> NameContentWidget;

	/** Texts used for this property row. */
	FText DisplayName;
	FText ToolTip;
};
