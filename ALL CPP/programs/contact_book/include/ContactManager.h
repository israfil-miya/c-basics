#pragma once

#include <unordered_map>
#include <string>

class ContactManager
{
private:
    struct Contact
    {
        std::string email;
        std::string phone;
    };

    std::unordered_map<std::string, Contact> contactBook;

    void clearInputBuffer() const;

public:
    bool addOrEditContact();
    bool deleteContact();
    bool searchContact() const;
    bool exportToCSV() const;
};