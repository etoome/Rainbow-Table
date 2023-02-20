#pragma once

#include <string>
#include <string.h>
#include <stdio.h>
#include <cmath>

#include "constants.hpp"

std::string reduce(const char *hashed, int position, int passwordLength)
{
    int keepLength = 16;
    int base = CHAR_POLICY.length();

    char password[passwordLength + 1];
    char subhash[keepLength + 1];

    strncpy(subhash, hashed, keepLength); // no overflow attack because strncpy stop at \0
    subhash[keepLength] = '\0';
    password[passwordLength] = '\0';

    unsigned long long int exposant = std::pow(base, passwordLength);
    unsigned long long int hex_to_int = std::stoull(subhash, nullptr, keepLength);
    unsigned long long int reduced = (hex_to_int + position) % exposant;

    for (int i = 0; i < passwordLength; i++)
    {
        password[i] = CHAR_POLICY[reduced % base];
        reduced = reduced / base;
    }

    return password;
}
