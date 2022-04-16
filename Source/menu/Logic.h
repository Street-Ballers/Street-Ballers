// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Action.h"
#include "Input.h"
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

class RingBuffer {
private:
  Frame* v;
  int n;
  int start;
  int end;

public:
  RingBuffer() = default;
  
  void reserve(int size);

  void push(const Frame& x);

  const Frame& last();

  ~RingBuffer();
};

UCLASS()
class MENU_API ALogic : public AActor
{
	GENERATED_BODY()
	
public:
        // character references so we can actually command them.
        UPROPERTY(EditAnywhere)
        AActor* character1;
        UPROPERTY(EditAnywhere)
        AActor* character2;

        // HUD reference so we can actually command it.
        UPROPERTY(EditAnywhere)
        AActor* hud;

        // Invisible objects at the ends of the stages. We will use
        // these just to grab their coordinates and not let players
        // move past them.
        UPROPERTY(EditAnywhere)
        AActor* stageBoundLeft;
        UPROPERTY(EditAnywhere)
        AActor* stageBoundRight;

        // Starting positions of the characters in the stage. We again
        // use these just for their coordinates.
        UPROPERTY(EditAnywhere)
        AActor* leftStart;
        UPROPERTY(EditAnywhere)
        AActor* rightStart;

        int maxRollback = 10; // keep around 10 frames or so for rollback
        RingBuffer frames;
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
