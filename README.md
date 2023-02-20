<h1 align="center">
    Rainbow Table
</h1>

<p align="center">
    <a href="https://en.wikipedia.org/wiki/Rainbow_table">Rainbow Table</a> attack by lenght on SHA256.
</p>

# Generator

## Prebuild

To build generator you need to go to generator folder using `cd` command (`cd generator`).

## Compilation

To compile the generator program, type `make` command that will create an executable using the makefile provided in the generator folder. If the executable already exists, you can clean the folder by typing `make clean`.

## Execution

To generate the rainbow table, type the following command in a shell `./generator --minPasswordLength=? --maxPasswordLength=? --numberChains=?` where minPasswordLength is minimal length of the password (between 6 and 10 characters), maxPasswordLength is maximal length of the password (between 6 and 10 characters) and numberChains is the number of chains to generate in the rainbow table. You can also type `./generator --help` to get informations about the different parameters to pass to the program and their meanings.

# Crack

## Prebuild

To build crack program that allows to crack passwords using rainbow table you need to go to crack folder using `cd` command (`cd crack`).

## Compilation

To build crack you need to go to crack folder using `cd` command (`cd crack`).

## Execution

To launch an attack, type the following command in a shell,`./crack --filePath=? --passwordLength=?` where filePath is the path to the file and passwordLength is the length of the password. You can also type `./generator --help` to get informations about the different parameters to pass to the program and their meanings.

# Authors

- **[etoome](https://github.com/etoome)**: primary developer
- **[Elli Makdis](https://gitlab.com/SilentElli)**: binarySearch()
- **[Hamza Zakaria Foudia](https://gitlab.com/hzfoudia)**: externalSort()
- **[Naim Boussaid](https://gitlab.com/nboussai)**: externalSort()
- **[Alan Vanderhoeven](https://gitlab.com/alvdhoev)**
