/////////////////////////////////////////////////////////////////////
//  Cient.h - Client code												                   //
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
This module acts as the client code for the project. It makes messages
for all types of messages, and gives back the reply.

Public Interface:
=================
Client c(clientUrl, serverUrl);			// establish urls for the client and server
c.start();													// run tests

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

#ifndef CLIENT_H
#define CLIENT_H

#include "../Sender/Sender.h"
#include "../Receiver/Receiver.h"

class Client {
public:
	Client(std::string clientUrl, std::string serverUrl);
	void start(int timeout);
private:
	void testType0(Sender& sndr, int timeout);
	void testType1(Sender& sndr, int timeout);
	void testType3(Sender& sndr, int timeout);
	void testType5(Sender& sndr, int timeout);
	void testType6(Sender& sndr, int timeout);
	void testType7(Sender& sndr, int timeout);

	std::string localUrl;
	std::string remoteUrl;
};
#endif