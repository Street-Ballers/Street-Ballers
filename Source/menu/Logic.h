// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Action.h"
#include "FightInput.h"
#include "Logic.generated.h"

class Player {
public:
  FVector pos;
  HAction action;
  bool isFacingRight;
  int actionStart;
  int health;
  int hitstun = 0;

  Player(FVector pos, HAction action): pos(pos), action(action), actionStart(0), health(100) {};
  Player() {};

  void TryStartingNewAction(int frame, AFightInput& input, bool isOnLeft);
  float collidesWithBoundary(float boundary, bool isRightBound);
  void doDamagedAction(int frame);
  void doBlockAction(int frame);
};

class Frame {
public:
  Player p1;
  Player p2;
  int hitstop = 0; // number of frames of hitstop left
  int hitPlayer; // when hitstop>0, 0=both, 1=p1, 2=p2

  Frame(Player p1, Player p2): p1(p1), p2(p2) {};
  Frame() {};
};

class RingBuffer {
private:
  std::vector<Frame> v;
  int n;
  int end;

public:
  RingBuffer() = default;
  
  void reserve(int size);

  void clear();

  void push(const Frame& x);

  const Frame& last();

  // pop the m last elements
  void popn(int m);
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
        UPROPERTY(EditAnywhere)
        AFightInput* p1Input;
        UPROPERTY(EditAnywhere)
        AFightInput* p2Input;
        int frame;

	// Sets default values for this actor's properties
	ALogic();

private:
        bool _beginFight = false;

        bool collides(const Box &p1b, const Box &p2b, const Frame &f, int targetFrame);
        bool collides(const Hitbox &p1b, const Box &p2b, const Frame &f, int targetFrame);
        bool collides(const Box &p1b, const Hitbox &p2b, const Frame &f, int targetFrame);
        bool collides(const Hitbox &p1b, const Hitbox &p2b, const Frame &f, int targetFrame);
        float playerCollisionExtent(const Player &p, const Player &q, int targetFrame);
        void HandlePlayerBoundaryCollision(Frame &f, int targetFrame, bool doRightBoundary);
        bool IsPlayerOnLeft(const Player& p1, const Player& p2);
        bool IsP1OnLeft(const Frame& f);

        void computeFrame(int targetFrame);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
        // Reset the fight; put players back at start with full
        // health, clear inputs. Do this before beginFight().
        void reset(bool flipSpawns);

        // start and stop the actual fight logic
        void beginFight();
        void endFight();
	// Called every frame
        void FightTick();
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
