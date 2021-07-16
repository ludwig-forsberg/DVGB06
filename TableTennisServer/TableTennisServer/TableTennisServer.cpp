/* Copyright (c) 2013-2018 the Civetweb developers
 * Copyright (c) 2013 No Face Press, LLC
 * License http://opensource.org/licenses/mit-license.php MIT License
 */

 // Simple example program on how to use Embedded C++ interface.

#include "CivetServer.h"
#include <cstring>
#include <vector>
#include <map>
#include "JsonParser.h"
#include "Game.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define DOCUMENT_ROOT "./website"
#define PORT "80,443s"
#define SSL_CERTIFICATE_PATH "C:/Users/ludde/Documents/EscapeReality/TableTennisApp/server.pem"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"




/* Exit flag for main loop */
volatile bool exitNow = false;


class ExitHandler : public CivetHandler
{
public:
   bool
	  handleGet(CivetServer* server, struct mg_connection* conn)
   {
	  mg_printf(conn,
		 "HTTP/1.1 200 OK\r\nContent-Type: "
		 "text/plain\r\nConnection: close\r\n\r\n");
	  mg_printf(conn, "Bye!\n");
	  exitNow = true;
	  return true;
   }
};

class WebsocketHandler : public CivetWebSocketHandler {
   unsigned int clientCounter = 0;
   unsigned int teamCounter = 0;
   unsigned int tourCounter = 0;
   char gameData[1000] = "";
   std::map<unsigned int, User*> all_users;
   std::map<unsigned int, Team*> all_teams;
   std::map<unsigned int, Tour*> all_tours;

   void broadcast(string& message) {
	  for (std::map<unsigned int, User*>::iterator iter = all_users.begin(); iter != all_users.end(); ++iter)
	  {
		 mg_websocket_write(iter->second->conn, MG_WEBSOCKET_OPCODE_TEXT, message.c_str(), message.length());
	  }
   }

   void generateJson1(string& json, User& user) {
	  json += "{\"action\":1,\"data\":{\"id\":";
	  json += to_string(user.id);
	  json += ",\"name\":\"";
	  json += user.name;
	  json += "\"}}";
   }

   void generateJson2(string& json, User& user) {
	  json += "{\"action\":2,\"data\":{\"users\":[{\"id\":";
	  json += to_string(user.id);
	  json += ",\"name\":\"";
	  json += user.name;
	  json += "\",\"team\":\"";
	  if (user.team == nullptr) {
		 json += "none";
	  }
	  else {
		 json += user.team->name;
	  }
	  json += "\"}]}}";
   }

   void generateJson2(string& json) {
	  json += "{\"action\":2,\"data\":{\"users\":[";

	  bool empty = true;
	  for (std::map<unsigned int, User*>::iterator iter = all_users.begin(); iter != all_users.end(); ++iter)
	  {
		 if (!empty) {
			json += ",";
		 }
		 json += "{\"id\":";
		 json += to_string(iter->first);
		 json += ",\"name\":\"";
		 json += iter->second->name;
		 json += "\",\"team\":\"";
		 if (iter->second->team == nullptr) {
			json += "none";
		 }
		 else {
			json += iter->second->team->name;
		 }
		 json += "\"}";
		 empty = false;
	  }
	  json += "]}}";
   }

   void generateJson3(string& json, Team& team) {
	  json += "{\"action\":3,\"data\":{\"teams\":[";
	  json += "{\"id\":";
	  json += to_string(team.id);
	  json += ",\"name\":\"";
	  json += team.name;
	  json += "\",\"owner\":{\"id\":";
	  json += to_string(team.owner->id);
	  json += ",\"name\":\"";
	  json += team.owner->name;
	  json += "\"}}]}}";
   }

   void generateJson3(string& json) {
	  json += "{\"action\":3,\"data\":{\"teams\":[";

	  bool empty = true;
	  for (std::map<unsigned int, Team*>::iterator iter = all_teams.begin(); iter != all_teams.end(); ++iter)
	  {
		 if (!empty) {
			json += ",";
		 }
		 json += "{\"id\":";
		 json += to_string(iter->first);
		 json += ",\"name\":\"";
		 json += iter->second->name;
		 json += "\",\"owner\":{\"id\":";
		 json += to_string(iter->second->owner->id);
		 json += ",\"name\":\"";
		 json += iter->second->owner->name;
		 json += "\"}}";
		 empty = false;
	  }

	  json += "]}}";
   }

   void generateJson4(string& json, Tour& tour) {
	  json += "{\"action\":4,\"data\":{\"tours\":[";
	  json += "{\"id\":";
	  json += to_string(tour.id);
	  json += ",\"name\":\"";
	  json += tour.name;


	  json += "\",\"home\":{\"id\":";
	  json += to_string(tour.home->id);
	  json += ",\"name\":\"";
	  json += tour.home->name;

	  json += "\"},\"away\":{\"id\":";
	  json += to_string(tour.away->id);
	  json += ",\"name\":\"";
	  json += tour.away->name;


	  json += "\"},\"owner\":{\"id\":";
	  json += to_string(tour.owner->id);
	  json += ",\"name\":\"";
	  json += tour.owner->name;
	  json += "\"}}]}}";
   }

   void generateJson4(string& json) {
	  json += "{\"action\":4,\"data\":{\"tours\":[";

	  bool empty = true;
	  for (std::map<unsigned int, Tour*>::iterator iter = all_tours.begin(); iter != all_tours.end(); ++iter)
	  {
		 if (!empty) {
			json += ",";
		 }
		 json += "{\"id\":";
		 json += to_string(iter->first);
		 json += ",\"name\":\"";
		 json += iter->second->name;


		 json += "\",\"home\":{\"id\":";
		 json += to_string(iter->second->home->id);
		 json += ",\"name\":\"";
		 json += iter->second->home->name;

		 json += "\"},\"away\":{\"id\":";
		 json += to_string(iter->second->away->id);
		 json += ",\"name\":\"";
		 json += iter->second->away->name;


		 json += "\"},\"owner\":{\"id\":";
		 json += to_string(iter->second->owner->id);
		 json += ",\"name\":\"";
		 json += iter->second->owner->name;
		 json += "\"}}";
		 empty = false;
	  }

	  json += "]}}";
   }

   void generateJson7(string& json, Team& team) {
	  json += "{\"action\":7,\"data\":{\"teams\":[";
	  json += "{\"id\":";
	  json += to_string(team.id);
	  json += "}]}}";
   }


   virtual bool handleConnection(CivetServer* server,
	  const struct mg_connection* conn) {
	  printf("WS connected\n");
	  return true;
   }

   virtual void handleReadyState(CivetServer* server,
	  struct mg_connection* conn) {
	  printf("WS ready\n");

	  string response = "{\"actions\":[";
	  generateJson2(response);
	  response += ",";
	  generateJson3(response);
	  response += ",";
	  generateJson4(response);
	  response += "]}";
	  mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, response.c_str(), response.length());
   }

   virtual bool handleData(CivetServer* server,
	  struct mg_connection* conn,
	  int bits,
	  char* data,
	  size_t data_len) {

	  if (data_len <= 2) {
		 return false;
	  }
	  printf("WS got %lu bytes: \r\n", (long unsigned)data_len);
	  data[data_len] = '\0';

	  cout << data << endl;

	  unsigned int idx = 0;
	  JsonObject jsonRoot;
	  parseObject(jsonRoot, data, idx, data_len);
	  vector<JsonValue*>* jsonArray = jsonRoot.m_nameValuePair.at(0)->jsonValue.m_data.u_jsonArray;
	  for (auto& iter : *jsonArray) {
		 JsonObject* jsonObject = iter->m_data.u_jsonObject;
		 if (jsonObject->m_nameValuePair.at(0)->name._Equal("action")) {
			int action = stoi(*jsonObject->m_nameValuePair.at(0)->jsonValue.m_data.u_number);
			JsonObject* jsonData = jsonObject->m_nameValuePair.at(1)->jsonValue.m_data.u_jsonObject;
			switch (action)
			{
			case 1://Authenticate
			   if (jsonData->m_nameValuePair.size() >= 1) {
				  User* user = nullptr;

				  bool idSet = false;
				  unsigned int id = 0;

				  string* name = nullptr;

				  for (auto& value : jsonData->m_nameValuePair) {
					 if (value->name._Equal("id")) {
						id = stoi(*value->jsonValue.m_data.u_number);
						idSet = true;
					 }
					 else if (value->name._Equal("name")) {
						name = value->jsonValue.m_data.u_string;
					 }
				  }

				  if (!idSet || id >= clientCounter) {
					 id = clientCounter++;
					 idSet = true;
					 user = new User();
					 all_users[id] = user;
				  }
				  else {
					 user = all_users[id];
				  }

				  user->id = id;
				  if (name != nullptr) {
					 user->name = *name;
				  }
				  if (user->name._Equal("")) {
					 user->name = "User ";
					 user->name += to_string(id);
				  }
				  user->conn = conn;

				  string response = "{\"actions\":[";
				  generateJson1(response, *user);
				  response += "]}";
				  mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, response.c_str(), response.length());


				  response = "{\"actions\":[";
				  generateJson2(response, *user);
				  response += "]}";
				  broadcast(response);
			   }
			   break;
			case 2: //User/Team/Tour info
			   break;
			case 3: //Create team
			   break;
			case 4:
			   break;
			case 5:
			   if (jsonData->m_nameValuePair.size() >= 1) {
				  string* name = nullptr;
				  unsigned int ownerId = 0;

				  for (auto& value : jsonData->m_nameValuePair) {
					 if (value->name._Equal("owner-id")) {
						ownerId = stoi(*value->jsonValue.m_data.u_number);
					 }
					 else if (value->name._Equal("name")) {
						name = value->jsonValue.m_data.u_string;
					 }
				  }

				  unsigned int id = teamCounter++;
				  Team* newTeam = new Team();
				  newTeam->id = id;
				  if (name != nullptr) {
					 newTeam->name = *name;
				  }
				  if (newTeam->name._Equal("")) {
					 newTeam->name = "Team ";
					 newTeam->name += to_string(id);
				  }
				  User* owner = all_users[ownerId];
				  newTeam->owner = owner;
				  newTeam->users.push_back(owner);

				  all_teams[id] = newTeam;

				  string response = "{\"actions\":[";
				  generateJson3(response, *newTeam);
				  response += "]}";
				  broadcast(response);
			   }
			   break;
			case 6:
			   if (jsonData->m_nameValuePair.size() >= 1) {
				  string* name = nullptr;
				  unsigned int homeId = 0;
				  unsigned int awayId = 0;
				  unsigned int ownerId = 0;

				  for (auto& value : jsonData->m_nameValuePair) {
					 if (value->name._Equal("home-id")) {
						homeId = stoi(*value->jsonValue.m_data.u_number);
					 }
					 else if (value->name._Equal("away-id")) {
						awayId = stoi(*value->jsonValue.m_data.u_number);
					 }
					 else if (value->name._Equal("owner-id")) {
						ownerId = stoi(*value->jsonValue.m_data.u_number);
					 }
					 else if (value->name._Equal("name")) {
						name = value->jsonValue.m_data.u_string;
					 }
				  }

				  unsigned int id = tourCounter++;
				  Tour* newTour = new Tour();
				  newTour->id = id;
				  if (name != nullptr) {
					 newTour->name = *name;
				  }
				  if (newTour->name._Equal("")) {
					 newTour->name = "Tour ";
					 newTour->name += to_string(id);
				  }

				  Team* home = all_teams[homeId];
				  newTour->home = home;

				  Team* away = all_teams[awayId];
				  newTour->away = away;

				  User* owner = all_users[ownerId];
				  newTour->owner = owner;

				  all_tours[id] = newTour;

				  string response = "{\"actions\":[";
				  generateJson4(response, *newTour);
				  response += "]}";
				  broadcast(response);
			   }
			   break;
			case 7:
			   if (jsonData->m_nameValuePair.size() >= 1) {
				  unsigned int id = 0;

				  for (auto& value : jsonData->m_nameValuePair) {
					 if (value->name._Equal("id")) {
						id = stoi(*value->jsonValue.m_data.u_number);
					 }
				  }

				  string response = "{\"actions\":[";
				  generateJson7(response, *all_teams[id]);
				  response += "]}";
				  broadcast(response);

				  all_teams.erase(id);
			   }
			   break;
			case 8:
			   break;
			case 9:
			   break;
			}
		 }
	  }



	  /*string json = "";
	  stringifyObject(json, jsonObject, false);
	  cout << json << endl;
	  json = "";
	  stringifyObject(json, jsonObject, true);
	  cout << json << endl;*/
	  /*
	  for (int i = 0; i < data_len; i++) {
		 gameData[i] = data[i];
	  }
	  gameData[data_len] = '\0';

	  printf(gameData);

	  for (auto & value : all_connections) {
		 if (value != conn) {
			mg_websocket_write(value, MG_WEBSOCKET_OPCODE_TEXT, gameData, strlen(gameData));
		 }
	  }
	  */

	  return true;
   }

   virtual void handleClose(CivetServer* server,
	  const struct mg_connection* conn) {
	  printf("WS closed\n");
   }
};





int
main(int argc, char* argv[])
{
   mg_init_library(0);

   const char* options[] = {
	   "document_root", DOCUMENT_ROOT, "listening_ports", PORT, "ssl_certificate", SSL_CERTIFICATE_PATH, 0 };

   std::vector<std::string> cpp_options;
   for (int i = 0; i < (sizeof(options) / sizeof(options[0]) - 1); i++) {
	  cpp_options.push_back(options[i]);
   }

   // CivetServer server(options); // <-- C style start
   CivetServer server(cpp_options); // <-- C++ style start

   WebsocketHandler h_websocket;
   server.addWebSocketHandler("/websocket", h_websocket);
   printf("Run websocket example at http://localhost:%s/ws\n", PORT);

   printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
   printf("Exit at http://localhost:%s%s\n", PORT, EXIT_URI);

   while (!exitNow) {
#ifdef _WIN32
	  Sleep(1000);
#else
	  sleep(1);
#endif
   }

   printf("Bye!\n");
   mg_exit_library();

   return 0;
}
