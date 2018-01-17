// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreTypes.h"
#if WITH_EDITOR
#include "EdGraph/EdGraphNode.h"
#endif

#include "DlgCondition.h"
#include "DlgEvent.h"

#include "DlgNode.generated.h"


class UDlgContextInternal;
class UDlgNode;
class USoundWave;
class UDialogueWave;

/**
 * The representation of a child in a node. Defined by a TargetIndex which points to the index array in the Dialogue.Nodes
 */
USTRUCT()
struct FDlgEdge
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgEdge& Other) const
	{
		return TargetIndex == Other.TargetIndex &&
			Text.EqualTo(Other.Text) &&
			Conditions == Other.Conditions;
	}

	bool operator!=(const FDlgEdge& Other) const
	{
		return !(*this == Other);
	}

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgEdge& DlgEdge)
	{
		Ar << DlgEdge.TargetIndex;
		Ar << DlgEdge.Text;
		Ar << DlgEdge.Conditions;
		return Ar;
	}

	/** Creates a simple edge without text, without conditions */
	FDlgEdge(int32 InTargetIndex = INDEX_NONE) : TargetIndex(InTargetIndex) {}

	/** Returns with true if every condition attached to the edge and every enter condition of the target node are satisfied */
	bool Evaluate(class UDlgContextInternal* DlgContext, TSet<UDlgNode*> AlreadyVisitedNodes) const;

	/** Returns if the Edge is valid, has the TargetIndex non negative  */
	bool IsValid() const
	{
		return TargetIndex > INDEX_NONE;
	}

public:
	/** Index of the node in the Nodes TArray of the dialogue this edge is leading to */
	UPROPERTY(VisibleAnywhere, Meta = (ClampMin = -1))
	int32 TargetIndex = INDEX_NONE;

	/** Required but not sufficient conditions - target node's enter conditions are checked too */
	UPROPERTY(EditAnywhere)
	TArray<FDlgCondition> Conditions;

	/** Text associated with the child, can be used for user choices */
	UPROPERTY(EditAnywhere, Meta = (MultiLine = true))
	FText Text;
};

/**
 *  Abstract base class for Dialogue nodes
 *  Depending on the implementation in the child class the dialogue node can contain one or more lines of one or more participants,
 *  or simply some logic to go on in the UDlgNode graph
 */
UCLASS(BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgNode : public UObject
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	/** UObject serializer. */
	void Serialize(FArchive& Ar) override;

	/** @return a one line description of an object. */
	FString GetDesc() override { return TEXT("INVALID DESCRIPTION"); }

#if WITH_EDITOR
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
	 */
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param InThis Object to collect references from.
	 * @param Collector	FReferenceCollector objects to be used to collect references.
	*/
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject Interface.

	// Begin own function
	// Used internally by the Dialogue editor:
	virtual FString GetNodeTypeString() const { return TEXT("INVALID"); }
#endif //WITH_EDITOR

#if WITH_EDITOR
	void SetGraphNode(UEdGraphNode* InNode) { GraphNode = InNode; }
	void ClearGraphNode() { GraphNode = nullptr; }
	UEdGraphNode* GetGraphNode() const { return GraphNode; }
#endif

	/** Broadcasts whenever a property of this dialogue changes. */
	DECLARE_EVENT_TwoParams(UDlgNode, FDialogueNodePropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */, int32 /* EdgeIndexChanged */);
	FDialogueNodePropertyChanged OnDialogueNodePropertyChanged;

	virtual bool HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<UDlgNode*> NodesEnteredWithThisStep);

	virtual bool ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<UDlgNode*> AlreadyEvaluated);

	virtual bool CheckNodeEnterConditions(UDlgContextInternal* DlgContext, TSet<UDlgNode*> AlreadyVisitedNodes);

	virtual bool OptionSelected(int32 OptionIndex, UDlgContextInternal* DlgContext);

	// Getters/Setters:
	// For the ParticipantName
	virtual FName GetNodeParticipantName() const { return OwnerName; }
	virtual void SetNodeParticipantName(const FName& InName) { OwnerName = InName; }

	// For the EnterConditions
	virtual const TArray<FDlgCondition>& GetNodeEnterConditions() const { return EnterConditions; }
	virtual void SetNodeEnterConditions(const TArray<FDlgCondition>& InEnterConditions) { EnterConditions = InEnterConditions; }

	/** Gets the mutable enter condition at location EnterConditionIndex. */
	virtual FDlgCondition* GetMutableEnterConditionAt(int32 EnterConditionIndex)
	{
		check(EnterConditions.IsValidIndex(EnterConditionIndex));
		return &EnterConditions[EnterConditionIndex];
	}

	// For the EnterEvents
	virtual const TArray<FDlgEvent>& GetNodeEnterEvents() const { return EnterEvents; }
	virtual void SetNodeEnterEvents(const TArray<FDlgEvent>& InEnterEvents) { EnterEvents = InEnterEvents; }

	// For the Children
	/** Gets this nodes children (edges) as a const/mutable array */
	virtual const TArray<FDlgEdge>& GetNodeChildren() const { return Children; }
	virtual void SetNodeChildren(const TArray<FDlgEdge>& InChildren) { Children = InChildren; }

	/** Adds an Edge to the end of the Children Array. */
	virtual void AddNodeChild(const FDlgEdge& InChild) { Children.Add(InChild); }

	/** Removes the Edge at the specified EdgeIndex location. */
	virtual void RemoveChildAt(int32 EdgeIndex)
	{
		check(Children.IsValidIndex(EdgeIndex));
		Children.RemoveAt(EdgeIndex);
	}

	/** Removes all edges/children */
	virtual void RemoveAllChildren() { Children.Empty(); }

	/** Gets the mutable edge/child at location EdgeIndex. */
	virtual FDlgEdge* GetMutableNodeChildAt(int32 EdgeIndex)
	{
		check(Children.IsValidIndex(EdgeIndex));
		return &Children[EdgeIndex];
	}

	/** Gets the mutable Edge that corresponds to the provided TargetIndex or nullptr if nothing was found. */
	virtual FDlgEdge* GetMutableNodeChildForTargetIndex(int32 TargetIndex);

	/** Gets all the edges (children) indicies that DO NOT have a valid TargetIndex (is negative). */
	const TArray<int32> GetNodeOpenChildren_DEPRECATED() const;

	/** Gathers associated participants, they are only added to the array if they are not yet there */
	virtual void GetAssociatedParticipants(TArray<FName>& OutArray) const;

	/** Gets the Text of this Node */
	virtual const FText& GetNodeText() const { return FText::GetEmpty(); }

	/** Gets the voice of this Node as a SoundWave. */
	virtual USoundWave* GetNodeVoiceSoundWave() const { return nullptr; }

	/** Gets the voice of this Node as a DialogueWave. Only the first Dialogue context in the wave should be used. */
	virtual UDialogueWave* GetNodeVoiceDialogueWave() const { return nullptr; }

	/** Helper method to get directly the Dialogue */
	class UDlgDialogue* GetDialogue() const;

	/** Helper functions to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameOwnerName() { return GET_MEMBER_NAME_CHECKED(UDlgNode, OwnerName); }
	static FName GetMemberNameCheckChildrenOnEvaluation() { return GET_MEMBER_NAME_CHECKED(UDlgNode, bCheckChildrenOnEvaluation); }
	static FName GetMemberNameEnterConditions() { return GET_MEMBER_NAME_CHECKED(UDlgNode, EnterConditions); }
	static FName GetMemberNameEnterEvents() { return GET_MEMBER_NAME_CHECKED(UDlgNode, EnterEvents); }
	static FName GetMemberNameChildren() { return GET_MEMBER_NAME_CHECKED(UDlgNode, Children); }

protected:

	void FireNodeEnterEvents(UDlgContextInternal* DlgContext);

protected:

#if WITH_EDITORONLY_DATA
	/** Node's Graph representation, used to get position. */
	UPROPERTY(Meta = (DlgNoExport))
	UEdGraphNode* GraphNode = nullptr;

	// Used to build the change event and broadcast it
	int32 BroadcastPropertyEdgeIndexChanged = INDEX_NONE;
#endif

	/** Name of a participant (speaker) associated with this node. */
	UPROPERTY(EditAnywhere, Category = DlgNodeData, Meta = (DisplayName = "Participant Name"))
	FName OwnerName;

	/**
	 *  If it is set the node is only satisfied if at least one of its children is
	 *  Should not be used if entering this node can modify the condition results of its children.
	 */
	UPROPERTY(EditAnywhere, Category = DlgNodeData)
	bool bCheckChildrenOnEvaluation = false;

	/** Conditions necessary to enter this node */
	UPROPERTY(EditAnywhere, Category = DlgNodeData)
	TArray<FDlgCondition> EnterConditions;

	/** Events fired when the node is reached in the dialogue */
	UPROPERTY(EditAnywhere, Category = DlgNodeData)
	TArray<FDlgEvent> EnterEvents;

	/** Edges that point to Children of this Node */
	UPROPERTY(EditAnywhere, EditFixedSize, AdvancedDisplay, Category = DlgNodeData)
	TArray<FDlgEdge> Children;
};
