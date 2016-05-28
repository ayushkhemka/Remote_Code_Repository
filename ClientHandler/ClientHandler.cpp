/////////////////////////////////////////////////////////////////////
//  CientHandler.cpp - Client Handler for receiver                   //
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
ClientHandler.cpp, Cpp11-BlockingQueue.cpp, FileSystem.cpp, HttpMessage.cpp,
Sender.cpp, Sockets.cpp, Tokenizer.cpp, Utilities.cpp, XmlDocument.cpp,
XmlElement.cpp, xmlElementParts.cpp, XmlParser.cpp
Build commands (one by one)
- devenv Project4ood.sln /TEST_CLIENTHANDLER
- devenv Project4ood.sln /TEST_SENDER

Maintenance History:
====================
ver 1.0 : 03 May 16
- first release
*/

#include "ClientHandler.h"
#include "../Sender/Sender.h"
#include "../FileSystem/FileSystem.h"
#include "../Utilities/Utilities.h"
#include "../Logger/Logger.h"
#include <iomanip>
#include <string>

using Show = StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

//----< this defines processing to frame messages >------------------
HttpMessage ClientHandler::readMessage(Socket& socket) {
	connectionClosed_ = false;
	HttpMessage msg;
	while (true) { // read message attributes
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1) {
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else break;
	}
	if (msg.attributes().size() == 0) { // If client is done, connection breaks
		connectionClosed_ = true;					// and recvString returns empty string
		return msg;
	}
	if (msg.attributes()[0].first == "POST") // read body if POST
	{
		if (msg.attributes()[0].second == "Message") msg = readBody(msg, socket); // case 0 - normal message
		else if (msg.attributes()[0].second == "File") { // case 1 - client sending file to server
			saveFileServer(msg, socket);
			msg = constructMessage(msg);
		}
		else if (msg.attributes()[0].second == "closePackage") msg = closePackage(msg); // case 7 - close a package
		else if (msg.attributes()[0].second == "returnFile") { // case 4 - server sends files to client
			saveFileClient(msg, socket);
			msg = constructMessage(msg);
		}
	}
	else if (msg.attributes()[0].first == "GET") { // read message if GET
		msg = readBody(msg, socket);
		if (msg.attributes()[0].second == "getFileList")  msg = getFileList(msg); // case 5 - request list of packages
		else if (msg.attributes()[0].second == "getFileNameList")  msg = getFileNameList(msg); // case 6 - request list of files in a packages
		else if (msg.attributes()[0].second == "File" || msg.attributes()[0].second == "FileWithDeps")
			msg = getFiles(msg, msg.attributes()[0].second);
	}
	else {
		msg.removeAttribute("content-length");
		std::string bodyString = "<msg>Error message</msg>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}
	return msg;
}

//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string filename, size_t fileSize, Socket& socket, const std::string path)
{
	std::string fqname = path + filename;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood()) return false;

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	size_t bytesToRead;

	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}

// ---------------< read the contents of the message body, if it is a normal message >----------
HttpMessage ClientHandler::readBody(HttpMessage msg, Socket& socket) {
	size_t numBytes = 0;
	size_t pos = msg.findAttribute("content-length");
	HttpMessage message = msg;
	if (pos < msg.attributes().size()) {
		numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
		Socket::byte* buffer = new Socket::byte[numBytes + 1];
		socket.recv(numBytes, buffer);
		buffer[numBytes] = '\0';
		std::string msgBody(buffer);
		message.addBody(msgBody);
		delete[] buffer;
	}
		return message;
}

// -----------< save a file to the server >---------------
void ClientHandler::saveFileServer(HttpMessage msg, Socket& socket) {
	FileSystem::Directory::create("../TestFileServer/");
	std::string filename = msg.findValue("file") + ".ver1";
	std::string packageName = msg.findValue("dir") + "." + getCurrentDate();
	std::string path = "../TestFileServer/" + packageName + "/";
	FileSystem::Directory::create(path);
	std::string dependencies = msg.findValue("deps");
	std::vector<std::string> deps;
	if(dependencies != "") deps = findDeps(dependencies);
	std::vector<std::string> files;
	for (auto fileSpec : FileSystem::Directory::getFiles(path, msg.findValue("file") + ".ver*"))
		if (fileSpec.substr(fileSpec.find_last_of(".")) != ".xml") files.push_back(fileSpec);
	if (files.size() > 0) {
		std::string name = files[files.size() - 1];
		int ver = std::stoi(name.substr(name.find_last_of(".") + 4));
		filename = msg.findValue("file") + ".ver" + std::to_string(ver + 1);
	}

	std::string xmlpath = path + filename + ".xml";
	XmlDocument doc = builDocument(filename, deps);
	FileSystem::File file(xmlpath);
	
	file.open(FileSystem::File::out, FileSystem::File::text);
	if (file.isGood())
		file.putLine(doc.toString());
	file.close();

	size_t contentSize;
	std::string sizeString = msg.findValue("content-length");
	if (sizeString != "")
		contentSize = Converter<size_t>::toValue(sizeString);
	else
		return;
	readFile(filename, contentSize, socket, path);
	updateXML(packageName, msg.findValue("file"), deps);
}

// ---------------< update package xml file >--------------
void ClientHandler::updateXML(std::string package, std::string filename, std::vector<std::string> deps) {
	if (FileSystem::File::exists("../TestFileServer/" + package + "/" + package + ".xml")) {
		XmlDocument doc("../TestFileServer/" + package + "/" + package + ".xml", XmlDocument::file);
		for (auto file : doc.elements("file").select()) {
			if (file->tag() == "")
				if (file->value().substr(0, file->value().find("\n")) == filename) return;
		}
		sPtr file = makeFileElement(doc.xmlRoot(), package, filename, deps);
		FileSystem::File::remove("../TestFileServer/" + package + "/" + package + ".xml");
		FileSystem::File xml("../TestFileServer/" + package + "/" + package + ".xml");
		xml.open(FileSystem::File::out, FileSystem::File::text);
		if (xml.isGood()) xml.putLine(doc.toString());
		xml.close();
	}

	else {
		sPtr pRoot = makeTaggedElement("metadata");
		XmlDocument doc(XmlProcessing::makeDocElement(pRoot));
		sPtr name = makeTaggedElement("name");
		name->addChild(makeTextElement(package));
		pRoot->addChild(name);
		sPtr file = makeFileElement(pRoot, package, filename, deps);
		FileSystem::File xml("../TestFileServer/" + package + "/" + package + ".xml");
		xml.open(FileSystem::File::out, FileSystem::File::text);
		if (xml.isGood()) xml.putLine(doc.toString());
		xml.close();
	}
}

// ---------------< create xml element for file and its dependencies >-------------
sPtr ClientHandler::makeFileElement(sPtr pRoot, std::string package, std::string filename, std::vector<std::string> deps) {
	sPtr file = makeTaggedElement("file");
	file->addChild(makeTextElement(filename));
	sPtr depNames = makeTaggedElement("deps");
	for (auto depName : deps) {
		sPtr dep = makeTaggedElement("dep");
		dep->addChild(makeTextElement(depName));
		depNames->addChild(dep);
	}
	file->addChild(depNames);
	pRoot->addChild(file);
	return pRoot;
}

// ----------------< close a package to prevent addditional updating >-------------
HttpMessage ClientHandler::closePackage(HttpMessage msg) {
	HttpMessage message;
	if (!FileSystem::Directory::exists("../TestFileServer/" + msg.findValue("dir"))) {
		message.addAttribute(HttpMessage::attribute("type", "error"));
		message.addAttribute(HttpMessage::attribute("fromAddr", msg.findValue("toAddr")));
		message.addAttribute(HttpMessage::attribute("toAddr", msg.findValue("fromAddr")));
		std::string str = "\n  no package with the name " + msg.findValue("dir");
		message.addBody(str);
		return message;
	}
	std::string name = "../TestFileServer/" + msg.findValue("dir") + "/" + msg.findValue("dir") + "." + getCurrentDate() + ".xml";
	XmlDocument doc(name, XmlDocument::file);
	sPtr close = makeTaggedElement("close");
	close->addChild(makeTextElement("true"));
	doc.xmlRoot()->addChild(close);
	FileSystem::File::remove(name);
	FileSystem::File xml(name);
	xml.open(FileSystem::File::out, FileSystem::File::text);
	if (xml.isGood()) xml.putLine(doc.toString());
	xml.close();
	return message;
}

// --------------< save the file to the clients machine >------------------
void ClientHandler::saveFileClient(HttpMessage msg, Socket& socket) {
	std::string filename = msg.findValue("file").substr(msg.findValue("file").find_last_of("/") + 1);
	std::string path = "../TestFileClient/";
	FileSystem::Directory::create(path);
	size_t contentSize;
	std::string sizeString = msg.findValue("content-length");
	if (sizeString != "")
		contentSize = Converter<size_t>::toValue(sizeString);
	else
		return;
	readFile(filename, contentSize, socket, path);
}

// ----------------< find the dependencies along with their versions >---------
std::vector<std::string> ClientHandler::findDeps(std::string str) {
	std::vector<std::string> vect;
	bool ver = false;
	std::istringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ','))
	{
		size_t posVer = token.find_last_of("."); // check if user has specified the version number
		if (posVer < token.length() - 4) // if not, the file depends on the first version
			if (token.substr(posVer + 1, 3) == "ver") ver = true;
		if(!ver) vect.push_back(token + ".ver1");
		else vect.push_back(token);
	}
	return vect;
}

// ---------------< split a comma delimited string >------------------------
std::vector<std::string> ClientHandler::splitString(std::string str) {
	std::vector<std::string> vect;
	std::istringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ',')) vect.push_back(token);
	return vect;
}

// ---------------< create the xml document for a checked in file >-------------
XmlDocument ClientHandler::builDocument(std::string filename, std::vector<std::string> deps) {
	using sPtr = std::shared_ptr<AbstractXmlElement>;
	sPtr pRoot = makeTaggedElement("metadata");
	XmlDocument doc(XmlProcessing::makeDocElement(pRoot));

	sPtr name = makeTaggedElement("name");
	name->addChild(makeTextElement(filename));
	pRoot->addChild(name);
	sPtr depNames = makeTaggedElement("deps");
	for (auto depName : deps) {
		sPtr dep = makeTaggedElement("dep");
		dep->addChild(makeTextElement(depName));
		depNames->addChild(dep);
	}
	pRoot->addChild(depNames);
	return doc;
}

// ---------------< construct a general message to be sent as acknowledgement >---------
HttpMessage ClientHandler::constructMessage(HttpMessage msg) {
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + msg.findValue("file") + "</file>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
		return msg;
}

// --------------< construct a message based on custom body string >----------------
HttpMessage ClientHandler::constructMessage(HttpMessage msg, std::string body) {
	Sender sndr(msg.findValue("toAddr"));
	HttpMessage message = sndr.makeMessage(0, body, msg.findValue("fromAddr"));
	return message;
}

// --------------< get today's date >-----------------
std::string ClientHandler::getCurrentDate() {
	auto t = std::time(nullptr);
	struct tm tm;
	localtime_s(&tm, &t);
	std::string s = std::to_string(tm.tm_year + 1900) + "." + std::to_string(tm.tm_mon + 1) + "." + std::to_string(tm.tm_mday);
	return s;
}

// ----------------< get the list of packagees >---------------
HttpMessage ClientHandler::getFileList(HttpMessage msg) {
	std::vector<std::string> fileList = FileSystem::Directory::getDirectories("../TestFileServer/");
	std::string body = "";
	for (size_t i = 0; i < fileList.size(); ++i) 
		if(fileList[i] != "." && fileList[i] != "..")
			body += "\n  " + fileList[i];
	body += "\n";
	HttpMessage message;
	message.addAttribute(HttpMessage::attribute("toAddr", msg.findValue("fromAddr")));
	message.addAttribute(HttpMessage::attribute("fromAddr", msg.findValue("toAddr")));
	message.addAttribute(HttpMessage::attribute("type","returnList"));
	message.addBody(body);
	return message;
}

// ----------------< get the list of files in a specified package >-------------
HttpMessage ClientHandler::getFileNameList(HttpMessage msg) {
	std::vector<std::string> fileList = FileSystem::Directory::getFiles("../TestFileServer/" + msg.findValue("dir"));
	std::string body = "";
	for (size_t i = 0; i < fileList.size(); ++i)
		if (fileList[i] != "." && fileList[i] != "..")
			body += "\n  " + fileList[i];
	body += "\n";
	HttpMessage message;
	message.addAttribute(HttpMessage::attribute("fromAddr", msg.findValue("toAddr")));
	message.addAttribute(HttpMessage::attribute("toAddr", msg.findValue("fromAddr")));
	message.addAttribute(HttpMessage::attribute("type", "returnList"));
	message.addBody(body);
	return message;
}

// -----------------< prepare error message >--------------------
HttpMessage ClientHandler::errorMessage(HttpMessage msg, std::string body) {
	HttpMessage message;
	message.addAttribute(HttpMessage::attribute("type", "error"));
	message.addAttribute(HttpMessage::attribute("fromAddr", msg.findValue("toAddr")));
	message.addAttribute(HttpMessage::attribute("toAddr", msg.findValue("fromAddr")));
	message.addBody(body);
	return message;
}

// -----------------< get the requested files, with or without dependencies >------------
HttpMessage ClientHandler::getFiles(HttpMessage msg, std::string type) {
	HttpMessage message;
	if (!FileSystem::Directory::exists("../TestFileServer/" + msg.bodyString()))
		return errorMessage(msg, "\n  no package " + msg.bodyString());
	std::string path = "../TestFileServer/" + msg.bodyString() + "/";
	std::vector<std::string> files = FileSystem::Directory::getFiles(path, "*.ver*");
	if (files.size() == 0) return errorMessage(msg, "\n  so files in package " + msg.bodyString());
	std::vector<std::string> sendFiles;
	for (int i = files.size() - 1; i >= 0; --i) { // figure out the latest version of the files
		std::string ext = files[i].substr(files[i].find_last_of(".") + 1);
		if (ext == "xml") continue;
		std::string name = files[i].substr(0, files[i].find_last_of("."));
		if (find(sendFiles, name) == sendFiles.size())
			sendFiles.push_back(files[i]); // add only unique file names to be sent
	}
	std::vector<std::shared_ptr<AbstractXmlElement>> deps;
	if (type == "FileWithDeps") { // figure out dependencies of files to be sent
		for (int i = 0; i < sendFiles.size(); ++i) {
			std::string file = sendFiles[i];
			if (!FileSystem::File::exists("../TestFileServer/" + msg.bodyString() + "/" + file)) continue;
			std::string xmlname = "../TestFileServer/" + msg.bodyString() + "/" + file + ".xml";
			XmlDocument doc(xmlname, XmlDocument::file);
			deps = doc.element("deps").descendents().select();
			for (auto dep : deps)
				if (dep->tag() == "") { // prevent circular dependencies to be stuck in a loop
					std::string depName = dep->value().substr(0, dep->value().find("\n"));
					if (find(sendFiles, depName.substr(0, depName.find_last_of("."))) == sendFiles.size()) sendFiles.push_back(depName);
				}
		}
	}
	std::string body;
	for (size_t i = 0; i < sendFiles.size() - 1; ++i) body += sendFiles[i] + ",";
	body += sendFiles[sendFiles.size() - 1];
	message.addAttribute(HttpMessage::attribute("fromAddr", msg.findValue("toAddr")));
	message.addAttribute(HttpMessage::attribute("toAddr", msg.findValue("fromAddr")));
	message.addAttribute(HttpMessage::attribute("type", "returnFiles"));
	message.addBody(body);
	return message;
}

// ------------------< find a given string in the vector >-------------------
size_t ClientHandler::find(std::vector<std::string> vec, std::string str) {
	for (size_t i = 0; i < vec.size(); ++i)
		if (vec[i].substr(0, vec[i].find_last_of(".")) == str) return i;
	return vec.size();
}

// ----------------< look for file on the server to be sent >----------------
std::string ClientHandler::searchFile(std::string filename) {
	for (auto dir : FileSystem::Directory::getDirectories("../TestFileServer/")) {
		if (dir != "." && dir != "..") {
			std::vector<std::string> files = FileSystem::Directory::getFiles("../TestFileServer/" + dir, filename);
			if (files.size() == 0) continue;
			else return "../TestFileServer/" + dir + "/" + filename;
		}
	}
	return "";
}

// ------------------< prepare and send message to recepient >-------------
void ClientHandler::sendMessage(HttpMessage msg) {
	HttpMessage message;
	Sender sndr(msg.findValue("fromAddr"));
	sndr.start();
	message = sndr.makeMessage(0, msg.bodyString(), msg.findValue("toAddr"));
	sndr.postMessage(message);
	sndr.postMessage(sndr.makeMessage(0, "closeServer", msg.findValue("toAddr")));
	sndr.postMessage(sndr.makeMessage(0, "quit", msg.findValue("toAddr")));
	sndr.wait();
}

//----< receiver functionality is defined by this function >---------
void ClientHandler::operator()(Socket socket) {
	std::function<void()> threadProc = [&]() {
		while (true) {
			HttpMessage msg = readMessage(socket); // read a message from socket
			if (msg.attributes().size() == 0 || msg.bodyString() == "quit") break;
			if (msg.findValue("type") == "returnList") {
				sendMessage(msg);
				continue;
			}
			else if (msg.findValue("type") == "returnFiles") {
				Sender sndr(msg.findValue("fromAddr"));
				sndr.start();
				std::vector<std::string> files = splitString(msg.bodyString());
				for (std::string file : files) {
					std::string fileSpec = searchFile(file);
					FileSystem::FileInfo fi(fileSpec);
					FileSystem::File fileIn(fileSpec);
					fileIn.open(FileSystem::File::in, FileSystem::File::binary);
					if (!fileIn.isGood()) {
						std::cout << "\n  could not open file " << file;
						continue;
					}
					size_t fileSize = fi.size();
					std::string sizeString = Converter<size_t>::toString(fileSize);
					HttpMessage message = sndr.makeMessage(4, "", msg.findValue("toAddr"));
					message.addAttribute(HttpMessage::Attribute("file", fileSpec));
					message.addAttribute(HttpMessage::Attribute("content-length", sizeString));
					sndr.postMessage(message);
				}
				sndr.postMessage(sndr.makeMessage(0, "closeServer", msg.findValue("toAddr")));
				sndr.postMessage(sndr.makeMessage(0, "quit", msg.findValue("toAddr")));
				sndr.wait();
				continue;
			}
			else if (msg.attributes()[0].second == "error") {
				sendMessage(msg);
				continue;
			}
			msgQ_.enQ(msg);
		}
	};
	std::thread receiveThread(threadProc);
	receiveThread.join();
}

// --------------< test stub >-------------
#ifdef TEST_CLIENTHANDLER

int main() {
	::SetConsoleTitle("ClientHandler");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  ClientHandler Server started");

	BlockingQueue<HttpMessage> msgQ;
	SocketSystem ss;
	SocketListener sl(8080, Socket::IP6);
	ClientHandler cp(msgQ);
	sl.start(cp);

	try
	{
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			std::cout << "\n\n  clienthandler " + msg.findValue("toAddr") + " recvd message contents:\n  "
				+ msg.bodyString() + "\n  from client " + msg.findValue("fromAddr") + "\n";
			if (msg.bodyString() == "closeServer") {
				::Sleep(100);
			}
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}

	std::cout << "\n\n";
	return 0;
}

#endif