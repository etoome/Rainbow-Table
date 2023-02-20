#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <vector>
#include <thread>
#include <iterator>
#include <algorithm>

#include "../shared/constants.hpp"
#include "../shared/reduce.hpp"
#include "../shared/hash.hpp"
#include "../shared/utils.hpp"

void repositionHead(std::ifstream &file, int line, int passwordLength, int deltaPosition)
{
    file.clear(); // when EOF seekg flags must be reinitialized
    file.seekg(0, file.beg);
    file.seekg((line * (2 * passwordLength + 2)) + deltaPosition);
}

std::string readLine(std::ifstream &file, int passwordLength)
{
    char candidate[passwordLength + 1];
    file.read(candidate, passwordLength);
    candidate[passwordLength] = '\0';
    return std::string(candidate);
}

std::vector<std::string> binarySearch(std::ifstream &file, std::string tailToFind, unsigned passwordLength, unsigned lineCount)
{
    std::vector<std::string> heads;

    file.clear();
    file.seekg(0, file.beg);

    int low = 0, high = lineCount - 1;
    int mid = (high + low) / 2;
    // This below check covers all cases , so need to check
    while (high != low && high >= 1 && low <= high)
    {
        repositionHead(file, mid, passwordLength, 0);
        std::string candidate = readLine(file, passwordLength);
        if (strcmp(candidate.c_str(), tailToFind.c_str()) == 0)
        { // make sure comparing strings works !!!!
            break;
        }
        else if (candidate > tailToFind)
        {
            high = mid - 1;
        }
        else if (candidate < tailToFind)
        {
            low = mid + 1;
        }
        mid = (high + low) / 2;
    }
    std::string line;
    repositionHead(file, mid, passwordLength, 0);
    std::string candidate = readLine(file, passwordLength);
    if (strcmp(tailToFind.c_str(), candidate.c_str()) == 0)
    {
        repositionHead(file, mid, passwordLength, passwordLength + 1 /*,*/);
        std::string found = readLine(file, passwordLength);
        heads.push_back(found);

        int index = mid - 1;
        if (index >= 0)
        {
            repositionHead(file, index, passwordLength, 0);
            candidate = readLine(file, passwordLength);
            while (strcmp(tailToFind.c_str(), candidate.c_str()) == 0 && index >= 0)
            {
                repositionHead(file, index, passwordLength, passwordLength + 1 /*,*/);
                found = readLine(file, passwordLength);

                heads.push_back(found);

                index--;

                repositionHead(file, index, passwordLength, 0);
                candidate = readLine(file, passwordLength);
            }
        }
        std::string candidate;
        index = mid + 1;
        if (index <= lineCount)
        {
            repositionHead(file, index, passwordLength, 0);
            candidate = readLine(file, passwordLength);
            while (strcmp(tailToFind.c_str(), candidate.c_str()) == 0 && index <= lineCount)
            {
                repositionHead(file, index, passwordLength, passwordLength + 1 /*,*/);
                found = readLine(file, passwordLength);

                heads.push_back(found);

                index++;

                repositionHead(file, index, passwordLength, 0);
                candidate = readLine(file, passwordLength);
            }
        }
    }

    return heads;
}

void findInChain(std::vector<std::string> heads, std::string tail, std::string hashToFind, int passwordLength)
{
    bool found = false;
    int head_i = 0;

    std::string blockPassword;
    std::string blockHash;

    while (!found || head_i < heads.size())
    {
        std::string head(heads[head_i]);
        blockPassword = head;
        blockHash = hash(head);

        int block_i = 0;

        while (!found && block_i < CHAIN_LENGTH)
        {
            if (blockHash == hashToFind)
            {
                found = true;
            }
            else
            {
                blockPassword = reduce(blockHash.c_str(), block_i++, passwordLength);
                blockHash = hash(blockPassword);
            }
        }

        head_i++;
    }

    if (found)
    {
        printf("%s -> %s\n", blockHash.c_str(), blockPassword.c_str());
    }
    else
    {
        printf("%s Not found!\n", blockHash.c_str());
    }
}

void crackHash(std::string hashToCrack, int passwordLength, std::string filePath)
{
    int block_i = CHAIN_LENGTH - 1;
    bool found = false;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        fprintf(stderr, "could not open %s\n try to generate it with ./generator\n", filePath.c_str());
        exit(1);
    }

    file.unsetf(std::ios_base::skipws);
    unsigned lineCount = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');

    while (!found && block_i >= 0)
    {
        std::string tempHash(hashToCrack);
        for (int i = block_i; i < CHAIN_LENGTH - 1; i++) // extremly slow
        {
            tempHash = hash(reduce(tempHash.c_str(), i, passwordLength));
        }

        std::string tail = reduce(tempHash.c_str(), CHAIN_LENGTH - 1, passwordLength);
        block_i--;

        std::vector<std::string> heads = binarySearch(file, tail, passwordLength, lineCount);

        if (!heads.empty())
        {
            found = true;
            findInChain(heads, tail, hashToCrack, passwordLength);
        }
    }
    file.close();
    if (!found)
    {
        printf("%s -> not found in rainbow table\n", hashToCrack.c_str());
    }
}

void crackHashes(std::vector<std::string> hashes, int passwordLength)
{
    std::vector<std::thread> threads;

    std::string filePath = "./rainbow_table-" + std::to_string(passwordLength) + "char.csv";

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        fprintf(stderr, "could not open %s\n", filePath.c_str());
        exit(1);
    }

    // easy tasks will free computational power for harder tasks
    // the bottleneck of over-threading and context switching are omitted
    for (std::string &hashed : hashes)
    {
        threads.push_back(std::thread(crackHash, hashed, passwordLength, filePath));
        // threads.back().join(); // debug
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }
}

int main(int argc, char const *argv[])
{
    std::string filePath;
    int passwordLength = 0;

    std::string cmd;
    std::string param;
    std::string value;

    for (int argc_i = 1; argc_i < argc; ++argc_i)
    {
        cmd = argv[argc_i];

        if (cmd.compare(std::string("--help")) == 0)
        {
            printf(
                "--passwordLength=[int] : lenght of all passwords in filePath\n"
                "--filePath=[string] : path of a file with 1 hash/line (64 char each)\n");
        }

        param = cmd.substr(0, cmd.find("="));
        value = cmd.substr(cmd.find("=") + 1, -1);

        if (param.compare(std::string("--filePath")) == 0)
        {
            filePath = value;
        }
        else if (param.compare(std::string("--passwordLength")) == 0)
        {
            passwordLength = std::stoi(value);
        }
    }

    if (passwordLength < 1)
    {
        fprintf(stderr, "--passwordLength invalid\n");
        exit(1);
    }

    if (filePath.empty())
    {
        fprintf(stderr, "--filePath invalid\n");
        exit(1);
    }

    std::ifstream input;
    if (!filePath.empty())
    {
        input.open(filePath);
        if (!input.is_open())
        {
            fprintf(stderr, "could not open %s\n", filePath.c_str());
            exit(1);
        }
    }

    std::vector<std::string> hashes;

    std::string line;
    while (std::getline(input, line))
    {
        hashes.push_back(line);
    }

    crackHashes(hashes, passwordLength);

    return 0;
}
