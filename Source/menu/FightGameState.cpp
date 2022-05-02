// Fill out your copyright notice in the Description page of Project Settings.

#include "FightGameState.h"
#include "Logic.h"

void AFightGameState::ServerPlayerReady(int playerNumber) {
  switch (playerNumber) {
  case 0: p0Ready = true; break;
  case 1: p1Ready = true; break;
  }

  if (p0Ready && p1Ready) {
    ClientPlayersReady();
  }
}

void AFightGameState::ClientPlayersReady_Implementation() {
  ALogic* l = FindLogic(GetWorld());
  // TODO: we should call preRound() instead. An actor bound to the
  // preround event should then call beginRound() after its done doing
  // things like displaying a "ready...fight" banner
  //l->preRound();
  l->beginRound();
}
