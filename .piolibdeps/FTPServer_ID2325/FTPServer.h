
/*
Copyright (c) 2011 Robert Ellis (holistic [at] robellis [dot] org [dot] uk)
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/**
FTP Server header file
*/

#ifndef FTP_SERVER_H
#define FTP_SERVER_H


#include "core/net.h"
#include "api/TCPSocket.h"
#include "FTPUser.h"
class FTPRequestHandler;

#include <string>
#include <list>
#include <map>
using namespace std;



/**
 * A simple FTP Server.
 */
class FTPServer
{
public:

  /**
   * Constructor.
   */
  FTPServer();
  ~FTPServer();
  
  /**
   * Set the welcome message to return when a client connects.
   * @param message The welcome message.
   */
  void setWelcomeMessage(string message);
  
  /**
   * Set the IP address of the server. Required for passive mode.
   * @param ip The IP address of this server.
   */
  void setIp(const IpAddr& ip) { m_ip = ip; }
  
  /**
   * Get the welcome message.
   */
  string& getWelcomeMessage();
  
  /**
   * Get the IP address of the server.
   */
  const IpAddr& getIp() { return m_ip; }
  
  /**
   * Get the port number of the data port.
   */
  int getDataPort() { return m_dataPort; }
  
  /**
   * Add an FTP user to the server. At least one user is required for a client to be able to log in.
   * @param user The user to add.
   */
  void addUser(FTPUser& user);
  
  /**
   * Check if the given usernamen and password is a valid log in.
   * @param username The username.
   * @param password The password.
   * @return The FTPUser if the username and password are correct, otherwise returns NULL.
   */
  FTPUser* checkUser(const string& username, const string& password);
  
  /**
  * Binds server to a specific port and starts listening.
  * @param commandPort Port on which to listen for incoming connections.
  * @param dataPort Default port to listen for data connections. Used in passive mode only.
  */
  void bind(int comandPort = 21, int dataPort = 20);
  
private:

  void onCommandSocketEvent(TCPSocketEvent e);
  void onDataSocketEvent(TCPSocketEvent e);
  bool checkError(char* msg);
  
  // listening sockets
  TCPSocket m_commandSocket;
  TCPSocket m_dataSocket;
  
  IpAddr m_ip;
  int m_commandPort;
  int m_dataPort;
  
  string m_message;
  list<FTPUser> users;
  
  map<long, FTPRequestHandler*> connections;
  TCPSocketErr error;

};


#endif
