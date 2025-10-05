// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Targeting/TetheredTargetingSortTaskBase.h"



//namespace TargetingSystemCVars
//{
//    static bool bEnableSortTargetingDebug = false;
//    FAutoConsoleVariableRef CvarEnableSortingTargetingDebugging(
//        /EXT(Tethered.Targeting.EnableDebug.Sort.Base”),
//        bEnableSortTargetingDebug,
//        /EXT(“Enable on - screen debugging for base sort targeting. (Enabled: true, Disabled : false)”));
//
//}

void UTetheredTargetingSortTaskBase::Init(const FTargetingRequestHandle& TargetingHandle) const
{
    Super::Init(TargetingHandle);

    if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
    {
        if (APawn* InPawn = Cast<APawn>(SourceContext->SourceActor))
        {
            SourcePawn = InPawn;
            SourcePlayerController = Cast<APlayerController>(SourcePawn->GetController());
            bIsSourceActorMoving = SourcePawn->GetLastMovementInputVector().Length() > 0.0f;
        }
    }

    FinalScoreMultiplier = bIsSourceActorMoving ? ScoreMultiplierWhenMoving : DefaultScoreMultiplier;
}

void UTetheredTargetingSortTaskBase::Execute(const FTargetingRequestHandle& TargetingHandle) const
{
    SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Executing);

    if (TargetingHandle.IsValid())
    {
        if (FTargetingDefaultResultsSet* ResultData = FTargetingDefaultResultsSet::Find(TargetingHandle))
        {
            const int32 NumTargets = ResultData->TargetResults.Num();

            TArray<float> RawScores;
            for (const FTargetingDefaultResultData& TargetResult : ResultData->TargetResults)
            {
                const float RawScore = GetScoreForTarget(TargetingHandle, TargetResult);
                RawScores.Add(RawScore);
            }

            if (ensureMsgf(NumTargets == RawScores.Num(), TEXT("The cached raw scores should be the same size as the number of targets!")))
            {
                // Adding the normalized scores to each target result.
                for (int32 TargetIterator = 0; TargetIterator < NumTargets; ++TargetIterator)
                {
                    FTargetingDefaultResultData& TargetResult = ResultData->TargetResults[TargetIterator];
                    TargetResult.Score += RawScores[TargetIterator];
                }
            }

            // sort the set
            ResultData->TargetResults.Sort([this](const FTargetingDefaultResultData& Lhs, const FTargetingDefaultResultData& Rhs)
                {
                    return Lhs.Score < Rhs.Score;
                });
        }

    }

    SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);

    /*if (GASCourse_TargetingSystemCVars::bEnableSortTargetingDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT(“Is Source Actor Moving ? % s”), bIsSourceActorMoving ? TEXT(“TRUE”) : TEXT(“FALSE”));
        UE_LOG(LogTemp, Warning, TEXT(“Final Score Multiplier = % f”), FinalScoreMultiplier);
    }*/
}

float UTetheredTargetingSortTaskBase::GetScoreForTarget(const FTargetingRequestHandle& TargetingHandle,
    const FTargetingDefaultResultData& TargetData) const
{
    return Super::GetScoreForTarget(TargetingHandle, TargetData);
}
