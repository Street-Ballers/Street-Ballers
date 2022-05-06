// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FightInput.h"
#include "LogicPlayerController.generated.h"

UCLASS()
class MENU_API ALogicPlayerController : public APlayerController
{
  GENERATED_BODY()

private:
  int playerNumber;
  bool readiedUp;
  AFightInput* input;

protected:
	virtual void SetupInputComponent() override;

public:
  ALogicPlayerController();

  UFUNCTION (Server, Reliable)
  void ServerPostLogin(int playerNumber_);
  UFUNCTION (Client, Reliable)
  void ClientPostLogin(int playerNumber_);
  void BeginPlay();

  UFUNCTION (Server, Reliable)
  void ServerReadyUp();

  void Tick(float deltaSeconds);

  UFUNCTION (Server, Reliable)
  void ServerButtons(int8 buttonsPressed, int8 buttonsReleased, int targetFrame);

  UFUNCTION (BlueprintCallable, Category="Player")
  int getPlayerNumber();

  void RightInput(float value);
  void LeftInput(float value);
  void HP();
  void LP();
  void HK();
  void LK();
};
