#pragma once

#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <string>
#include <unistd.h>

#include <iostream>

/* 85% of ram availble for program, in KB*/
long double getFreeRam()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return ((pages * page_size) >> 10) * 0.85; // in kB.
}

unsigned getFileLines(std::ifstream &file)
{
    file.unsetf(std::ios_base::skipws);
    return (std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n'));
}

bool canFileBeInRam(std::ifstream &file, int passwordLength)
{
    unsigned fileLines = getFileLines(file);
    int lineMemorySize = passwordLength + 1 /*,*/ + passwordLength + 1 /*\n*/;
    file.clear();
    file.seekg(0, file.beg);
    return ((fileLines * lineMemorySize) / 1024.0) <= getFreeRam();
}
