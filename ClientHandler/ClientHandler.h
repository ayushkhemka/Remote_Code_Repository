/////////////////////////////////////////////////////////////////////
//  CientHandler.h - Client Handler for receiver                   //
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
This module defines the client handling functionality for the receiver
to use. It expects a receiver queue. It receives messages from the socket,
parses it to detect the type, performs necessary actions, then puts an
appropriate message inside the receiver queue. The receiver then
dequeues this queue, and if the message is quit, stops receiving any more
messages and exits.

This Client Handler is common to both the client and server.

Public Interface:
=================
BlockingQueue<T> msgQ;				// defines the message queue to use
ClientHandler cp(msgQ);				// insantiates the client handler to use the message queue
SocketListener sl(port, ver);	// defines a socket listener
sl.start(cp);									// start receiving messages and put them in msgQ

Build Process:
==============
Required files
- ClientHandler.h, Cpp11-BlockingQueue.h, FileSystem.h, HttpMessage.h,
itokcollection.h, Sender.h, Sockets.h, Tokenizer.h, Utilities.h,
XmlDocument.h, XmlElement.h, xmlElementParts.h, XmlParser.h
Build commands (one by one)
- devenv Project4ood.sln /TEST_CLIENTHANDLER
- devenv Project4ood.sln /TEST_SENDER

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../HttpMessage/HttpMessage.h"
#include "../XmlDocument/XmlDocument.h"
#include "../XmlElement/XmlElement.h"
#include "../XmlParser/XmlParser.h"
#include "../XmlElementParts/xmlElementParts.h"
#include "../Sockets/Sockets.h"
using namespace XmlProcessing;

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You need to be careful using data members of this class
//   because each client handler thread gets a reference to this 
//   instance so you may get unwanted sharing.
// - I may change the SocketListener semantics (this summer) to pass
//   instances of this class by value.
// - that would mean that all ClientHandlers would need either copy or
//   move semantics.
//

using sPtr = std::shared_ptr<AbstractXmlElement>;
class ClientHandler
{
public:
	ClientHandler(BlockingQueue<HttpMessage>& msgQ) : msgQ_(msgQ) {}
	void operator()(Socket socket);
private:
	bool connectionClosed_;
	bool dirClosed_;
	std::string dirname = "";
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string filename, size_t fileSize, Socket& socket, const std::string path);
	HttpMessage readBody(HttpMessage msg, Socket& socket);
	void saveFileServer(HttpMessage msg, Socket& socket);
	void saveFileClient(HttpMessage msg, Socket& socket);
	std::string getCurrentDate();
	HttpMessage constructMessage(HttpMessage msg);
	HttpMessage constructMessage(HttpMessage msg, std::string body);
	std::vector<std::string> findDeps(std::string str);
	std::vector<std::string> splitString(std::string str);
	XmlDocument builDocument(std::string filename, std::vector<std::string> deps);
	HttpMessage getFileList(HttpMessage msg);
	HttpMessage getFileNameList(HttpMessage msg);
	HttpMessage getFiles(HttpMessage msg, std::string type);
	size_t find(std::vector<std::string> vec, std::string str);
	std::string searchFile(std::string str);
	void updateXML(std::string package, std::string filename, std::vector<std::string> deps);
	sPtr makeFileElement(sPtr pRoot, std::string package, std::string filename, std::vector<std::string> deps);
	HttpMessage closePackage(HttpMessage msg);
	void sendMessage(HttpMessage msg);
	HttpMessage errorMessage(HttpMessage msg, std::string body);
	
	BlockingQueue<HttpMessage>& msgQ_;
};

#endif