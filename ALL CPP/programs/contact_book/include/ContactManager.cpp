#include "ContactManager.h"
#include <iostream>
#include <fstream>
#include <limits>

void ContactManager::clearInputBuffer() const
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool ContactManager::addOrEditContact()
{
    std::string name, email, phone;
    bool contact_update = false;

    std::cout << "Enter contact name: ";
    clearInputBuffer();
    std::getline(std::cin, name);

    if (contactBook.find(name) != contactBook.end())
    {
        std::cout << "Contact already exists. Do you want to edit it? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y')
        {
            return false;
        }
        contact_update = true;
        clearInputBuffer();
    }

    std::cout << "Enter email" << (contact_update ? " (\"-\" to skip): " : ": ");
    std::getline(std::cin, email);

    std::cout << "Enter phone" << (contact_update ? " (\"-\" to skip): " : ": ");
    std::getline(std::cin, phone);

    if (email != "-" || !contact_update)
    {
        contactBook[name].email = email;
    }

    if (phone != "-" || !contact_update)
    {
        contactBook[name].phone = phone;
    }

    std::cout << "Contact " << (contact_update ? "updated" : "added") << " successfully.\n";
    return true;
}

bool ContactManager::deleteContact()
{
    std::string name;
    std::cout << "Enter contact name: ";
    clearInputBuffer();
    std::getline(std::cin, name);

    auto it = contactBook.find(name);
    if (it != contactBook.end())
    {
        std::cout << "Are you sure you want to delete this contact? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y')
        {
            return false;
        }

        contactBook.erase(it);
        std::cout << "Deleted the contact successfully!\n";
        return true;
    }

    std::cout << "Couldn't find the contact.\n";
    return false;
}

bool ContactManager::searchContact() const
{
    std::string name;
    std::cout << "Enter contact name: ";
    clearInputBuffer();
    std::getline(std::cin, name);

    auto it = contactBook.find(name);
    if (it != contactBook.end())
    {
        std::cout << "\nName: " << it->first
                  << "\nEmail: " << it->second.email
                  << "\nPhone: " << it->second.phone << "\n";
        return true;
    }

    std::cout << "Couldn't find the contact.\n";
    return false;
}

bool ContactManager::exportToCSV() const
{
    if (contactBook.empty())
    {
        std::cout << "Contact book is empty!\n";
        return false;
    }

    std::ofstream outFile("contacts.csv");
    outFile << "Name,Email,Phone\n";

    for (const auto &[name, details] : contactBook)
    {
        outFile << name << ',' << details.email << ',' << details.phone << "\n";
    }
    std::cout << "Contacts exported to contacts.csv!\n";
    return true;
}