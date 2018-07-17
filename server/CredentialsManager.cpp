//
// Created by Jordan Bare on 7/12/18.
//

#include <cereal/archives/portable_binary.hpp>
#include <fstream>
#include "CredentialsManager.h"

void CredentialsManager::checkForCredentialsFile() {
    /*
    std::string credFileName = mCredentialsDirectory + "loginInfo.txt";
    std::ifstream credentialsInFile(credFileName);
    if(!credentialsInFile.is_open()){
        std::string userName;
        std::string password;
        std::cout << "Enter the Username: " << std::endl;
        std::cin >> userName;
        std::cout << "Enter the Password: " << std::endl;
        std::cin >> password;
        std::unique_ptr credentials = std::make_unique<Credentials>(userName, password);
        OPENSSL_cleanse(&userName, userName.length());
        OPENSSL_cleanse(&password, password.length());
        std::ofstream credentialsOutFile(credFileName);
        if(credentialsOutFile.is_open()){
            cereal::PortableBinaryOutputArchive outputArchive(credentialsOutFile);
            outputArchive(*credentials);
            credentialsOutFile.close();
        }
    } else {
        credentialsInFile.close();
    }
     */
}

CredentialsManager::CredentialsManager(const std::string &folderRoot):mCredentialsDirectory(folderRoot) {
    //checkForCredentialsFile();
}

bool CredentialsManager::compareCredentials(const std::string &body) {
    std::cout << body << std::endl;
    std::string usrAttribute = "usr=";
    std::string pwdAttribute = "\npwd=";
    unsigned long usrLoc = body.find(usrAttribute);
    unsigned long pwdLoc = body.find(pwdAttribute);
    unsigned long csrfLoc = body.find("\n_csrf=");
    std::string user = body.substr(usrLoc + usrAttribute.length(), (pwdLoc - (usrLoc + usrAttribute.length())) - 1);
    std::string password = body.substr(pwdLoc + pwdAttribute.length(), (csrfLoc - (pwdLoc + pwdAttribute.length())) - 1);

    /*
    std::unique_ptr<Credentials> credentials = std::make_unique<Credentials>();
    std::ifstream credentialsFile(mCredentialsDirectory + "loginInfo.txt");
    if(credentialsFile.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(credentialsFile);
        inputArchive(*credentials);
        credentialsFile.close();
    }
    if(!credentials->compareUserName(user)){
        return false;
    }
    else if(!credentials->comparePassword(password)){
        return false;
    }
     */
    /*

    if(password == comparison){
        std::cout << "Password match" << std::endl;
        return true;
    }
    return false;
     */
    std::cout << user << " : " << user.length() << std::endl;
    std::string comparison("password");
    std::cout << comparison << password << std::endl;
    std::cout << password.length() << " : " << comparison.length() << std::endl;
    return password == comparison;
}
