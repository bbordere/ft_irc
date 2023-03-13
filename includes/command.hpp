#pragma once
#include "irc.hpp"
#include "const.hpp"

typedef void* (*ptrFonction)(std::vector<std::string>);

void*	capCmd(std::vector<std::string>);
void*	nickCmd(std::vector<std::string>);
void*	userCmd(std::vector<std::string>);
void*	operCmd(std::vector<std::string>);
void*	modeCmd(std::vector<std::string>);
void*	quitCmd(std::vector<std::string>);
void*	squitCmd(std::vector<std::string>);
void*	joinCmd(std::vector<std::string>);
void*	killCmd(std::vector<std::string>);
void*	kickCmd(std::vector<std::string>);
void*	errorCmd(std::vector<std::string>);
void*	partCmd(std::vector<std::string>);
void*	privmsgCmd(std::vector<std::string>);
void*	noticeCmd(std::vector<std::string>);
void*	awayCmd(std::vector<std::string>);
void*	dieCmd(std::vector<std::string>);
void*	restartCmd(std::vector<std::string>);