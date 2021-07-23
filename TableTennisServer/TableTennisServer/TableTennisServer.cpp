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
#define SSL_CERTIFICATE_PATH "./server.pem"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"


#define FORMAT_JSON_OUTPUT true

/* Exit flag for main loop */
volatile bool exitNow = false;

unsigned int clientCounter = 0;
unsigned int teamCounter = 0;
unsigned int tourCounter = 0;
std::map<unsigned int, User*> all_users;
std::map<unsigned int, Team*> all_teams;
std::map<unsigned int, Tour*> all_tours;


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
   
   char gameData[1000] = "";

   void broadcast(string& message) {
	  for (std::map<unsigned int, User*>::iterator iter = all_users.begin(); iter != all_users.end(); ++iter)
	  {
		 mg_websocket_write(iter->second->conn, MG_WEBSOCKET_OPCODE_TEXT, message.c_str(), message.length());
	  }
   }

   virtual bool handleConnection(CivetServer* server,
	  const struct mg_connection* conn) {
	  printf("WS connected\n");
	  return true;
   }

   virtual void handleReadyState(CivetServer* server,
	  struct mg_connection* conn) {
	  printf("WS ready\n");
   }

   void sendUpdatedUser(User* user) {
	  JsonObject jo_root;
	  JsonObject* jo_update= new JsonObject();
	  vector<JsonValue*>* ja_users = new vector<JsonValue*>();


	  JsonValue* jv_user = new JsonValue();
	  jv_user->m_type = JsonValueTypeObject;

	  JsonObject* jo_user = new JsonObject();
	  user->generateJson(*jo_user);
	  jv_user->m_data.u_jsonObject = jo_user;

	  ja_users->push_back(jv_user);


	  jo_update->AddNameValuePairArray("users", ja_users);
	  jo_root.AddNameValuePairObject("update", jo_update);



	  string response = "";
	  stringifyObject(response, jo_root, FORMAT_JSON_OUTPUT);
	  broadcast(response);
	  cout << "<- " << response << endl;
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

	  cout << "-> " << data << endl;

	  unsigned int idx = 0;
	  JsonObject jsonRoot;
	  parseObject(jsonRoot, data, idx, data_len);
	  for (auto& request : jsonRoot.m_nameValuePair) {
		 if (request->name._Equal("auth")) {
			User* user = nullptr;

			bool idSet = false;
			unsigned int id = 0;

			string* name = nullptr;

			for (auto& value : request->jsonValue.m_data.u_jsonObject->m_nameValuePair) {
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


			JsonObject out_jo_root;
			JsonObject* out_jo_auth = new JsonObject();
			user->generateJson(*out_jo_auth);

			out_jo_root.AddNameValuePairObject("auth", out_jo_auth);

			string response;
			stringifyObject(response, out_jo_root, FORMAT_JSON_OUTPUT);
			mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, response.c_str(), response.length());
			cout << "<- " << response << endl;


			sendUpdatedUser(user);


		 }
		 else if (request->name._Equal("get")) {
			JsonObject out_jo_root;
			JsonObject* out_jo_get = new JsonObject();

			vector<JsonValue*>* out_ja_users = new vector<JsonValue*>();
			vector<JsonValue*>* out_ja_teams = new vector<JsonValue*>();
			vector<JsonValue*>* out_ja_tours = new vector<JsonValue*>();

			for (auto& data : request->jsonValue.m_data.u_jsonObject->m_nameValuePair) {
			   if (data->name._Equal("users")) {
				  auto* userData = data->jsonValue.m_data.u_jsonArray;
				  if (userData->size() == 0) {

					 for (std::map<unsigned int, User*>::iterator user = all_users.begin(); user != all_users.end(); ++user)
					 {
						JsonValue* out_jv_user = new JsonValue();
						out_jv_user->m_type = JsonValueTypeObject;

						JsonObject* out_jo_user = new JsonObject();

						user->second->generateJson(*out_jo_user);

						out_jv_user->m_data.u_jsonObject = out_jo_user;

						out_ja_users->push_back(out_jv_user);
					 }
				  }
			   }
			   else if (data->name._Equal("teams")) {
				  auto* teamData = data->jsonValue.m_data.u_jsonArray;
				  if (teamData->size() == 0) {

					 for (std::map<unsigned int, Team*>::iterator team = all_teams.begin(); team != all_teams.end(); ++team)
					 {
						JsonValue* out_jv_team = new JsonValue();
						out_jv_team->m_type = JsonValueTypeObject;

						JsonObject* out_jo_team = new JsonObject();

						team->second->generateJson(*out_jo_team);

						out_jv_team->m_data.u_jsonObject = out_jo_team;

						out_ja_teams->push_back(out_jv_team);
					 }
				  }
			   }
			   else if (data->name._Equal("tours")) {
				  auto* tourData = data->jsonValue.m_data.u_jsonArray;
				  if (tourData->size() == 0) {

					 for (std::map<unsigned int, Tour*>::iterator tour = all_tours.begin(); tour != all_tours.end(); ++tour)
					 {
						JsonValue* out_jv_tour = new JsonValue();
						out_jv_tour->m_type = JsonValueTypeObject;

						JsonObject* out_jo_tour = new JsonObject();

						tour->second->generateJson(*out_jo_tour);

						out_jv_tour->m_data.u_jsonObject = out_jo_tour;

						out_ja_tours->push_back(out_jv_tour);
					 }

				  }
				  else {
					 for (auto& jv_tour : *tourData) {
						JsonValue* out_jv_tour = new JsonValue();
						out_jv_tour->m_type = JsonValueTypeObject;

						JsonObject* out_jo_tour = new JsonObject();

						


						JsonObject* jo_tour = jv_tour->m_data.u_jsonObject;
						unsigned int id = 0;
						vector<JsonValue*>* ja_matches = nullptr;
						for (auto& nvp_tour : jo_tour->m_nameValuePair) {
						   if (nvp_tour->name._Equal("id")) {
							  id = stoi(*nvp_tour->jsonValue.m_data.u_number);
						   }
						   else if (nvp_tour->name._Equal("matches")) {
							  ja_matches = nvp_tour->jsonValue.m_data.u_jsonArray;
						   }
						}

						Tour& tour = *all_tours[id];

						tour.generateJson(*out_jo_tour);

						if (ja_matches != nullptr) {
						   if (ja_matches->empty()) {

							  vector<JsonValue*>* out_ja_matches = new vector<JsonValue*>();

							  for (auto& matchIter : tour.matches) {
								 JsonValue* out_jv_match = new JsonValue();
								 out_jv_match->m_type = JsonValueTypeObject;

								 JsonObject* out_jo_match = new JsonObject();

								 matchIter.second.generateJson(*out_jo_match);

								 out_jv_match->m_data.u_jsonObject = out_jo_match;

								 out_ja_matches->push_back(out_jv_match);
							  }

							  out_jo_tour->AddNameValuePairArray("matches", out_ja_matches);
						   }
						}

						out_jv_tour->m_data.u_jsonObject = out_jo_tour;
						out_ja_tours->push_back(out_jv_tour);
					 }
				  }
			   }
			}

			if (!out_ja_users->empty())
			   out_jo_get->AddNameValuePairArray("users", out_ja_users);
			else
			   delete out_ja_users;

			if (!out_ja_tours->empty())
			   out_jo_get->AddNameValuePairArray("tours", out_ja_tours);
			else
			   delete out_ja_tours;

			if (!out_ja_teams->empty())
			   out_jo_get->AddNameValuePairArray("teams", out_ja_teams);
			else
			   delete out_ja_teams;

			out_jo_root.AddNameValuePairObject("get", out_jo_get);
			string response;
			stringifyObject(response, out_jo_root, FORMAT_JSON_OUTPUT);
			mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, response.c_str(), response.length());
			cout << "<- " << response << endl;
		 }
		 else if (request->name._Equal("update")) {
			JsonObject out_jo_root;
			JsonObject* out_jo_update = new JsonObject();

			vector<JsonValue*>* out_ja_users = new vector<JsonValue*>();

			for (auto& data : request->jsonValue.m_data.u_jsonObject->m_nameValuePair) {
			   if (data->name._Equal("users")) {
				  auto* ja_users = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_user : *ja_users) {
					 JsonObject* jo_user = jv_user->m_data.u_jsonObject;

					 unsigned int id = 0;
					 string* name = nullptr;
					 bool nameSet = false;
					 int teamId = -1;
					 bool teamIdSet = false;

					 for (auto& nvp_team : jo_user->m_nameValuePair) {
						if (nvp_team->name._Equal("id")) {
						   id = stoi(*nvp_team->jsonValue.m_data.u_number);
						}
						else if (nvp_team->name._Equal("name")) {
						   name = nvp_team->jsonValue.m_data.u_string;
						   nameSet = true;
						}
						else if (nvp_team->name._Equal("team-id")) {
						   teamId = stoi(*nvp_team->jsonValue.m_data.u_number);
						   teamIdSet = true;
						}
					 }

					 auto& user = all_users[id];

					 if (nameSet) {
						user->name = *name;
					 }
					 if (teamIdSet) {
						if (teamId == -1) {
						   user->team = nullptr;
						}
						else {
						   user->team = all_teams[teamId];
						   user->team->users.push_back(user);
						}
					 }



					 JsonValue* out_jv_user = new JsonValue();
					 out_jv_user->m_type = JsonValueTypeObject;
					 JsonObject* out_jo_user = new JsonObject();
					 user->generateJson(*out_jo_user);
					 out_jv_user->m_data.u_jsonObject = out_jo_user;
					 out_ja_users->push_back(out_jv_user);
				  }


			   }
			}

			if (!out_ja_users->empty())
			   out_jo_update->AddNameValuePairArray("users", out_ja_users);
			else
			   delete out_ja_users;
			


			out_jo_root.AddNameValuePairObject("update", out_jo_update);
			string response;
			stringifyObject(response, out_jo_root, FORMAT_JSON_OUTPUT);
			broadcast(response);
			cout << "<- " << response << endl;
		 }
		 else if (request->name._Equal("create")) {
			JsonObject out_jo_root;
			JsonObject* out_jo_create = new JsonObject();

			vector<JsonValue*>* out_ja_teams = new vector<JsonValue*>();
			vector<JsonValue*>* out_ja_tours = new vector<JsonValue*>();

			for (auto& data : request->jsonValue.m_data.u_jsonObject->m_nameValuePair) {
			   if (data->name._Equal("teams")) {
				  auto* ja_teams = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_team : *ja_teams) {
					 JsonObject* jo_team = jv_team->m_data.u_jsonObject;

					 string* name = nullptr;
					 unsigned int ownerId = 0;

					 for (auto& nvp_team: jo_team->m_nameValuePair) {
						if (nvp_team->name._Equal("owner-id")) {
						   ownerId = stoi(*nvp_team->jsonValue.m_data.u_number);
						}
						else if (nvp_team->name._Equal("name")) {
						   name = nvp_team->jsonValue.m_data.u_string;
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

					 all_teams[id] = newTeam;

					 JsonValue* out_jv_team = new JsonValue();
					 out_jv_team->m_type = JsonValueTypeObject;
					 JsonObject* out_jo_team = new JsonObject();
					 newTeam->generateJson(*out_jo_team);
					 out_jv_team->m_data.u_jsonObject = out_jo_team;
					 out_ja_teams->push_back(out_jv_team);
				  }
			   }else if (data->name._Equal("tours")) {
				  auto* ja_tours = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_tour : *ja_tours) {
					 JsonObject* jo_tour = jv_tour->m_data.u_jsonObject;


					 string* name = nullptr;
					 unsigned int homeId = 0;
					 unsigned int awayId = 0;
					 unsigned int ownerId = 0;

					 for (auto& nvp_tour : jo_tour->m_nameValuePair) {
						if (nvp_tour->name._Equal("home-id")) {
						   homeId = stoi(*nvp_tour->jsonValue.m_data.u_number);
						}
						else if (nvp_tour->name._Equal("away-id")) {
						   awayId = stoi(*nvp_tour->jsonValue.m_data.u_number);
						}
						else if (nvp_tour->name._Equal("owner-id")) {
						   ownerId = stoi(*nvp_tour->jsonValue.m_data.u_number);
						}
						else if (nvp_tour->name._Equal("name")) {
						   name = nvp_tour->jsonValue.m_data.u_string;
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



					 JsonValue* out_jv_tour = new JsonValue();
					 out_jv_tour->m_type = JsonValueTypeObject;
					 JsonObject* out_jo_tour = new JsonObject();
					 newTour->generateJson(*out_jo_tour);
					 out_jv_tour->m_data.u_jsonObject = out_jo_tour;
					 out_ja_tours->push_back(out_jv_tour);
				  }
			   }
			}

			if (!out_ja_tours->empty())
			   out_jo_create->AddNameValuePairArray("tours", out_ja_tours);
			else
			   delete out_ja_tours;

			if (!out_ja_teams->empty())
			   out_jo_create->AddNameValuePairArray("teams", out_ja_teams);
			else
			   delete out_ja_teams;

			out_jo_root.AddNameValuePairObject("create", out_jo_create);
			string response;
			stringifyObject(response, out_jo_root, FORMAT_JSON_OUTPUT);
			broadcast(response);
			cout << "<- " << response << endl;
		 }
		 else if (request->name._Equal("delete")) {
			JsonObject out_jo_root;
			JsonObject* out_jo_delete = new JsonObject();

			vector<JsonValue*>* out_ja_teams = new vector<JsonValue*>();
			vector<JsonValue*>* out_ja_tours = new vector<JsonValue*>();


			for (auto& data : request->jsonValue.m_data.u_jsonObject->m_nameValuePair) {
			   if (data->name._Equal("teams")) {
				  auto* ja_teams = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_team : *ja_teams) {
					 JsonObject* jo_team = jv_team->m_data.u_jsonObject;

					 unsigned int id = 0;

					 for (auto& nvp_team : jo_team->m_nameValuePair) {
						if (nvp_team->name._Equal("id")) {
						   id = stoi(*nvp_team->jsonValue.m_data.u_number);
						}
					 }

					 auto& team = all_teams[id];

					 for (vector<User*>::iterator user = team->users.begin(); user != team->users.end(); ++user) {
						(*user)->team = nullptr;
						sendUpdatedUser(*user);
					 }

					 team->users.clear();

					 for (std::map<unsigned int, Tour*>::iterator tourIter = all_tours.begin(); tourIter != all_tours.end(); ++tourIter) {
						unsigned int tourId = tourIter->first;
						Tour* tour = tourIter->second;

						if (tour->home == team || tour->away == team) {
						   JsonValue* out_jv_tour = new JsonValue();
						   out_jv_tour->m_type = JsonValueTypeObject;
						   JsonObject* out_jo_tour = new JsonObject();
						   out_jo_tour->AddNameValuePairNumber("id", tourId);
						   out_jv_tour->m_data.u_jsonObject = out_jo_tour;
						   out_ja_tours->push_back(out_jv_tour);


						   all_tours.erase(tourId);
						   free(tour);
						   tourIter = all_tours.begin();
						   if (tourIter == all_tours.end()) {
							  break;
						   }
						}
					 }
					 

					 JsonValue* out_jv_team = new JsonValue();
					 out_jv_team->m_type = JsonValueTypeObject;
					 JsonObject* out_jo_team = new JsonObject();
					 out_jo_team->AddNameValuePairNumber("id", id);
					 out_jv_team->m_data.u_jsonObject = out_jo_team;
					 out_ja_teams->push_back(out_jv_team);

					 all_teams.erase(id);
				  }
			   }else if (data->name._Equal("tours")) {
				  auto* ja_tours = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_tour : *ja_tours) {
					 JsonObject* jo_tour = jv_tour->m_data.u_jsonObject;

					 unsigned int id = 0;

					 for (auto& nvp_tour : jo_tour->m_nameValuePair) {
						if (nvp_tour->name._Equal("id")) {
						   id = stoi(*nvp_tour->jsonValue.m_data.u_number);
						}
					 }

					 JsonValue* out_jv_tour = new JsonValue();
					 out_jv_tour->m_type = JsonValueTypeObject;
					 JsonObject* out_jo_tour = new JsonObject();
					 out_jo_tour->AddNameValuePairNumber("id", id);
					 out_jv_tour->m_data.u_jsonObject = out_jo_tour;
					 out_ja_tours->push_back(out_jv_tour);

					 all_tours.erase(id);
				  }
			   }
			}

			if (!out_ja_tours->empty())
			   out_jo_delete->AddNameValuePairArray("tours", out_ja_tours);
			else
			   delete out_ja_tours;

			if (!out_ja_teams->empty())
			   out_jo_delete->AddNameValuePairArray("teams", out_ja_teams);
			else
			   delete out_ja_teams;

			out_jo_root.AddNameValuePairObject("delete", out_jo_delete);
			string response;
			stringifyObject(response, out_jo_root, FORMAT_JSON_OUTPUT);
			broadcast(response);
			cout << "<- " << response << endl;
		 }
	  }
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
