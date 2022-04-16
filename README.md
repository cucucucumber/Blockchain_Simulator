# EE450 Programming Project
Author: Zijian Song

ID: 3971390536 

---

## Completed Tasks
- Part 1: Estabish Connection

- Part 2: CHECK WALLET and TXCOINS

- Part 3: TXLIST

- Part 4(Extra): stats

## Folder Structure
```
Project Root
│
├── src
│   ├── clientA.cpp
│   ├── clientB.cpp
│   ├── serverA.cpp
│   ├── serverB.cpp
│   ├── serverC.cpp
│   ├── serverM.cpp
│
├── inc
│   ├── constants.h (port numbers, etc)
│
├── debug (empty debug folder)
│
├── obj (empty object folder)
│
├── data (you may replace the data files)
│   ├── block1.txt
│   ├── block2.txt
│   ├── block3.txt
│ 
├── bin (executables, initially empty)
│   ├── clientA
│   ├── clientB
│   ├── serverA
│   ├── serverB
│   ├── serverC
│   ├── serverM
│
├── makefile
├── README.md 
└── .gitignore
```

## Make instruction
The simplest way to compile this project is to type 

`make`

in the command line at **project root**. 

`make` will create all the necessary folders, clean all the binaries, re-compile all the source codes from `/src`, and put the compiled executables into `/bin`.

Alternatively, you can also type `make all` as it wrote in the project instruction. But `make all` will not clean the previously compiled binaries and will not create folders if they do not exist.

## Test the project
>We need data/block1.txt, data/block2.txt, and data/block3.txt for this project to work.

- boot-up:

    At the **project root**, run

    `bin/{boot-up target}`

    to boot-up the servers or the clients. For example, we can run

    `bin/serverM`

    to boot-up the main server.

- CHECK WALLET

    After booting-up all the servers, run

    `bin/clientA <username>`

    at project root to check wallet.

- TXCOINS

    `bin/clientA <username1> <username2> <amount>`

- TXLIST

    `bin/clientA TXLIST`

    `alichain.txt` will be placed under `./data`.

- stats

    `bin/clientA stats`

## Reused Code

Part of the socket programming idioms and C++ fsteam commands were refered from the public resource (Beej, StackOverflow, GeekforGeeks). But no large block of codes was used.

## Miscellaneous

For clarification purpose and also according to the Piazza discussions, I added some more on-screen messages for TXCOINS and stats phases. 

## Idiosyncrasy

The project will definitly fail if the hard-coded ports are not available. Sometimes the zommbie process may impair this project. The project will also fail when no data files are provided or the data files are not written in correct format.
