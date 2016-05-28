/////////////////////////////////////////////////////////////////////
//  TestExec.cpp - Test Executive									                 //
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
This module acts as the test executive for CSE 687 Pr #4 Sp16

Build Process:
==============
Required files
- ClientHandler.h, Cpp11-BlockingQueue.h, FileSystem.h, HttpMessage.h,
itokcollection.h, Sender.h, Sockets.h, Tokenizer.h, Utilities.h,
XmlDocument.h, XmlElement.h, xmlElementParts.h, XmlParser.h
Build commands (one by one)
- devenv Project4ood.sln /TEST_TESTEXEC

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#include "../Client/Client.h"
#include "../ClientHandler/ClientHandler.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../HttpMessage/HttpMessage.h"
#include "../FileSystem/FileSystem.h"
#include "../Receiver/Receiver.h"
#include "../Sender/Sender.h"
#include "../Server/Server.h"
#include "../Sockets/Sockets.h"
#include "../Utilities/Utilities.h"
using namespace Utilities;
using Utils = StringHelper;

#ifdef TEST_TESTEXEC

int main() {
	Utils::Title("Demonstrating Requirements for Pr #4", '=');
	Utils::Title("Start 2 clients and 1 server");
	putline();

	Client c1("localhost:8081", "localhost:8080");
	Server s("localhost:8080");

	std::thread t1([&]() { c1.start(100); });
	s.start();

	t1.join();
	
	std::cout << "\n\n";
	return 0;
}

#endif