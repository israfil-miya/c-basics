#pragma once
#include <string>

class RPSGame
{
public:
    enum class Move
    {
        ROCK,
        PAPER,
        SCISSORS
    };
    enum class Result
    {
        TIE,
        WIN,
        LOSE
    };

    Result determineWinner(Move player, Move computer) const;
    Move generateComputerMove() const;

    static std::string moveToString(Move move);
    static std::string resultToString(Result result);
};

class GameUI
{
public:
    static void run();
};