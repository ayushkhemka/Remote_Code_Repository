/////////////////////////////////////////////////////////////////////
//  Sender.cpp - Create and send HTTP style messages               //
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
The Sender module can be used to create and send HTTP style messsages
across a socket. The type of the message determines whether the client
wants to send a message or a file, and the server can use the Receiver
to react to the message. The Sender class establishes a new socket
connection using the Socket Connecter for each message to be sent.
The client has to pass in the quit message for the Sender to stop
sending any more messages.

Public Interface:
=================
Sender sndr(url);															 // set up a socket connecter at the url
msg = sndr.makeMessage(type, body, toAddr);		// make a HTTP style message
sndr.postMessage(msg);												// send the message
sndr.postFile(msg);														// send a file
sndr.start();																	// start sending messages

Build Process:
==============
Required files
- ClientHandler.h, Cpp11-BlockingQueue.h, FileSystem.h, HttpMessage.h,
itokcollection.h, Sender.h, Sockets.h, Tokenizer.h, Utilities.h,
XmlDocument.h, XmlElement.h, xmlElementParts.h, XmlParser.h
Build command
- devenv Project4ood.sln /TEST_SENDER

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#include "Sender.h"
#include "../Receiver/Receiver.h"
#include "../Logger/Logger.h"
using Show = StaticLogger<0>;
using Utils = StringHelper;

// -------------< test stub >-------------
#ifdef TEST_SENDER
int main() {
	::SetConsoleTitle("Sender Class");
	Utils::Title("Testing Sender class", '=');
	Sender sndr("localhost:8081");
	sndr.start();
	Receiver rcvr("localhost:8081");
	HttpMessage msg = sndr.makeMessage(1, "Logger,Logger.h,Sockets.h,Sockets.cpp", "localhost:8080");
	sndr.postFile(msg);
	::Sleep(100);
	msg = sndr.makeMessage(1, "Logger,Logger.cpp,Sockets.h,Sockets.cpp", "localhost:8080");
	sndr.postFile(msg);
	::Sleep(100);
	msg = sndr.makeMessage(1, "Sockets,Sockets.h,Logger.h.ver1", "localhost:8080");
	sndr.postFile(msg);
	::Sleep(100);
	msg = sndr.makeMessage(1, "Sockets,Sockets.cpp,Logger.cpp.ver2", "localhost:8080");
	sndr.postFile(msg);
	::Sleep(100);
	msg = sndr.makeMessage(5, "", "localhost:8080");
	sndr.postMessage(msg);
	::Sleep(100);
	msg = sndr.makeMessage(6, "Sockets.2016.5.3", "localhost:8080");
	sndr.postMessage(msg);
	::Sleep(100);
	msg = sndr.makeMessage(2, "Sockets.2016.5.3", "localhost:8080");
	sndr.postMessage(msg);
	::Sleep(100);
	msg = sndr.makeMessage(3, "Logger.2016.5.3", "localhost:8080");
	sndr.postMessage(msg);
	::Sleep(100);
	msg = sndr.makeMessage(7, "Logger", "localhost:8080");
	sndr.postMessage(msg);

	::Sleep(100);	
	rcvr.start();
	msg = sndr.makeMessage(0, "closeServer", "localhost:8080");
	sndr.postMessage(msg);
	msg = sndr.makeMessage(0, "quit", "localhost:8080");
	sndr.postMessage(msg);
	sndr.wait();

	std::cout << "\n\n";
	return 0;
}
#endif