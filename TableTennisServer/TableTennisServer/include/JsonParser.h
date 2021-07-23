#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class JsonValue;
class JsonObject;

typedef enum
{
   JsonValueTypeString,
   JsonValueTypeNumber,
   JsonValueTypeObject,
   JsonValueTypeArray,
   JsonValueTypeTrue,
   JsonValueTypeFalse,
   JsonValueTypeNull
} JsonValueType;

typedef union
{
   std::string *u_string;
   std::string *u_number;
   JsonObject *u_jsonObject;
   std::vector<JsonValue *> *u_jsonArray;
} JsonValueData;

class JsonValue
{
public:
   JsonValue()
   {
      m_type = JsonValueTypeNull;
      m_data.u_jsonArray = NULL;
   }

   void assign(JsonValueType type);

   JsonValueType m_type;
   JsonValueData m_data;

   ~JsonValue();

private:
   JsonValue(const JsonValue&);
   JsonValue& operator=(const JsonValue& other) = delete;
};

class JsonNameValuePair
{
public:
   JsonNameValuePair()
   {
   }

   std::string name;
   JsonValue jsonValue;
private:
   JsonNameValuePair(const JsonNameValuePair&);
   JsonNameValuePair& operator=(const JsonNameValuePair& other) = delete;
};

class JsonObject
{
public:
   JsonObject()
   {
   }

   ~JsonObject()
   {
      for (auto i : m_nameValuePair)
      {
         delete i;
      }
   }
   void AddNameValuePairNumber(const char* name, int value);
   void AddNameValuePairBool(const char* name, bool value);
   void AddNameValuePairString(const char* name, const char* value);
   void AddNameValuePairArray(const char* name, const int* values, const int count);
   void AddNameValuePairObject(const char* name, JsonObject* value);
   void AddNameValuePairArray(const char* name, vector<JsonValue*>* value);

   std::vector<JsonNameValuePair*> m_nameValuePair;
private:
   JsonObject(const JsonObject&);
   JsonObject& operator=(const JsonObject& other);
};





void parseString(std::string &, const char*, unsigned int&, const size_t);
void parseValue(JsonValue &, const char *, unsigned int &, const size_t);
void parseObject(JsonObject &jsonRootObject, const char *str2parse, unsigned int &idx, const size_t len);

char getNextChar(const char *str2parse, unsigned int &idx, const size_t len, bool adjustRowNumber = false);
void eatChar(char char2eat, const char *str2parse, unsigned int &idx, const size_t len);
bool isChar(char char2check, const char *str2parse, unsigned int &idx, const size_t len);
void parseArray(vector<JsonValue*> &jsonArray, const char *str2parse, unsigned int &idx, const size_t len);
void parseNumber(string &u_number, const char *str2parse, unsigned int &idx, const size_t len);
bool checkCharacterPattern(string wantedPattern, const char *str2parse, unsigned int &idx, const size_t len);
void parseNull(const char *str2parse, unsigned int &idx, const size_t len);
void parseTrue(const char *str2parse, unsigned int &idx, const size_t len);
void parseFalse(const char *str2parse, unsigned int &idx, const size_t len);
void printObject(const char *name, JsonObject &jsonObject);

void stringifyArray(string& output, const vector<JsonValue*>& jsonArray, const bool formatted);
void stringifyObject(string& output, const JsonObject& jsonObject, const bool formatted);
