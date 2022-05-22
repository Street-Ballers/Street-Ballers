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
  int8 buttonsPressed;
  int8 buttonsReleased;

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
  void ServerButtons(int8 _buttonsPressed, int8 _buttonsReleased, int targetFrame);

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
