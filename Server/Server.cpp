/////////////////////////////////////////////////////////////////////
//  Server.h - Server code												                 //
//  ver 1.0                                                        //
//                                                                 //
//  Language:      Visual C++ 2015, Update 2                       //
//  Platform:      Lenovo g505s AMD A8, Windows 10 Ultimate        //
//  Application:   CSE 687 Project #4, Sp16			                   //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
//  Author:        Ayush Khemka, Syracuse University							 //
//                 538044584, aykhemka@syr.edu				             //
/////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This module acts as the server code for the project. It receives messages
for all types of messages, and gives back the reply.

Public Interface:
=================
Server s(serverUrl);		// establish urls for the client and server
s.start();							// run tests

Build Process:
==============
Required files
- ClientHandler.h, Cpp11-BlockingQueue.h, FileSystem.h, HttpMessage.h,
itokcollection.h, Sender.h, Sockets.h, Tokenizer.h, Utilities.h,
XmlDocument.h, XmlElement.h, xmlElementParts.h, XmlParser.h
Build commands (one by one)
- devenv Project4ood.sln /TEST_CLIENT
- devenv Project4ood.sln /TEST_SERVER

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#include "Server.h"
#include "../Utilities/Utilities.h"
using namespace Utilities;
using Utils = StringHelper;

Server::Server(std::string url) { remoteUrl = url; }

void Server::start() {
	std::string msg = "Start Server on URL " + remoteUrl;
	Utils::Title(msg, '=');
	Receiver rcvr(remoteUrl);
	rcvr.start();
}

#ifdef TEST_SERVER

int main() {
	Server s("localhost:8080");
	s.start();

	std::cout << "\n\n";
	return 0;
}

#endif