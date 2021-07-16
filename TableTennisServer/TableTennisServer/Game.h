#pragma once
#include <string>
#include "CivetServer.h"
#include <vector>

using namespace std;

class Team;

class User {
public:
   unsigned int id;
   string name = "";
   mg_connection* conn;
   Team* team = nullptr;
};

class Team {
public:
   unsigned int id;
   string name = "";
   User* owner;
   vector<User*> users;
};

class Tour {
public:
   unsigned int id;
   string name = "";
   Team* home = nullptr;
   Team* away = nullptr;
   User* owner;
};