/////////////////////////////////////////////////////////////////////
//  Sender.h - Create and send HTTP style messages                 //
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

#ifndef SENDER_H
#define SENDER_H

#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <unordered_map>
#include "../FileSystem/FileSystem.h"
#include "../Sockets/Sockets.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../HttpMessage/HttpMessage.h"
#include "../Utilities/Utilities.h"
using namespace Utilities;

using EndPoint = std::string;
using Queue = BlockingQueue<HttpMessage>;

class Sender {
public:

	Sender(const Sender& s) = delete; // disable copying

	// ------------< constructor sets the localUrl to start sending messages from >---------------
	Sender(std::string ep) {
		localUrl = ep;
	}

	// ---------------< post a message by enqueueing in sender queue >------------
	bool postMessage(HttpMessage msg) {
		sndrQueue.enQ(msg);
		return true;
	}

	// -------------< start sending messages >-----------------
	void start() {
		std::function<void()> threadProc = [&]() {
			while (true) {
				HttpMessage msg = sndrQueue.deQ();
				SocketSystem ss;
				SocketConnecter si;
				remoteUrl = msg.findValue("toAddr");
				std::string addr = remoteUrl.substr(0, remoteUrl.find(":"));
				size_t port = stoi(remoteUrl.substr(remoteUrl.find(":") + 1, 4));
				if (msg.bodyString() == "quit") {
					std::cout << "\n\n  client " << msg.findValue("fromAddr") << " terminating";
					break;
				}
				if (Connect(si, remoteUrl)) {
					if (sendMessage(msg, si)) {
						if (msg.bodyString() != "")
							std::cout << "\n\n  " << msg.findValue("fromAddr") << " sent message: \n  "
							<< msg.attributes()[0].first << " " << msg.attributes()[0].second << ": " << msg.bodyString();
					}
					else std::cout << "\n\n  could not send message";
				}
			}
		};
		_pThread = new std::thread(threadProc);
	}

	// ----------< send the package name, file name, and dependencies >--------------
	void postFile(HttpMessage message) {
		std::string body = message.bodyString();
		size_t posComma = body.find(",");
		std::string dir = body.substr(0, posComma);
		std::string dependencies;
		std::string filename;
		body = body.substr(posComma + 1);
		posComma = body.find(",");
		if (posComma < body.length()) {
			filename = body.substr(0, posComma);
			dependencies = body.substr(posComma + 1);
		}
		else filename = body;
		std::string fqname = "/TestFiles/" + filename;
		FileSystem::FileInfo fi(fqname);
		FileSystem::File file(fqname);
		file.open(FileSystem::File::in, FileSystem::File::binary);
		if (!file.isGood()) {
			std::cout << "\n  could not open file " << fqname;
			return;
		}
		size_t fileSize = fi.size();
		std::string sizeString = Converter<size_t>::toString(fileSize);
		HttpMessage msg = makeMessage(1, "", message.findValue("toAddr"));
		msg.addAttribute(HttpMessage::Attribute("dir", dir));
		msg.addAttribute(HttpMessage::Attribute("file", filename));
		msg.addAttribute(HttpMessage::Attribute("deps", dependencies));
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		postMessage(msg);
	}

	// ----------< make a HTTP style message >----------------------
	HttpMessage makeMessage(size_t n, const std::string body, const EndPoint ep) {
		HttpMessage msg;
		HttpMessage::Attribute attrib;
		EndPoint myEndPoint = localUrl;
		msg.clear();
		switch (n) {
		case 0: // for informative, quit and closeServer messages
			msg.addAttribute(HttpMessage::attribute("POST", "Message"));
			break;
		case 1: // client send file with dependencies to server
			msg.addAttribute(HttpMessage::attribute("POST", "File"));
			break;
		case 2: // client get file from server
			msg.addAttribute(HttpMessage::attribute("GET", "File"));
			break;
		case 3: // client get file with dependencies from server
			msg.addAttribute(HttpMessage::attribute("GET", "FileWithDeps"));
			break;
		case 4: // server send files to client
			msg.addAttribute(HttpMessage::attribute("POST", "returnFile"));
			break;
		case 5: // client request file list from server
			msg.addAttribute(HttpMessage::attribute("GET", "getFileList"));
			break;
		case 6: // client request file list for a package from server
			msg.addAttribute(HttpMessage::attribute("GET", "getFileNameList"));
			msg.addAttribute(HttpMessage::attribute("dir", body));
			break;
		case 7: // close package
			msg.addAttribute(HttpMessage::attribute("POST", "closePackage"));
			msg.addAttribute(HttpMessage::attribute("dir", body));
			break;
		default:
			msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
			return msg;
		}
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
		msg.addBody(body);
		if (body.size() > 0) {
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		return msg;
	}

	// ---------< wait for sender to complete >-------------------
	void wait() {
		_pThread->join();
	}

	// ---------< clean up resources >----------------
	~Sender() {
		delete _pThread;
	}

private:
	// -----------< attempt to connect to the server >-------------
	bool Connect(SocketConnecter& s, std::string _remoteUrl) {
		std::cout << "\n";
		int numberOfAttempt = 1;
		size_t posColon = _remoteUrl.find(":");
		std::string addr = _remoteUrl.substr(0, posColon);
		size_t port = stoi(_remoteUrl.substr(posColon + 1, 4));
		while (!s.connect(addr, port)) {
			if (numberOfAttempt == 5) return false;
			std::cout << "\n  attempt to connect to " << addr << ":" << port << " #" << numberOfAttempt;
			++numberOfAttempt;
		}
		return true;
	}

	// -----------< send a string message using socket >------------------
	bool sendMessage(HttpMessage msg, Socket& socket) {
		std::string msgString = msg.toString();
		std::string filename = msg.findValue("file");
		if (!socket.send(msgString.size(), (Socket::byte*)msgString.c_str())) return false;
		if (filename != "") {
			if (!sendFile(filename, socket)) {
				std::cout << "\n  could not send file \n  " << filename;
				return false;
			}
			else std::cout << "\n\n  " << msg.findValue("fromAddr") << " sent file: " << filename << "\n";
		}
		return true;
	}

	// -----------< send a file using sockets >------------------
	bool sendFile(const std::string filename, Socket& socket) { // assumes that socket is connected
		std::string fqname = "../TestFiles/" + filename;
		FileSystem::FileInfo fi(fqname);
		size_t fileSize = fi.size();
		std::string sizeString = Converter<size_t>::toString(fileSize);
		FileSystem::File file(fqname);
		file.open(FileSystem::File::in, FileSystem::File::binary);
		if (!file.isGood()) return false;

		const size_t BlockSize = 2048;
		Socket::byte buffer[BlockSize];
		while (true) {
			FileSystem::Block blk = file.getBlock(BlockSize);
			if (blk.size() == 0) break;
			for (size_t i = 0; i < blk.size(); ++i)
				buffer[i] = blk[i];
			socket.send(blk.size(), buffer);
			if (!file.isGood())
				break;
		}
		file.close();
		return true;
	}

	SocketSystem ss;
	SocketConnecter si;
	std::string localUrl = "localhost:8081";
	std::string remoteUrl = "localhost:8080";
	Queue sndrQueue;
	std::thread * _pThread;
};

#endif