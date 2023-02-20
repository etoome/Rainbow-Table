#pragma once

#include <string>

#include "../utils/sha256.h"

std::string hash(std::string password)
{
    return sha256(password);
}
