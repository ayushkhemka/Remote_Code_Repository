/////////////////////////////////////////////////////////////////////
//  Receiver.h - Establish receiver for client and server          //
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
This module defines the receiver for the client and server to use.
When the client or server establishes the receiver, it starts to read
messages for it, setting up the Socket Listener for the url specified.

The message processing is done by the Client Handler so the establishing
module need not do anything. Since the receiver doesn't quit until the
listener receives a closeServer message, further messages cannot be sent
until the Socket Listener quits reading messages.

Public Interface:
=================
Receiver rcvr(url);				// start listening for messages on the socket
rcvr.start();							// start receiving messages

Build Process:
==============
Required files
- ClientHandler.h, Cpp11-BlockingQueue.h, FileSystem.h, HttpMessage.h,
itokcollection.h, Logger.h, Sockets.h, Tokenizer.h, Utilities.h,
XmlDocument.h, XmlElement.h, xmlElementParts.h, XmlParser.h
Build command
- devenv Project4ood.sln /TEST_RECEIVER

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#ifndef RECEIVER_H
#define RECEIVER_H

#include "../ClientHandler/ClientHandler.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Sockets/Sockets.h"

class Receiver {
public:
	Receiver(std::string url);
	void start();
private:
	BlockingQueue<HttpMessage> rcvrQ;
	std::string localUrl;
};

#endif