// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/Widget.h"
#include "StreetBrallersGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MENU_API UStreetBrallersGameInstance : public UGameInstance
{
  GENERATED_BODY()
	
public:

  UPROPERTY(BlueprintReadWrite)
    int p1Char;
  UPROPERTY(BlueprintReadWrite)
    int p2Char;

  UPROPERTY(BlueprintReadWrite)
    bool isOnline;

  UPROPERTY(BlueprintReadWrite)
    bool turn;

  void OnStart();

  UFUNCTION (BlueprintImplementableEvent, Category="Menu")
    void ReturnToMenuWithMessage(const FString& message);
  UFUNCTION (BlueprintImplementableEvent, Category="Menu")
    void ReturnToMenu();
  UFUNCTION (BlueprintImplementableEvent, Category="Menu")
    void CreateMessagesWidget();

  virtual void LoadComplete(const float LoadTime, const FString& MapName) override;
};
