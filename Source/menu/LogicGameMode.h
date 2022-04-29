// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LogicPlayerController.h"
#include "LogicGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MENU_API ALogicGameMode : public AGameMode
{
  GENERATED_BODY()

public:
  ALogicGameMode();

  void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

  void PreLogin(const FString& Options,
                const FString& Address,
                const FUniqueNetIdRepl& UniqueId,
                FString& ErrorMessage);

  void PostLogin(APlayerController* NewPlayer);
};
