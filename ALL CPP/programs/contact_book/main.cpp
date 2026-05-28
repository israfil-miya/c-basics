#include <iostream>

int main()
{

    int choice;

    do
    {

        std::cout << "\n ----- Welcome to the Contact Book! ----- \n"
                  << "1. Add/Edit Contact\n"
                  << "2. Search Contacts\n"
                  << "3. Delete Contact\n"
                  << "4. Export to CSV\n"
                  << "0. Exit\n";
        std::cout << "Please enter your choice: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            std::cout << "You chose to add a contact.\n";
            break;
        case 2:
            std::cout << "You chose to view contacts.\n";
            break;
        case 3:
            std::cout << "You chose to delete a contact.\n";
            break;
        case 0:
            std::cout << "Exiting the Contact Book. Goodbye!\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);

    return 0;
}