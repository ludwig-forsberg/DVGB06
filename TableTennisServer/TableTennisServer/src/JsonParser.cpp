#include "JsonParser.h"

unsigned int g_row = 1;   // TODO remove global
JsonValue::~JsonValue()
{
   switch (m_type)
   {
   case JsonValueTypeString:
      if (m_data.u_string != NULL)
      {
         delete m_data.u_string;
         m_data.u_string = NULL;
      }
      break;
   case JsonValueTypeNumber:
      if (m_data.u_number != NULL)
      {
         delete m_data.u_number;
         m_data.u_number = NULL;
      }
      break;
   case JsonValueTypeObject:
      if (m_data.u_jsonObject != NULL)
      {
         delete m_data.u_jsonObject;
         m_data.u_jsonObject = NULL;
      }
      break;
   case JsonValueTypeArray:
      if (m_data.u_jsonArray != NULL)
      {
         for (auto i : *m_data.u_jsonArray)
         {
            delete i;
         }

         delete m_data.u_jsonArray;
         m_data.u_jsonArray = NULL;
      }
      break;
   default:
      break;
   }
}


char getNextChar(const char *str2parse, unsigned int &idx, const size_t len, bool adjustRowNumber)
{
   //idx++;
   char nextChar = str2parse[idx];

   while (nextChar == '\n' || nextChar == '\r' || nextChar == '\t' || nextChar == ' ')
   {
      if (nextChar == '\n' && adjustRowNumber == true)
         g_row++;
      idx++;
      nextChar = str2parse[idx];
   }
   return nextChar;
}

void eatChar(char char2eat, const char *str2parse, unsigned int &idx, const size_t len)
{
   if (idx >= len)
   {
      printf("Unexpected end of file in json at row: %d. Expected char '%c'\n", g_row, char2eat);
      return;
   }

   char currentChar = str2parse[idx];
   while (idx < len && (currentChar == '\n' || currentChar == '\r' || currentChar == '\t' || currentChar == ' '))
   {
      if (currentChar == '\n')
         g_row++;
      idx++;
      currentChar = str2parse[idx];
   }

   if (idx >= len)
   {
      printf("Unexpected end of file in json at row: %d. Expected char '%c'\n", g_row, char2eat);
      return;
   }

   currentChar = str2parse[idx];
   if (currentChar != char2eat) {
      printf("Unexpected error in json: Expected char '%c' but got '%c'. Row: %d\n", char2eat, currentChar, g_row);
   }
   else {
      idx++;
   }
}

bool isChar(char char2check, const char *str2parse, unsigned int &idx, const size_t len)
{
   if (str2parse[idx] == char2check) {
      return true;
   }
   else
   {
      return false;
   }

}

void parseArray(vector<JsonValue*> &jsonArray, const char *str2parse, unsigned int &idx, const size_t len)
{

   eatChar('[', str2parse, idx, len);
   unsigned int storedIdx = idx;
   if (getNextChar(str2parse, storedIdx, len) != ']') {
      JsonValue *jsonValue = new JsonValue;
      parseValue(*jsonValue, str2parse, idx, len);
      jsonArray.push_back(jsonValue);
      while (isChar(',', str2parse, idx, len))
      {
         eatChar(',', str2parse, idx, len);
         jsonValue = new JsonValue;
         parseValue(*jsonValue, str2parse, idx, len);
         jsonArray.push_back(jsonValue);
      }
   }
   eatChar(']', str2parse, idx, len);
}

void parseString(string &u_string, const char *str2parse, unsigned int &idx, const size_t len)
{
   u_string = "";
   eatChar('"', str2parse, idx, len);
   //char currentChar = str2parse[idx];
   while (!isChar('"', str2parse, idx, len))
   {
      u_string += str2parse[idx];
      idx++;
      //if (!isChar('"', str2parse, idx, len))
      //u_string += getNextChar(str2parse, idx, len);

   }
   eatChar('"', str2parse, idx, len);
   //u_string += '\0';
}

void parseNumber(string &u_number, const char *str2parse, unsigned int &idx, const size_t len)
{
   u_number = "";
   //eatChar('"', str2parse, idx, len);
   while ((int)str2parse[idx] == (int)'.' || (int)str2parse[idx] == (int)'-' || ((int)str2parse[idx] >= (int)'0' && (int)str2parse[idx] <= (int)'9'))
   {
      u_number += str2parse[idx];
      idx++;
   }
   //eatChar('"', str2parse, idx, len);
   //u_number += '\0';
}

bool checkCharacterPattern(string wantedPattern, const char *str2parse, unsigned int &idx, const size_t len)
{
   bool valid = true;
   size_t endIdx = idx + wantedPattern.size();
   int i = 0;
   while (idx < endIdx)
   {
      if (str2parse[idx] != wantedPattern[i])
      {
         valid = false;
      }
      i++;
      idx++;
   }
   return valid;
}

void parseNull(const char *str2parse, unsigned int &idx, const size_t len)
{
   bool valid = checkCharacterPattern("null", str2parse, idx, len);
}

void parseTrue(const char *str2parse, unsigned int &idx, const size_t len)
{
   bool valid = checkCharacterPattern("true", str2parse, idx, len);
}

void parseFalse(const char *str2parse, unsigned int &idx, const size_t len)
{
   bool valid = checkCharacterPattern("false", str2parse, idx, len);
}

void parseValue(JsonValue &jsonValue, const char *str2parse, unsigned int &idx, const size_t len)
{
   char nextChar = getNextChar(str2parse, idx, len, true);
   if (nextChar == '{')
   {
      jsonValue.m_type = JsonValueTypeObject;
      jsonValue.m_data.u_jsonObject = new JsonObject;
      parseObject(*jsonValue.m_data.u_jsonObject, str2parse, idx, len);
   }
   else if (nextChar == '[')
   {
      jsonValue.m_type = JsonValueTypeArray;
      jsonValue.m_data.u_jsonArray = new vector<JsonValue*>;
      parseArray(*jsonValue.m_data.u_jsonArray, str2parse, idx, len);
   }
   else if (nextChar == '"')
   {
      jsonValue.m_type = JsonValueTypeString;
      jsonValue.m_data.u_string = new string;
      parseString(*jsonValue.m_data.u_string, str2parse, idx, len);
   }
   else if (nextChar == 'n')
   {
      jsonValue.m_type = JsonValueTypeNull;
      parseNull(str2parse, idx, len);
   }
   else if (nextChar == 'f')
   {
      jsonValue.m_type = JsonValueTypeFalse;
      parseFalse(str2parse, idx, len);
   }
   else if (nextChar == 't')
   {
      jsonValue.m_type = JsonValueTypeTrue;
      parseTrue(str2parse, idx, len);
   }
   else
   {
      jsonValue.m_type = JsonValueTypeNumber;
      jsonValue.m_data.u_string = new string;
      parseNumber(*jsonValue.m_data.u_string, str2parse, idx, len);
   }

}

void parseObject(JsonObject &jsonRootObject, const char *str2parse, unsigned int &idx, const size_t len)
{
   // string : value and 0 or more combination of , string : value

   eatChar('{', str2parse, idx, len);
   unsigned int storedIdx = idx;
   if (getNextChar(str2parse, storedIdx, len) != '}') {
      JsonNameValuePair *valuePair = new JsonNameValuePair;

      jsonRootObject.m_nameValuePair.push_back(valuePair);

      parseString(valuePair->name, str2parse, idx, len);
      eatChar(':', str2parse, idx, len);
      parseValue(valuePair->jsonValue, str2parse, idx, len);

      while (isChar(',', str2parse, idx, len))
      {
         valuePair = new JsonNameValuePair;
         jsonRootObject.m_nameValuePair.push_back(valuePair);

         eatChar(',', str2parse, idx, len);
         parseString(valuePair->name, str2parse, idx, len);
         eatChar(':', str2parse, idx, len);
         parseValue(valuePair->jsonValue, str2parse, idx, len);
      }
   }

   eatChar('}', str2parse, idx, len);

}

static unsigned int g_objNumber = 0;

static void indent()
{
   for (unsigned int i = 0; i < g_objNumber; i++)
   {
      cout << "   ";
   }
}

static void indent(string& output)
{
   for (unsigned int i = 0; i < g_objNumber; i++)
   {
      output.append("   ");
   }
}
void printObject(const char *name, JsonObject &jsonObject) 
{
   indent();
   cout << name << "(" << "Object" << ")" << endl;
   g_objNumber++;
   for (unsigned int i = 0; i < jsonObject.m_nameValuePair.size(); i++)
   {
      if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeObject) 
      {
         printObject(jsonObject.m_nameValuePair.at(i)->name.c_str(), *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_jsonObject);
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeArray)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "Array" << ")" << endl;
         for (auto u : *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_jsonArray) {
            if (u->m_type == JsonValueTypeObject) {
               printObject("[]", *u->m_data.u_jsonObject);
            }
         }
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeString)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "String" << ")" << endl;
         indent();
         cout << "   " << *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_string << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeNumber)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "Number" << ")" << endl;
         indent();
         cout << "   " << *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_number << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeTrue)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "TRUE" << ")" << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeFalse)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "FALSE" << ")" << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeNull)
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "NULL" << ")" << endl;
      }
      else
      {
         indent();
         cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "TODO" << ")" << endl;
      }
   }
   g_objNumber--;
}

void stringifyArray(string& output, const vector<JsonValue*>& jsonArray, const bool formatted) {
   size_t size = jsonArray.size();
   unsigned int i = 0;
   for (auto u : jsonArray) {
      if (u->m_type == JsonValueTypeObject) {
         stringifyObject(output, *u->m_data.u_jsonObject, formatted);
      }
      else if (u->m_type == JsonValueTypeArray)
      {
         if (formatted)
            indent(output);
         output.append("[");
         g_objNumber++;
         if (formatted)
            output.append("\r\n");
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "Array" << ")" << endl;
         stringifyArray(output, *u->m_data.u_jsonArray, formatted);
         g_objNumber--;
         if (formatted)
            indent(output);
         output.append("]");
      }
      else if (u->m_type == JsonValueTypeString)
      {
         if (formatted)
            indent(output);
         output.append("\"" + *u->m_data.u_string + "\"");
      }
      else if (u->m_type == JsonValueTypeNumber)
      {
         if (formatted)
            indent(output);
         output.append(*u->m_data.u_number);
      }
      else if (u->m_type == JsonValueTypeTrue)
      {
         if (formatted)
            indent(output);
         output.append("true");
      }
      else if (u->m_type == JsonValueTypeFalse)
      {
         if (formatted)
            indent(output);
         output.append("false");
      }
      else if (u->m_type == JsonValueTypeNull)
      {
         if (formatted)
            indent(output);
         output.append("null");
      }
      if (i + 1 < size) {
         output.append(",");
      }
      if (formatted)
         output.append("\r\n");
      i++;
   }
}

void stringifyObject(string& output, const JsonObject& jsonObject, const bool formatted) {
   if (formatted)
      indent(output);
   output.append("{");
   if (formatted)
      output.append("\r\n");

   g_objNumber++;
   
   size_t size = jsonObject.m_nameValuePair.size();
   for (unsigned int i = 0; i < size; i++)
   {
      if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeObject)
      {
         if(formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":");
         stringifyObject(output, *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_jsonObject, formatted);
         //printObject(jsonObject.m_nameValuePair.at(i)->name.c_str(), *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_jsonObject);
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeArray)
      {
         if (formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":[");
         g_objNumber++;
         if (formatted)
            output.append("\r\n");
         
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "Array" << ")" << endl;
         stringifyArray(output, *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_jsonArray, formatted);

         g_objNumber--;
         if (formatted)
            indent(output);
         output.append("]");
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeString)
      {
         if (formatted)
            indent(output);
         output.append("\"" +jsonObject.m_nameValuePair.at(i)->name + "\":\"" + *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_string + "\"");
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeNumber)
      {
         if (formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":" + *jsonObject.m_nameValuePair.at(i)->jsonValue.m_data.u_number);
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeTrue)
      {
         if (formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":true");
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "TRUE" << ")" << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeFalse)
      {
         if (formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":false");
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "FALSE" << ")" << endl;
      }
      else if (jsonObject.m_nameValuePair.at(i)->jsonValue.m_type == JsonValueTypeNull)
      {
         if (formatted)
            indent(output);
         output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":null");
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "NULL" << ")" << endl;
      }
      else
      {
         //indent(output);
         //output.append("\"" + jsonObject.m_nameValuePair.at(i)->name + "\":false\r\n");
         //cout << jsonObject.m_nameValuePair.at(i)->name << "(" << "TODO" << ")" << endl;
      }
      if (i + 1 < size) {
         output.append(",");
      }
      if (formatted)
         output.append("\r\n");
   }
   g_objNumber--;
   if (formatted)
      indent(output);
   output.append("}");
}

void JsonObject::AddNameValuePairNumber(const char* name, int value)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   jsonNameValuePair->jsonValue.m_type = JsonValueTypeNumber;
   if (jsonNameValuePair->jsonValue.m_data.u_number == NULL)
      jsonNameValuePair->jsonValue.m_data.u_number = new string(to_string(value));
   else
      *jsonNameValuePair->jsonValue.m_data.u_number = to_string(value);

   this->m_nameValuePair.push_back(jsonNameValuePair);
}

void JsonObject::AddNameValuePairBool(const char* name, bool value)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   if(value)
      jsonNameValuePair->jsonValue.m_type = JsonValueTypeTrue;
   else
      jsonNameValuePair->jsonValue.m_type = JsonValueTypeFalse;

   this->m_nameValuePair.push_back(jsonNameValuePair);
}


void JsonObject::AddNameValuePairString(const char* name, const char* value)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   jsonNameValuePair->jsonValue.m_type = JsonValueTypeString;
   if (jsonNameValuePair->jsonValue.m_data.u_string == NULL)
      jsonNameValuePair->jsonValue.m_data.u_string = new string(value);
   else
      *jsonNameValuePair->jsonValue.m_data.u_string = value;

   this->m_nameValuePair.push_back(jsonNameValuePair);
}

void JsonObject::AddNameValuePairArray(const char* name, const int* values, const int count)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   jsonNameValuePair->jsonValue.m_type = JsonValueTypeArray;
   if (jsonNameValuePair->jsonValue.m_data.u_jsonArray == NULL)
      jsonNameValuePair->jsonValue.m_data.u_jsonArray = new vector<JsonValue*>();
   
   for (int i = 0; i < count; i++) {
      JsonValue* value = new JsonValue();
      value->m_type = JsonValueTypeNumber;
      value->m_data.u_number = new string(to_string(values[i]));
      jsonNameValuePair->jsonValue.m_data.u_jsonArray->push_back(value);
   }

   this->m_nameValuePair.push_back(jsonNameValuePair);
}

void JsonObject::AddNameValuePairObject(const char* name, JsonObject* value)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   jsonNameValuePair->jsonValue.m_type = JsonValueTypeObject;

   jsonNameValuePair->jsonValue.m_data.u_jsonObject = value;

   this->m_nameValuePair.push_back(jsonNameValuePair);
}

void JsonObject::AddNameValuePairArray(const char* name, vector<JsonValue*>* value)
{
   JsonNameValuePair* jsonNameValuePair = new JsonNameValuePair();

   jsonNameValuePair->name = string(name);
   jsonNameValuePair->jsonValue.m_type = JsonValueTypeArray;

   jsonNameValuePair->jsonValue.m_data.u_jsonArray = value;

   this->m_nameValuePair.push_back(jsonNameValuePair);
}
