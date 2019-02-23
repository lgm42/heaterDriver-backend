
#ifndef FTP_USER_H
#define FTP_USER_H

#include <string>

using namespace std;

/**
 * An FTP User.
 */
class FTPUser
{
public:
    
    /**
     * Constructor.
     * @param username Username.
     * @param password Password.
     */
    FTPUser(char* username, char* password);
    FTPUser();
    
    /**
     * Sets the username.
     * @param username Username.
     */
    void setUsername(const string& username) { m_username = username; }
    
    /**
     * Sets the password.
     * @param username Password.
     */
    void setPassword(const string& password) { m_password = password; }
    
    /**
     * Get the username.
     */
    string& getUsername() { return m_username; }
    
    /**
     * Get the password.
     */
    string& getPassword() { return m_password; }
    
private:
    string m_username;
    string m_password;
};

#endif
