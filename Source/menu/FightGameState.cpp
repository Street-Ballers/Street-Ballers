// Fill out your copyright notice in the Description page of Project Settings.


#include "FightGameState.h"

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
  //doPreRound();
  doBeginRound();
}

void AFightGameState::doPreRound() {
  if(OnPreRound.IsBound()) {
    OnPreRound.Broadcast();
  }
}

void AFightGameState::doBeginRound() {
  if(OnBeginRound.IsBound()) {
    OnBeginRound.Broadcast();
  }
}

void AFightGameState::doEndRound() {
  if(OnEndRound.IsBound()) {
    OnEndRound.Broadcast();
  }
}

void AFightGameState::doEndFight() {
  if(OnEndFight.IsBound()) {
    OnEndFight.Broadcast();
  }
}
