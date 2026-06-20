#include <iostream>
#include "include/ContactManager.h"

int main()
{
    ContactManager manager;
    int choice;

    do
    {
        std::cout << "\n\n ----- Welcome to the Contact Book! ----- \n"
                  << "1. Add/Edit Contact\n"
                  << "2. Search Contacts\n"
                  << "3. Delete Contact\n"
                  << "4. Export to CSV\n"
                  << "0. Exit\n"
                  << "Please enter your choice: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            manager.addOrEditContact();
            break;
        case 2:
            manager.searchContact();
            break;
        case 3:
            manager.deleteContact();
            break;
        case 4:
            manager.exportToCSV();
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