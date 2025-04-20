# Minimal Shell For Learning

## Description

Descripbed is a minimal shell that aims to demonstrate the multi-process nature of shells and their basic underlying implementation. The inital shell written merely had some built in functions and the ability to execute a binary on the current users path. Beyond learning I have decided to expand the shell to act as a personal interface that encapsulates projects and features together into a command line shell. This currently includes a very basic HTTP server and previously mentioned built in commands. Hopefully this will expand to a more full on HTTP server and built in commands to setup python and C/C++ project folders for me.

## Install and Run

Currently the project comprises of the single lsh.cpp file and the http.hpp file. This will need to be compiled with the only dependancy being the http implementation. Plan for cmake in the future.

## How to Use

The shell does not have a standalone window implementation (might try that.. maybe...) so all that needs to be done is run the executable from a cli and the shells loop will run, waiting for input. The first and most important built in command is the help command. Running this will give possible commands and directions. If a binary exits in the users path and this is executed in the shell it will spawn a new process, passing any command-line arguments to new process. The shell will wait for the child proccess to finish before returning to the shell proccess.

## Future Features

- Directory checking / directory creation for the structure of the HTTP server to place documents to serve
- Explore the idea of some basic automated project setups for python and C/C++
