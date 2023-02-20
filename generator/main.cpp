#include <thread>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <bits/stdc++.h>
#include <stdio.h>
#include <algorithm>
#include <sys/sysinfo.h>

#include "constants.hpp"

#include "../shared/constants.hpp"
#include "../shared/hash.hpp"
#include "../shared/reduce.hpp"
#include "../shared/utils.hpp"

struct Chain
{
    std::string head;
    std::string tail;
};

std::string generatePassword(int passwordLength)
{
    const int policy_length = CHAR_POLICY.length();
    char str[passwordLength + 1];

    for (int i = 0; i < passwordLength; i++)
    {
        str[i] = CHAR_POLICY[rand() % (policy_length - 1)];
    }
    str[passwordLength] = '\0';

    return std::string(str);
}

Chain *generateChain(int passwordLength)
{
    std::string head = generatePassword(passwordLength);

    std::string tempPassword(head);
    std::string tempHash;

    for (int chain_i = 0; chain_i < CHAIN_LENGTH; chain_i++)
    {
        tempHash = hash(tempPassword);
        tempPassword = reduce(tempHash.c_str(), chain_i, passwordLength);
    }

    return new Chain({head, tempPassword});
}

void writeToFile(std::string filePath, std::vector<Chain *> &chains)
{
    std::ofstream file;
    file.open(filePath, std::ios::app);
    if (!file.is_open())
    {
        fprintf(stderr, "could not open %s\n", filePath);
        exit(1);
    }

    for (Chain *chain : chains)
    {
        file << chain->tail << "," << chain->head << std::endl;
        delete chain;
    }

    file.close();
}

void generateChainBlock(std::string filePath, int numberChains, int passwordLength)
{
    std::vector<Chain *> chains;

    for (int chain_i = 0; chain_i < numberChains; chain_i++)
    {
        Chain *chain = generateChain(passwordLength);

        chains.push_back(chain);
    }

    writeToFile(filePath, chains);
}

void generateChains(std::string filePath, int numberChains, int passwordLength, long double availableRam)
{
    int numberChainsPerBlock = availableRam / (sizeof(Chain) / 1024.0);
    int numberBlocks = numberChains % numberChainsPerBlock == 0 ? numberChains / numberChainsPerBlock : numberChains / numberChainsPerBlock + 1;

    for (int block_i = 0; block_i < numberBlocks; block_i++)
    {
        int numberChainsInBlock = numberChains / numberBlocks + (block_i < numberChains % numberBlocks);
        generateChainBlock(filePath, numberChainsInBlock, passwordLength);
    }
}

std::string getFileLine(std::string filePath, int index, int passwordLength)
{
    std::ifstream file(filePath);
    char candidat[(passwordLength * 2) + 2];
    file.seekg((index) * (2 * passwordLength + 2));
    file.read(candidat, 2 * (passwordLength) + 1);
    candidat[(passwordLength * 2) + 1] = '\0';
    return candidat;
}

int mergeSort(std::string firstFilePath, std::string secondFilePath, std::string saveFilePath, int passwordLength)
{
    std::ofstream saveFile;
    saveFile.open(saveFilePath, std::ios::app);

    std::string lineFirstFile;
    std::string lineSecondFile;
    int i = 0, j = 0;
    int counterLines = 0;

    unsigned long int numberChainsFirstFile = stoul((firstFilePath.substr(firstFilePath.find(".chain") + 6, -1)), nullptr, 10);
    unsigned long int numberChainsSecondFile = stoul((secondFilePath.substr(secondFilePath.find(".chain") + 6, -1)), nullptr, 10);

    while (i < numberChainsFirstFile && j < numberChainsSecondFile)
    {
        lineFirstFile = getFileLine(firstFilePath, i, passwordLength);
        lineSecondFile = getFileLine(secondFilePath, j, passwordLength);
        if (lineFirstFile.compare(lineSecondFile) < 0)
        {
            saveFile << lineFirstFile << std::endl;
            i++;
        }
        else if (lineFirstFile.compare(lineSecondFile) == 0)
        {                                           // if same tail and same head.
            saveFile << lineFirstFile << std::endl; // store one of the two chains.
            i++;
            j++;
        }
        else
        {
            saveFile << lineSecondFile << std::endl;
            j++;
        }
        counterLines++;
    }

    // Store remaining chains of first file.
    while (i < numberChainsFirstFile)
    {
        lineFirstFile = getFileLine(firstFilePath, i, passwordLength);
        saveFile << lineFirstFile << std::endl;
        i++;
        counterLines++;
    }

    // Store remaining chains of second file.
    while (j < numberChainsSecondFile)
    {
        lineSecondFile = getFileLine(secondFilePath, j, passwordLength);
        saveFile << lineSecondFile << std::endl;
        j++;
        counterLines++;
    }
    saveFile.close();

    return counterLines;
}

bool enoughRamForOneMoreChain()
{
    return getFreeRam() >= ((sizeof(Chain) / 1024.0) * 1.2); // kB
}

bool compareTails(Chain *firstChain, Chain *secondChain)
{
    return firstChain->tail < secondChain->tail;
}

void externalSort(std::string filePath, int numberCore, int passwordLength)
{
    std::vector<Chain *> chains;
    std::vector<std::string> tempFilesMerge;
    std::string chain;
    std::string sorted;
    std::string tempFilePath;
    int counterChains = 0, counterFiles = 0, sortedFileIndex = 0;
    int numberLines;

    for (int i = 0; i < numberCore; i++)
    { // for each file of thread.
        std::string coreFilePath = filePath + std::to_string(i);
        std::ifstream file(coreFilePath);
        while (std::getline(file, chain))
        {
            chains.push_back(new Chain({chain.substr(chain.find(",") + 1, -1), chain.substr(0, chain.find(","))}));
            counterChains++;
            if (!enoughRamForOneMoreChain() || file.peek() == EOF)
            { // if there is no more space to add one more chain or we read the last line of the file.
                tempFilePath = coreFilePath + ".file" + std::to_string(counterFiles) + ".chain" + std::to_string(counterChains);
                std::sort(chains.begin(), chains.end(), compareTails); // to sort the vector.
                writeToFile(tempFilePath, chains);
                tempFilesMerge.push_back(tempFilePath);
                chains.clear();
                counterChains = 0;
                counterFiles++;
            }
        }

        file.close();
        remove(coreFilePath.c_str()); // delete the file.

        counterFiles = 0;
        counterChains = 0;
    }

    // merge files into one big file.
    while (tempFilesMerge.size() > 1)
    {
        sorted = "sorted" + std::to_string(sortedFileIndex++) + ".chain";
        numberLines = mergeSort(tempFilesMerge[0], tempFilesMerge[1], sorted, passwordLength);
        rename(sorted.c_str(), (sorted + std::to_string(numberLines)).c_str());
        remove(tempFilesMerge[0].c_str());
        remove(tempFilesMerge[1].c_str()); // remove merged files from the vector.
        tempFilesMerge.erase(tempFilesMerge.begin());
        tempFilesMerge.erase(tempFilesMerge.begin());
        tempFilesMerge.push_back(sorted + std::to_string(numberLines));
    }
    rename(tempFilesMerge.at(0).c_str(), filePath.c_str());
    tempFilesMerge.clear();
}

void generateRainbowTable(int numberChains, int passwordLength)
{
    unsigned int number_core = std::thread::hardware_concurrency();
    if (number_core < 4)
        number_core = 4;

    if (number_core > numberChains)
        number_core = numberChains;

    // number_core = 1; // debug

    std::thread threads[number_core];

    std::string filePath = "./rainbow_table-" + std::to_string(passwordLength) + "char.csv";

    long double freeRam = getFreeRam();

    for (int core_i = 0; core_i < number_core; core_i++)
    {
        threads[core_i] = std::thread(generateChains, (filePath + std::to_string(core_i)), numberChains / number_core + (core_i < numberChains % number_core), passwordLength, freeRam / number_core);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    externalSort(filePath, number_core, passwordLength);
}

void generateRainbowTables(int minPasswordLength, int maxPasswordLength, int numberChains)
{
    for (int passwordLength = minPasswordLength; passwordLength <= maxPasswordLength; passwordLength++)
    {
        generateRainbowTable(numberChains, passwordLength);
    }
}

int main(int argc, char const *argv[])
{
    int minPasswordLength = MIN_PASSWORD_LENGTH;
    int maxPasswordLength = MAX_PASSWORD_LENGTH;
    int numberChains = NUMBER_CHAINS;

    std::string cmd;
    std::string param;
    unsigned long long value;

    for (int argc_i = 1; argc_i < argc; ++argc_i)
    {
        cmd = argv[argc_i];

        if (cmd.compare(std::string("--help")) == 0)
        {
            printf("--minPasswordLength=[int] : minimum password length\n"
                   "--maxPasswordLength=[int] : maximum password length\n"
                   "--numberChains=[int] : number of chains to generate (per length)\n");
            exit(0);
        }

        param = cmd.substr(0, cmd.find("="));
        value = std::stoull(cmd.substr(cmd.find("=") + 1, cmd.size()));

        if (param.compare(std::string("--minPasswordLength")) == 0)
        {
            minPasswordLength = value;
        }
        else if (param.compare(std::string("--maxPasswordLength")) == 0)
        {
            maxPasswordLength = value;
        }
        else if (param.compare(std::string("--numberChains")) == 0)
        {
            numberChains = value;
        }
    }

    generateRainbowTables(minPasswordLength, maxPasswordLength, numberChains);

    return 0;
}
