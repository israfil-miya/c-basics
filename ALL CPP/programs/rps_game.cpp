#include <iostream>
#include <ctime>

class RPS
{
private:
    char choice;

public:
    RPS()
    {
        srand(static_cast<unsigned int>(time(0)));
    }

    enum Result
    {
        TIE = 0,
        WIN = 1,
        LOSE = 2
    };

    std::string toString(Result result) const
    {
        switch (result)
        {
        case TIE:
            return "It's a tie!";
        case WIN:
            return "Congratulations! You win!";
        case LOSE:
            return "Sorry, you lose!";
        default:
            return "Invalid result";
        }
    }

    char pChoice()
    {
        std::cout << "Enter your choice (R, P, S): ";
        std::cin >> choice;
        return choice;
    }

    char cChoice()
    {
        char choices[3] = {'R', 'P', 'S'};
        return choices[rand() % 3];
    }

    Result decision(char playerChoice, char computerChoice) const
    {
        if ((playerChoice == 'R' && computerChoice == 'S') ||
            (playerChoice == 'P' && computerChoice == 'R') ||
            (playerChoice == 'S' && computerChoice == 'P'))
            return WIN;

        if (playerChoice == computerChoice)
            return TIE;

        return LOSE;
    }
};

int main()
{
    std::cout << "================ Welcome to our Rock Paper Scissors game! =================\n";

    RPS game;

    char playerChoice = game.pChoice();
    std::cout << "You chose: " << playerChoice << "\n";

    char computerChoice = game.cChoice();
    std::cout << "Computer chose: " << computerChoice << "\n";

    RPS::Result result = game.decision(playerChoice, computerChoice);
    std::cout << game.toString(result) << "\n";

    std::cout << "Press Enter to exit...";

    std::cin.ignore();
    std::cin.get();
    std::cout << "================ Thank you for playing! Goodbye! =================\n";
}