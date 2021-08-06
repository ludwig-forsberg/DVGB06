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

#define USE_DB

#ifdef USE_DB
#include"mysql.h"
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
						   vector<JsonValue*>* out_ja_matches = new vector<JsonValue*>();
						   if (ja_matches->empty()) {


							  for (auto& matchIter : tour.matches) {
								 JsonValue* out_jv_match = new JsonValue();
								 out_jv_match->m_type = JsonValueTypeObject;

								 JsonObject* out_jo_match = new JsonObject();

								 matchIter.second.generateJson(*out_jo_match);

								 out_jv_match->m_data.u_jsonObject = out_jo_match;

								 out_ja_matches->push_back(out_jv_match);
							  }

						   }
						   else {
							  for (auto& jv_match : *ja_matches) {
								 unsigned int matchId = 0;
								 for (auto& nvp_match : jv_match->m_data.u_jsonObject->m_nameValuePair) {
									if (nvp_match->name._Equal("id")) {
									   matchId = stoi(*nvp_match->jsonValue.m_data.u_number);
									}
								 }

								 auto& match = tour.matches[matchId];

								 JsonValue* out_jv_match = new JsonValue();
								 out_jv_match->m_type = JsonValueTypeObject;

								 JsonObject* out_jo_match = new JsonObject();

								 match.generateJson(*out_jo_match);

								 out_jv_match->m_data.u_jsonObject = out_jo_match;

								 out_ja_matches->push_back(out_jv_match);
							  }
						   }
						   out_jo_tour->AddNameValuePairArray("matches", out_ja_matches);

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
			vector<JsonValue*>* out_ja_tours = new vector<JsonValue*>();


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
						   if (user->team != nullptr) {
							  vector<User*>::iterator position = find(user->team->users.begin(), user->team->users.end(), user);
							  if (position != user->team->users.end()) {
								 user->team->users.erase(position);
							  }
						   }
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
			   else if (data->name._Equal("tours")) {
				  auto* ja_tours = data->jsonValue.m_data.u_jsonArray;
				  for (auto& jv_tour : *ja_tours) {
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

					 out_jo_tour->AddNameValuePairNumber("id", id);

					 auto& tour = all_tours[id];



					 if (ja_matches != nullptr) {
						vector<JsonValue*>* out_ja_matches = new vector<JsonValue*>();

						for (auto& jv_match : *ja_matches) {
						   JsonValue* out_jv_match = new JsonValue();
						   //JsonObject* out_jo_match = new JsonObject();
						   out_jv_match->m_type = JsonValueTypeObject;
						   //out_jv_match->m_data.u_jsonObject = out_jo_match;


						   JsonObject* jo_match = jv_match->m_data.u_jsonObject;
						   unsigned int match_id = 0;

						   vector<JsonValue*>* ja_players_team_1_ids = nullptr;
						   vector<JsonValue*>* ja_players_team_2_ids = nullptr;
						   vector<JsonValue*>* actions = nullptr;

						   for (auto& nvp_match : jo_match->m_nameValuePair) {
							  if (nvp_match->name._Equal("id")) {
								 match_id = stoi(*nvp_match->jsonValue.m_data.u_number);
							  }
							  else if (nvp_match->name._Equal("players-team-1-ids")) {
								 ja_players_team_1_ids = nvp_match->jsonValue.m_data.u_jsonArray;
							  }
							  else if (nvp_match->name._Equal("players-team-2-ids")) {
								 ja_players_team_2_ids = nvp_match->jsonValue.m_data.u_jsonArray;
							  }
							  else if (nvp_match->name._Equal("actions")) {
								 actions = nvp_match->jsonValue.m_data.u_jsonArray;
							  }
						   }

						   //out_jo_match->AddNameValuePairNumber("id", match_id);

						   auto& match = tour->matches[match_id];

						   if (ja_players_team_1_ids != nullptr) {
							  match.playersTeam1.clear();

							  //vector<JsonValue*>* out_ja_players_team_1_ids = new vector<JsonValue*>();


							  for (auto& jv_id : *ja_players_team_1_ids) {
								 unsigned int player_id = stoi(*jv_id->m_data.u_number);
								 match.playersTeam1.push_back(all_users[player_id]);
								 //JsonValue* out_jv_players_team_1_id = new JsonValue();
								 //out_jv_players_team_1_id->m_type = JsonValueTypeNumber;
								 //out_jv_players_team_1_id->m_data.u_number = new string(to_string(player_id));
								 //out_ja_players_team_1_ids->push_back(out_jv_players_team_1_id);
							  }
							  //out_jo_match->AddNameValuePairArray("players-team-1-ids", out_ja_players_team_1_ids);
						   }

						   if (ja_players_team_2_ids != nullptr) {
							  match.playersTeam2.clear();

							  //vector<JsonValue*>* out_ja_players_team_2_ids = new vector<JsonValue*>();


							  for (auto& jv_id : *ja_players_team_2_ids) {
								 unsigned int player_id = stoi(*jv_id->m_data.u_number);
								 match.playersTeam2.push_back(all_users[player_id]);
								 //JsonValue* out_jv_players_team_2_id = new JsonValue();
								 //out_jv_players_team_2_id->m_type = JsonValueTypeNumber;
								 //out_jv_players_team_2_id->m_data.u_number = new string(to_string(player_id));
								 //out_ja_players_team_2_ids->push_back(out_jv_players_team_2_id);
							  }
							  //out_jo_match->AddNameValuePairArray("players-team-2-ids", out_ja_players_team_2_ids);
						   }

						   if (actions != nullptr) {
							  unsigned int sets_count = match.sets.size();
							  auto& currentSet = match.sets[sets_count - 1];
							  auto& team1Points = currentSet.team1Points;
							  auto& team2Points = currentSet.team2Points;

							  for (auto& jv_action : *actions) {
								 auto& action = *jv_action->m_data.u_string;
								 if (action._Equal("team-1-add-point")) {
									if (match.team1SetPoints + match.team2SetPoints == match.numberOfSets) continue;

									team1Points++;
									if (team1Points >= 11 && team1Points >= team2Points + 2) {
									   if (sets_count < match.numberOfSets) {
										  match.sets.push_back(Set(sets_count));
									   }
									   match.team1SetPoints++;
									}
								 }
								 else if (action._Equal("team-2-add-point")) {
									if (match.team1SetPoints + match.team2SetPoints == match.numberOfSets) continue;

									team2Points++;
									if (team2Points >= 11 && team2Points >= team1Points + 2) {
									   if (sets_count < match.numberOfSets) {
										  match.sets.push_back(Set(sets_count));
									   }
									   match.team2SetPoints++;
									}
								 }
								 else if (action._Equal("team-1-subtract-point")) {
									if (match.team1SetPoints + match.team2SetPoints == match.numberOfSets) continue;

									if (team1Points > 0) {
									   team1Points--;
									   if (team2Points >= 11 && team2Points >= team1Points + 2) {
										  if (sets_count < match.numberOfSets) {
											 match.sets.push_back(Set(sets_count));
										  }
										  match.team2SetPoints++;
									   }
									}

								 }
								 else if (action._Equal("team-2-subtract-point")) {
									if (match.team1SetPoints + match.team2SetPoints == match.numberOfSets) continue;

									if (team2Points > 0) {
									   team2Points--;
									   if (team1Points >= 11 && team1Points >= team2Points + 2) {
										  if (sets_count < match.numberOfSets) {
											 match.sets.push_back(Set(sets_count));
										  }
										  match.team1SetPoints++;
									   }
									}
								 }
								 else if (action._Equal("reset")) {
									match.sets.clear();
									match.sets.push_back(Set(0));
								 }
								 else if (action._Equal("swap-sides")) {
									match.originalSide = !match.originalSide;
								 }
								 else if (action._Equal("swap-serve")) {
									match.originalServe = !match.originalServe;
								 }
							  }
						   }

						   JsonObject* out_jo_match = new JsonObject();

						   match.generateJson(*out_jo_match);

						   out_jv_match->m_data.u_jsonObject = out_jo_match;


						   out_ja_matches->push_back(out_jv_match);
						}

						out_jo_tour->AddNameValuePairArray("matches", out_ja_matches);
					 }

					 
					 out_jv_tour->m_data.u_jsonObject = out_jo_tour;
					 out_ja_tours->push_back(out_jv_tour);
				  }
			   }
			}

			if (!out_ja_users->empty())
			   out_jo_update->AddNameValuePairArray("users", out_ja_users);
			else
			   delete out_ja_users;
			
			if (!out_ja_tours->empty())
			   out_jo_update->AddNameValuePairArray("tours", out_ja_tours);
			else
			   delete out_ja_tours;

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

#ifdef USE_DB
MYSQL* conn;

int db_connect() {
   conn = mysql_init(0);
   conn = mysql_real_connect(conn, "localhost", "root", "password", "", 3306, NULL, 0);
   if (!conn) {
	  puts("Connection to database has failed!");
	  return 1;
   }
   
   return 0;
}

int db_load() {
   MYSQL_ROW row;
   MYSQL_RES* res;

   string query = "SELECT COUNT(*) FROM information_schema.schemata WHERE SCHEMA_NAME='table_tennis'";
   int qstate = mysql_query(conn, query.c_str());
   if (!qstate)
   {
	  res = mysql_store_result(conn);
	  row = mysql_fetch_row(res);

	  if (strcmp(row[0], "1") == 0) {
		 printf("table_tennis exists\n");
		 
		 if (!mysql_query(conn, "SELECT `user_id`,`user_name` FROM table_tennis.users")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   auto user = new User();
			   unsigned int id = stoi(row[0]);
			   user->id = id;
			   user->name = row[1];
			   all_users[id] = user;
			   clientCounter++;
			}
		 }
		 else {
			cout << "users was not found: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (!mysql_query(conn, "SELECT `team_id`,`team_name`,`team_owner_id` FROM table_tennis.teams")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   auto team = new Team();
			   unsigned int id = stoi(row[0]);
			   team->id = id;
			   team->name = row[1];
			   team->owner = all_users[stoi(row[2])];
			   all_teams[id] = team;
			   teamCounter++;
			}
		 }
		 else {
			cout << "teams was not found: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (!mysql_query(conn, "SELECT `user_id`,`user_team_id` FROM table_tennis.users")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   if (row[1] != NULL) {
				  auto user = all_users[stoi(row[0])];
				  auto team = all_teams[stoi(row[1])];
				  user->team = team;
				  team->users.push_back(user);

			   }
			}
		 }
		 else {
			cout << "users was not found: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (!mysql_query(conn, "SELECT * FROM table_tennis.tournaments")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   auto tour = new Tour();
			   unsigned int id = stoi(row[0]);
			   unsigned int homeTeamId = stoi(row[2]);
			   unsigned int awayTeamId = stoi(row[3]);
			   unsigned int ownerId = stoi(row[4]);

			   tour->id = id;
			   tour->name = row[1];
			   tour->home = all_teams[homeTeamId];
			   tour->away = all_teams[awayTeamId];
			   tour->owner = all_users[ownerId];

			   all_tours[id] = tour;
			   tourCounter++;
			}
		 }
		 else {
			cout << "tournaments was not found: " << mysql_error(conn) << endl;
			return 1;
		 }


		 if (!mysql_query(conn, "SELECT * FROM table_tennis.matches")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   unsigned int id = stoi(row[0]);
			   unsigned int tourId = stoi(row[1]);
			   unsigned int team1SetPoints = stoi(row[2]);
			   unsigned int team2SetPoints = stoi(row[3]);
			   unsigned int playersPerTeam = stoi(row[4]);
			   unsigned int numberOfSets = stoi(row[5]);
			   bool originalSide = stoi(row[6]);
			   bool originalServe = stoi(row[7]);

			   auto tour = all_tours[tourId];

			   tour->matches[id] = Match(id);
			   auto& match = tour->matches[id];

			   match.team1SetPoints = team1SetPoints;
			   match.team2SetPoints = team2SetPoints;
			   match.playersPerTeam = playersPerTeam;
			   match.numberOfSets = numberOfSets;
			   match.originalSide = originalSide;
			   match.originalServe = originalServe;
			}
		 }
		 else {
			cout << "matches was not found: " << mysql_error(conn) << endl;
			return 1;
		 }


		 if (!mysql_query(conn, "SELECT * FROM table_tennis.match_users")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   unsigned int id = stoi(row[0]);
			   unsigned int tourId = stoi(row[1]);
			   unsigned int matchId = stoi(row[2]);
			   unsigned int userId = stoi(row[3]);
			   bool fromTeam1 = stoi(row[4]);

			   auto& match = all_tours[tourId]->matches[matchId];
			   if (fromTeam1) {
				  match.playersTeam1.push_back(all_users[userId]);
			   }
			   else {
				  match.playersTeam2.push_back(all_users[userId]);
			   }
			}
		 }
		 else {
			cout << "match_users was not found: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (!mysql_query(conn, "SELECT * FROM table_tennis.sets")) {
			res = mysql_store_result(conn);
			while (row = mysql_fetch_row(res)) {
			   unsigned int id = stoi(row[0]);
			   unsigned int tourId = stoi(row[1]);
			   unsigned int matchId = stoi(row[2]);

			   unsigned int team1Points = stoi(row[3]);
			   unsigned int team2Points = stoi(row[4]);

			   auto& match = all_tours[tourId]->matches[matchId];
			   Set set = Set(id);
			   set.team1Points = team1Points;
			   set.team2Points = team2Points;
			   match.sets.push_back(set);
			}
		 }
		 else {
			cout << "sets was not found: " << mysql_error(conn) << endl;
			return 1;
		 }

	  }
	  else {
		 printf("table_tennis does not exist\n");

		 if (mysql_query(conn, "CREATE SCHEMA `table_tennis` DEFAULT CHARACTER SET utf8;")) {
			cout << "table_tennis creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }
		 
		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`users` (`user_id` INT NOT NULL, `user_name` VARCHAR(45) NOT NULL, `user_team_id` INT NULL, PRIMARY KEY (`user_id`));")) {
			cout << "users creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`teams` (`team_id` INT NOT NULL,`team_name` VARCHAR(45) NOT NULL,`team_owner_id` INT NOT NULL,PRIMARY KEY (`team_id`),INDEX `teams_users_idx` (`team_owner_id` ASC) VISIBLE,CONSTRAINT `teams_users` FOREIGN KEY (`team_owner_id`) REFERENCES `table_tennis`.`users`(`user_id`) ON DELETE NO ACTION ON UPDATE NO ACTION);")) {
			cout << "teams creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "ALTER TABLE `table_tennis`.`users` ADD INDEX `users_teams_idx` (`user_team_id` ASC) VISIBLE;")) {
			cout << "users alteration failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "ALTER TABLE `table_tennis`.`users` ADD CONSTRAINT `users_teams` FOREIGN KEY (`user_team_id`) REFERENCES `table_tennis`.`teams` (`team_id`) ON DELETE NO ACTION ON UPDATE NO ACTION;")) {
			cout << "users alteration 2 failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`tournaments` (`tournament_id` INT NOT NULL,`tournament_name` VARCHAR(45) NOT NULL,`tournament_home_team_id` INT NOT NULL,`tournament_away_team_id` INT NOT NULL,`tournament_owner_id` INT NOT NULL,PRIMARY KEY (`tournament_id`),INDEX `tournaments_users_idx` (`tournament_owner_id` ASC) VISIBLE,INDEX `tournaments_home_teams_idx` (`tournament_home_team_id` ASC) VISIBLE,INDEX `tournaments_away_teams_idx` (`tournament_away_team_id` ASC) VISIBLE,CONSTRAINT `tournaments_users` FOREIGN KEY (`tournament_owner_id`) REFERENCES `table_tennis`.`users` (`user_id`) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT `tournaments_home_teams` FOREIGN KEY (`tournament_home_team_id`) REFERENCES `table_tennis`.`teams` (`team_id`) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT `tournaments_away_teams` FOREIGN KEY (`tournament_away_team_id`) REFERENCES `table_tennis`.`teams` (`team_id`) ON DELETE NO ACTION ON UPDATE NO ACTION);")) {
			cout << "tournaments creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`matches` ( `match_id` INT NOT NULL, `match_tournament_id` INT NOT NULL, `match_team_1_set_points` INT NOT NULL, `match_team_2_set_points` INT NOT NULL, `match_players_per_team` INT NOT NULL, `match_number_of_sets` INT NOT NULL, `match_original_side` TINYINT NOT NULL, `match_original_serve` TINYINT NOT NULL, PRIMARY KEY (`match_id`, `match_tournament_id`), INDEX `matches_tournaments_idx` (`match_tournament_id` ASC) VISIBLE, CONSTRAINT `matches_tournaments` FOREIGN KEY (`match_tournament_id`) REFERENCES `table_tennis`.`tournaments` (`tournament_id`) ON DELETE NO ACTION ON UPDATE NO ACTION);")) {
			cout << "matches creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`sets` ( `set_id` INT NOT NULL, `set_tournament_id` INT NOT NULL, `set_match_id` INT NOT NULL, `set_team_1_points` INT NOT NULL, `set_team_2_points` INT NOT NULL, PRIMARY KEY (`set_id`, `set_tournament_id`, `set_match_id`), INDEX `sets_tournaments_idx` (`set_tournament_id` ASC) VISIBLE, INDEX `sets_matches_idx` (`set_match_id` ASC) VISIBLE, CONSTRAINT `sets_tournaments` FOREIGN KEY (`set_tournament_id`) REFERENCES `table_tennis`.`tournaments` (`tournament_id`) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT `sets_matches` FOREIGN KEY (`set_match_id`) REFERENCES `table_tennis`.`matches` (`match_id`) ON DELETE NO ACTION ON UPDATE NO ACTION);")) {
			cout << "sets creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 if (mysql_query(conn, "CREATE TABLE `table_tennis`.`match_users` ( `match_user_id` INT NOT NULL, `match_user_tour_id` INT NOT NULL, `match_user_match_id` INT NOT NULL, `match_user_user_id` INT NOT NULL, `match_user_from_team_1` TINYINT NOT NULL, PRIMARY KEY (`match_user_id`, `match_user_tour_id`, `match_user_match_id`), INDEX `match_users_tournaments_idx` (`match_user_tour_id` ASC) VISIBLE, INDEX `match_users_matches_idx` (`match_user_match_id` ASC) VISIBLE, INDEX `match_users_users_idx` (`match_user_user_id` ASC) VISIBLE, CONSTRAINT `match_users_tournaments` FOREIGN KEY (`match_user_tour_id`) REFERENCES `table_tennis`.`tournaments` (`tournament_id`) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT `match_users_matches` FOREIGN KEY (`match_user_match_id`) REFERENCES `table_tennis`.`matches` (`match_id`) ON DELETE NO ACTION ON UPDATE NO ACTION, CONSTRAINT `match_users_users` FOREIGN KEY (`match_user_user_id`) REFERENCES `table_tennis`.`users` (`user_id`) ON DELETE NO ACTION ON UPDATE NO ACTION);")) {
			cout << "match_users creation failed: " << mysql_error(conn) << endl;
			return 1;
		 }

		 cout << "table_tennis was created" << endl;
	  }
   }
   else
   {
	  cout << "Query failed: " << mysql_error(conn) << endl;
   }

   return 0;
}

int db_store() {
   MYSQL_ROW row;
   MYSQL_RES* res;

   if (mysql_query(conn, "UPDATE table_tennis.users SET `user_team_id`=null WHERE `user_id`>='0';")) {
	  cout << "user update failed: " << mysql_error(conn) << endl;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.sets WHERE `set_id`>=0;")) {
	  cout << "sets deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.match_users WHERE `match_user_id`>=0;")) {
	  cout << "match_users deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.matches WHERE `match_id`>=0;")) {
	  cout << "matches deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.tournaments WHERE `tournament_id`>=0;")) {
	  cout << "tournaments deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.teams WHERE `team_id`>=0;")) {
	  cout << "teams deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   if (mysql_query(conn, "DELETE FROM table_tennis.users WHERE `user_id`>=0;")) {
	  cout << "users deletion failed: " << mysql_error(conn) << endl;
	  return 1;
   }

   for (std::map<unsigned int, User*>::iterator iter = all_users.begin(); iter != all_users.end(); ++iter)
   {
	  auto user = iter->second;

	  string query = "INSERT INTO table_tennis.users VALUES ('" + to_string(user->id) + "','" + user->name + "',null);";
	  if (mysql_query(conn, query.c_str())) {
		 cout << "user addition failed: " << mysql_error(conn) << endl;
	  }
   }

   for (std::map<unsigned int, Team*>::iterator iter = all_teams.begin(); iter != all_teams.end(); ++iter)
   {
	  auto team = iter->second;

	  string query = "INSERT INTO table_tennis.teams VALUES ('" + to_string(team->id) + "','" + team->name + "','" + to_string(team->owner->id) + "');";
	  if (mysql_query(conn, query.c_str())) {
		 cout << "team addition failed: " << mysql_error(conn) << endl;
	  }
   }

   for (std::map<unsigned int, User*>::iterator iter = all_users.begin(); iter != all_users.end(); ++iter)
   {
	  auto user = iter->second;
	  if (user->team != nullptr) {
		 string query = "UPDATE table_tennis.users SET `user_team_id`='" + to_string(user->team->id) + "' WHERE `user_id`='"+ to_string(user->id) +"'";
		 if (mysql_query(conn, query.c_str())) {
			cout << "user update failed: " << mysql_error(conn) << endl;
		 }
	  }
   }
   unsigned int match_user_counter = 0;
   unsigned int sets_counter = 0;
   for (std::map<unsigned int, Tour*>::iterator iter = all_tours.begin(); iter != all_tours.end(); ++iter)
   {
	  auto tour = iter->second;
	  string query = "INSERT INTO table_tennis.tournaments VALUES ('" + to_string(tour->id) + "','" + tour->name + "','" + to_string(tour->home->id) + "','" + to_string(tour->away->id) + "','" + to_string(tour->owner->id) + "');";
	  if (mysql_query(conn, query.c_str())) {
		 cout << "tournament insertion failed: " << mysql_error(conn) << endl;
	  }

	  for (std::map<unsigned int, Match>::iterator iter = tour->matches.begin(); iter != tour->matches.end(); ++iter)
	  {
		 auto& match = iter->second;
		 query = "INSERT INTO table_tennis.matches VALUES ('" + to_string(match.id) + "','" + to_string(tour->id) + "','" + to_string(match.team1SetPoints) + "','" + to_string(match.team2SetPoints) + "','" + to_string(match.playersPerTeam) + "','" + to_string(match.numberOfSets) + "','" + to_string(match.originalSide) + "','" + to_string(match.originalServe) + "');";
		 if (mysql_query(conn, query.c_str())) {
			cout << "match insertion failed: " << mysql_error(conn) << endl;
		 }

		 for (const auto player : match.playersTeam1) {
			query = "INSERT INTO table_tennis.match_users VALUES ('" + to_string(match_user_counter++) + "','" + to_string(tour->id) + "','" + to_string(match.id) + "','" + to_string(player->id) + "','1');";
			if (mysql_query(conn, query.c_str())) {
			   cout << "match_user insertion failed: " << mysql_error(conn) << endl;
			}
		 }

		 for (const auto player : match.playersTeam2) {
			query = "INSERT INTO table_tennis.match_users VALUES ('" + to_string(match_user_counter++) + "','" + to_string(tour->id) + "','" + to_string(match.id) + "','" + to_string(player->id) + "','0');";
			if (mysql_query(conn, query.c_str())) {
			   cout << "match_user 2 insertion failed: " << mysql_error(conn) << endl;
			}
		 }

		 for (const auto set : match.sets) {
			query = "INSERT INTO table_tennis.sets VALUES ('" + to_string(sets_counter++) + "','" + to_string(tour->id) + "','" + to_string(match.id) + "','" + to_string(set.team1Points) + "','" + to_string(set.team2Points) + "');";
			if (mysql_query(conn, query.c_str())) {
			   cout << "sets insertion failed: " << mysql_error(conn) << endl;
			}
		 }
	  }
   }


   return 0;
}
#endif

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

   CivetServer server(cpp_options);

#ifdef USE_DB
   bool db_connected = db_connect() == 0;
   if (db_connected) {
	  db_load();
   }
#endif

   WebsocketHandler h_websocket;
   server.addWebSocketHandler("/websocket", h_websocket);
   printf("Run websocket example at http://localhost:%s/ws\n", PORT);

   ExitHandler h_exit;
   server.addHandler(EXIT_URI, h_exit);

   //printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
   printf("Exit at http://localhost:%s%s\n", PORT, EXIT_URI);



   while (!exitNow) {
#ifdef _WIN32
	  Sleep(1000);
#else
	  sleep(1);
#endif
   }

#ifdef USE_DB
   if (db_connected) {
	  db_store();
   }
#endif

   printf("Bye!\n");
   mg_exit_library();

   return 0;
}
