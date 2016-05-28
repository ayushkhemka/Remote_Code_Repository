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

#include "Client.h"
#include "../Utilities/Utilities.h"
#include <iostream>

using namespace Utilities;
using Utils = StringHelper;

// ----------< constructor initializes the urls >--------------
Client::Client(std::string clientUrl, std::string serverUrl) : localUrl(clientUrl), remoteUrl(serverUrl) {}

// ----------< test messages of type 0 >------------
void Client::testType0(Sender& sndr, int timeout) {
	Utils::title("Sending 5 test messages");
	putline();
	HttpMessage message;
	for (int i = 1; i <= 5; i++) {
		std::string body = "Test Message #" + i;
		message = sndr.makeMessage(0, body, remoteUrl);
		sndr.postMessage(message);
		::Sleep(timeout);
	}
}

// ----------< test messages of type 1 >------------
void Client::testType1(Sender& sndr, int timeout) {
	HttpMessage message;
	Utils::title("Sending package Logger");
	putline();
	HttpMessage msg = sndr.makeMessage(1, "Logger,Logger.h,Sockets.h,Sockets.cpp", remoteUrl);
	sndr.postFile(msg);
	::Sleep(timeout);
	msg = sndr.makeMessage(1, "Logger,Logger.cpp,Sockets.h,Sockets.cpp", remoteUrl);
	sndr.postFile(msg);
	::Sleep(timeout);

	Utils::title("Sending package Sockets");
	putline();
	msg = sndr.makeMessage(1, "Sockets,Sockets.h,Logger.h.ver1", remoteUrl);
	sndr.postFile(msg);
	::Sleep(timeout);
	msg = sndr.makeMessage(1, "Sockets,Sockets.cpp,Logger.cpp.ver2", remoteUrl);
	sndr.postFile(msg);
	std::cout << "-------< please verify the XML files created for these two packages >-------\n";
	::Sleep(timeout);

}

// ----------< test messages of type 5 >------------
void Client::testType5(Sender& sndr, int timeout) {
	HttpMessage message;
	Utils::title("Requesting list of packages from server");
	putline();
	HttpMessage msg = sndr.makeMessage(5, "", remoteUrl);
	sndr.postMessage(msg);
	::Sleep(timeout);
}

// ----------< test messages of type 6 >------------
void Client::testType6(Sender& sndr, int timeout) {
	HttpMessage message;
	Utils::title("Requesting list of files in the Socket package");
	putline();
	HttpMessage msg = sndr.makeMessage(6, "Sockets.2016.5.6", remoteUrl);
	sndr.postMessage(msg);
	::Sleep(timeout);
}

// ----------< test messages of type 7 >------------
void Client::testType7(Sender& sndr, int timeout) {
	HttpMessage message;
	Utils::title("Closing Logger package");
	putline();
	HttpMessage msg = sndr.makeMessage(7, "Logger", remoteUrl);
	sndr.postMessage(msg);
	::Sleep(timeout);
}

// ----------< test messages of type 3 >------------
void Client::testType3(Sender& sndr, int timeout) {
	HttpMessage message;
	Utils::title("Requesting package Logger with all of its dependencies");
	putline();
	HttpMessage msg = sndr.makeMessage(3, "Logger.2016.5.6", remoteUrl);
	sndr.postMessage(msg);
	::Sleep(timeout);
}

// -----------< start client testing >---------------
void Client::start(int timeout) {
	std::string msg = "Starting client at address " + localUrl;
	Utils::Title(msg, '=');
	putline();

	Sender sndr(localUrl);
	sndr.start();
	Receiver rcvr(localUrl);
	HttpMessage message;
	testType0(sndr, timeout);
	putline();
	testType1(sndr, timeout);
	putline();
	testType5(sndr, timeout);
	putline();
	testType6(sndr, timeout);
	putline();
	testType7(sndr, timeout);
	putline();
	testType3(sndr, timeout);
	putline();
	rcvr.start();
	message = sndr.makeMessage(0, "closeServer", remoteUrl);
	sndr.postMessage(message);
	putline();
	message = sndr.makeMessage(0, "quit", remoteUrl);
	sndr.postMessage(message);
	sndr.wait();
}

#ifdef TEST_CLIENT

int main() {

	Utils::Title("Testing Client", '=');
	Client c1("localhost:8081", "localhost:8080");
	c1.start(100);

	std::cout << "\n\n";
	return 0;
}

#endif