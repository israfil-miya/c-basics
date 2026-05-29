#include "RPSGame.h"
#include <iostream>
#include <random>
#include <limits>
#include <cctype>

RPSGame::Result RPSGame::determineWinner(Move player, Move computer) const
{
    if (player == computer)
        return Result::TIE;

    if ((player == Move::ROCK && computer == Move::SCISSORS) ||
        (player == Move::PAPER && computer == Move::ROCK) ||
        (player == Move::SCISSORS && computer == Move::PAPER))
    {
        return Result::WIN;
    }

    return Result::LOSE;
}

RPSGame::Move RPSGame::generateComputerMove() const
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);

    return static_cast<Move>(dis(gen));
}

std::string RPSGame::moveToString(Move move)
{
    switch (move)
    {
    case Move::ROCK:
        return "Rock";
    case Move::PAPER:
        return "Paper";
    case Move::SCISSORS:
        return "Scissors";
    default:
        return "Unknown";
    }
}

std::string RPSGame::resultToString(Result result)
{
    switch (result)
    {
    case Result::TIE:
        return "It's a tie!";
    case Result::WIN:
        return "Congratulations! You win!";
    case Result::LOSE:
        return "Sorry, you lose!";
    default:
        return "Invalid result";
    }
}

void GameUI::run()
{
    RPSGame game;
    char choice;
    bool keepPlaying = true;

    std::cout << "================ Welcome to our Rock Paper Scissors game! =================\n";

    while (keepPlaying)
    {
        std::cout << "\nEnter your choice (R = Rock, P = Paper, S = Scissors) or Q to Quit: ";
        std::cin >> choice;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        choice = std::toupper(choice);

        if (choice == 'Q')
        {
            keepPlaying = false;
            continue;
        }

        RPSGame::Move playerMove;
        if (choice == 'R')
            playerMove = RPSGame::Move::ROCK;
        else if (choice == 'P')
            playerMove = RPSGame::Move::PAPER;
        else if (choice == 'S')
            playerMove = RPSGame::Move::SCISSORS;
        else
        {
            std::cout << "Invalid input. Please try again.\n";
            continue;
        }

        RPSGame::Move computerMove = game.generateComputerMove();

        std::cout << "You chose: " << RPSGame::moveToString(playerMove) << "\n";
        std::cout << "Computer chose: " << RPSGame::moveToString(computerMove) << "\n";

        RPSGame::Result result = game.determineWinner(playerMove, computerMove);
        std::cout << RPSGame::resultToString(result) << "\n";
    }

    std::cout << "================ Thank you for playing! Goodbye! =================\n";
}