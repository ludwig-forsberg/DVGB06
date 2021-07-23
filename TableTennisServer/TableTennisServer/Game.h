#pragma once
#include <string>
#include "CivetServer.h"
#include <vector>
#include <map>
#include "JsonParser.h"

using namespace std;

class Team;

class GameObject {
public:
   virtual void generateJson(JsonObject& jsonObject) = 0;
   virtual void generateJson(JsonObject& jsonObject, unsigned int levels) = 0;
};

class User : public GameObject{
public:
   void generateJson(JsonObject& jsonObject);
   void generateJson(JsonObject& jsonObject, unsigned int levels);
   unsigned int id;
   string name = "";
   mg_connection* conn;
   Team* team = nullptr;
};

class Team : public GameObject {
public:
   void generateJson(JsonObject& jsonObject);
   void generateJson(JsonObject& jsonObject, unsigned int levels);
   unsigned int id;
   string name = "";
   User* owner;
   vector<User*> users;
};

class Set : public GameObject {
public:
   Set();
   Set(unsigned int id);
   unsigned int id;
   void generateJson(JsonObject& jsonObject);
   void generateJson(JsonObject& jsonObject, unsigned int levels);
   unsigned int team1Points = 0;
   unsigned int team2Points = 0;
};

class Match : public GameObject {
public:
   Match();
   Match(unsigned int id);
   void generateJson(JsonObject& jsonObject);
   void generateJson(JsonObject& jsonObject, unsigned int levels);
   unsigned int id;
   vector<User*> playersTeam1;
   vector<User*> playersTeam2;
   vector<Set> sets;
   unsigned int team1SetPoints = 0;
   unsigned int team2SetPoints = 0;
   unsigned int playersPerTeam = 1;
   unsigned int numberOfSets = 5;
   bool originalSide = true;
   bool originalServe = true;
};

class Tour : public GameObject {
public:
   Tour();
   void generateJson(JsonObject& jsonObject);
   void generateJson(JsonObject& jsonObject, unsigned int levels);
   unsigned int id;
   string name = "";
   Team* home = nullptr;
   Team* away = nullptr;
   User* owner;
   map<unsigned int, Match> matches;
};

