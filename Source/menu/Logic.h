// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Action.h"
#include "Input.h"
#include "RingBuffer.h"
#include "Logic.generated.h"

typedef struct Player {
  float pos;
  Input input;
  HAction action;
  int actionStart;
  int health;
} Player;

typedef struct Frame {
  Player p1;
  Player p2;
} Frame;


UCLASS()
class MENU_API ALogic : public AActor
{
	GENERATED_BODY()
	
public:	
        int maxRollback = 10; // keep around 10 frames or so for rollback
        RingBuffer<Frame> frames;
        int frame;

	// Sets default values for this actor's properties
	ALogic();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
