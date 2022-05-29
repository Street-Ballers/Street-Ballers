// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FightInput.h"
#include "LogicPlayerController.generated.h"

class ALogic;

UCLASS()
class MENU_API ALogicPlayerController : public APlayerController
{
  GENERATED_BODY()

private:
  int playerNumber;
  bool readiedUp;
  bool addedPC;
  AFightInput* input;
  ALogic *l;
  int8 buttonsPressed;
  int8 buttonsReleased;
  int lastTick;

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
  void ServerReadyUp(int p2Char);

  void Tick(float deltaSeconds);
  void sendButtons();

  UFUNCTION (Server, Reliable)
    void ServerButtons(int8 _buttonsPressed, int8 _buttonsReleased, int targetFrame, int avgLatency);

  UFUNCTION (BlueprintCallable, Category="Player")
  int getPlayerNumber();

  void ButtonRightPressed();
  void ButtonLeftPressed();
  void ButtonUpPressed();
  void ButtonDownPressed();
  void ButtonRightReleased();
  void ButtonLeftReleased();
  void ButtonUpReleased();
  void ButtonDownReleased();
  void ButtonHP();
  void ButtonLP();
  void ButtonHK();
  void ButtonLK();
};
