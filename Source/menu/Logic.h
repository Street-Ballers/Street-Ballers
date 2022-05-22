// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Action.h"
#include "FightInput.h"
#include "FightGameState.h"
#include "LogicMode.h"
#include "Logic.generated.h"

// Important fight sequence events. It should be possible to bind to
// these events from blueprints.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndFightDelegate);

class Player {
public:
  FVector pos;
  HAction action;
  bool isFacingRight;
  int actionStart;
  int health;
  int hitstun = 0;
  float knockdownVelocity;
  int actionNumber = 0; // used to prevent a lingering hitbox from hitting every frame

  Player(FVector pos, HAction action): pos(pos), action(action), actionStart(0), health(100) {};
  Player() {};

  void startNewAction(int frame, HAction newAction, bool isOnLeft);
  void TryStartingNewAction(int frame, AFightInput& input, bool isOnLeft);
  float collidesWithBoundary(float boundary, bool isRightBound, int targetFrame);
  void doDamagedAction(int frame);
  void doBlockAction(int frame);
  void doKdAction(int frame, bool isOnLeft, float knockdownDistance);
  void doThrownAction(int frame, bool isOnLeft, float knockdownDistance, HAction newAction, Player& q);
  void doMotion(int frame);
};

class Frame {
public:
  Player p1;
  Player p2;
  int hitstop = 0; // number of frames of hitstop left
  float pushbackPerFrame;
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
#define PREROUND_TIME 60
#define ENDROUND_TIME 60
        // set to true to skip the preround
        UPROPERTY(EditAnywhere)
        bool skipPreRound;
        // set to true to always rollback the maximum amount for
        // testing purposes
        UPROPERTY(EditAnywhere)
        bool alwaysRollback;

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

        UPROPERTY(EditAnywhere)
        AFightInput* p1Input;
        UPROPERTY(EditAnywhere)
        AFightInput* p2Input;

        // Fight sequence events
        UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
        FOnPreRoundDelegate OnPreRound;
        UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
        FOnBeginRoundDelegate OnBeginRound;
        UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
        FOnEndRoundDelegate OnEndRound;
        UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
        FOnEndFightDelegate OnEndFight;

        // Sets default values for this actor's properties
        ALogic();

private:
        int maxRollback = 10; // keep around 10 frames or so for rollback
        RingBuffer frames;
        int frame;
        int rollbackStopFrame; // When starting a new round, we don't
                               // want to rollback past the first
                               // frame.

        enum LogicMode mode;
        bool inPreRound; // setting this to true will cause Tick() to
                         // count forward 30 frames rounded to the
                         // nearest 15 frames, and then call
                         // beginRound().
        bool inEndRound;
        int roundStartFrame;
        void setMode(enum LogicMode);

        // Reset the fight; put players back at start with full
        // health, clear inputs and rollback buffer.
        void reset(bool flipSpawns);

        // a bunch of convenience functions for computeFrame()
        bool collides(const Box &p1b, const Box &p2b, const Frame &f, int targetFrame);
        bool collides(const Hitbox &p1b, const Box &p2b, const Frame &f, int targetFrame);
        bool collides(const Box &p1b, const Hitbox &p2b, const Frame &f, int targetFrame);
        bool collides(const Hitbox &p1b, const Hitbox &p2b, const Frame &f, int targetFrame);
        float playerCollisionExtent(const Player &p, const Player &q, int targetFrame);
        void HandlePlayerBoundaryCollision(Frame &f, int targetFrame, bool doRightBoundary);
        bool IsPlayerOnLeft(const Player& p1, const Player& p2);
        bool IsP1OnLeft(const Frame& f);

        void computeFrame(int targetFrame);

        // Called every frame
        void FightTick();

protected:
        // Called when the game starts or when spawned
        virtual void BeginPlay() override;

public:
        // reset() and Enter FightMode::Idle mode. Trigger OnPreRound
        // event.
        UFUNCTION (BlueprintCallable, Category="Fight Sequence")
        void preRound();
        // Enter FightMode::Fight mode. Trigger OnBeginRound event.
        UFUNCTION (BlueprintCallable, Category="Fight Sequence")
        void beginRound();
        // Go to FightMode::Idle mode. Trigger OnEndRound event.
        UFUNCTION (BlueprintCallable, Category="Fight Sequence")
        void endRound();
        // Go to FightMode::Wait. Trigger OnEndFight event.
        UFUNCTION (BlueprintCallable, Category="Fight Sequence")
        void endFight();

        virtual void Tick(float DeltaTime) override;

        // Getters to get values for updating other actors
        const Player& getPlayer1();
        const Player& getPlayer2();
        // player number is 0-indexed for this function
        const Player& getPlayer(int playerNumber);

        // These getters are not methods on Player because I don't
        // want to turn Player into a UObject and increase the size of
        // the data we need to save every frame.
        UFUNCTION (BlueprintCallable, Category="Player")
        FVector playerPos(int playerNumber);
        UFUNCTION (BlueprintCallable, Category="Player")
        bool playerIsFacingRight(int playerNumber);
        UFUNCTION (BlueprintCallable, Category="Player")
        int playerHealth(int playerNumber);
        UFUNCTION (BlueprintCallable, Category="Player")
        int playerAnimation(int playerNumber);
        UFUNCTION (BlueprintCallable, Category="Player")
        int playerFrame(int playerNumber);
};

static inline ALogic* FindLogic(UWorld *world) {
  TActorIterator<ALogic> i (world);
  ALogic *l = Cast<ALogic>(*i);
  if (l) {
    // UE_LOG(LogTemp, Display, TEXT("ALogic found!"));
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("ALogic NOT found!"));
  }
  return l;
}
