#pragma once

enum class LogicMode {
  Wait, // Logic and FightInput will do nothing, not even count ticks.
        // We are in this mode before both players are connected and
        // ticking.

  Idle, // Logic and FightInput will count ticks and perform
        // synchronization, but will not do normal fight logic or take
        // input

  Fight, // Logic and FightInput will do normal fight logic and take
         // input
};
