// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Action.h"
#include "Input.h"
#include "Logic.generated.h"

class Player {
public:
  FVector pos;
  HAction action;
  int actionStart;
  int health;

  Player(FVector pos, HAction action): pos(pos), action(action), actionStart(0), health(100) {};
};

class Frame {
public:
  Player p1;
  Player p2;

  Frame(Player p1, Player p2): p1(p1), p2(p2) {};
};

class RingBuffer {
private:
  Frame* v = nullptr;
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

// TODO: make this a subclass of AInfo instead
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
        UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
        FVector stageBoundLeft;
        UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
        FVector stageBoundRight;

        // Starting positions of the characters in the stage. We again
        // use these just for their coordinates.
        UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
        FVector leftStart;
        UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
        FVector rightStart;

        int maxRollback = 10; // keep around 10 frames or so for rollback
        RingBuffer frames;
        Input p1Input;
        Input p2Input;
        int frame;

	// Sets default values for this actor's properties
	ALogic();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

        // Getters to get values for updating other actors
        const Player& getPlayer1();
        const Player& getPlayer2();
};

static inline ALogic* FindLogic(UWorld *world) {
  TActorIterator<ALogic> i (world);
  ALogic *l = Cast<ALogic>(*i);
  if (l) {
    UE_LOG(LogTemp, Warning, TEXT("ALogic found!"));
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("ALogic NOT found!"));
  }
  return l;
}
