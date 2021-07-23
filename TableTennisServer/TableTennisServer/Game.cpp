#include "Game.h"


void Match::generateJson(JsonObject& jsonObject)
{
   jsonObject.AddNameValuePairNumber("id", this->id);
   jsonObject.AddNameValuePairNumber("players-per-team", this->playersPerTeam);
   jsonObject.AddNameValuePairNumber("number-of-sets", this->numberOfSets);
   jsonObject.AddNameValuePairBool("original-side", this->originalSide);
   jsonObject.AddNameValuePairBool("original-serve", this->originalServe);


   jsonObject.AddNameValuePairNumber("team-1-set-points", this->team1SetPoints);
   jsonObject.AddNameValuePairNumber("team-2-set-points", this->team2SetPoints);

   int playercountTeam1 = this->playersTeam1.size();
   int* playersTeam1Ids = (int*)malloc(playercountTeam1 * sizeof(int));
   int i = 0;
   for (auto& user : this->playersTeam1) {
      playersTeam1Ids[i] = user->id;
      i++;
   }
   free(playersTeam1Ids);
   jsonObject.AddNameValuePairArray("players-team-1-ids", playersTeam1Ids, playercountTeam1);

   int playercountTeam2 = this->playersTeam2.size();
   int* playersTeam2Ids = (int*)malloc(playercountTeam2 * sizeof(int));
   i = 0;
   for (auto& user : this->playersTeam2) {
      playersTeam2Ids[i] = user->id;
      i++;
   }
   free(playersTeam2Ids);
   jsonObject.AddNameValuePairArray("players-team-2-ids", playersTeam2Ids, playercountTeam2);

   vector<JsonValue*>* sets = new vector<JsonValue*>();

   for (auto& set : this->sets) {
      JsonValue* jv_set = new JsonValue();
      jv_set->m_type = JsonValueTypeObject;
      JsonObject* jo_set = new JsonObject();
      set.generateJson(*jo_set);
      jv_set->m_data.u_jsonObject = jo_set;
      sets->push_back(jv_set);
   }

   jsonObject.AddNameValuePairArray("sets", sets);
}

void Match::generateJson(JsonObject& jsonObject, unsigned int levels)
{
   throw "Not implemented";
}


Match::Match()
{
}

Match::Match(unsigned int id) {
   this->id = id;
   sets.push_back(Set(0));
}

 void User::generateJson(JsonObject& jsonObject)
{
    jsonObject.AddNameValuePairNumber("id", this->id);
    jsonObject.AddNameValuePairString("name", this->name.c_str());
    int teamId = -1;
    if (this->team != nullptr)
       teamId = this->team->id;
    jsonObject.AddNameValuePairNumber("team-id", teamId);

}

 void User::generateJson(JsonObject& jsonObject, unsigned int levels)
 {
    throw "Not implemented";
 }


 Set::Set()
 {
 }

 Set::Set(unsigned int id)
 {
    this->id = id;
 }

 void Set::generateJson(JsonObject& jsonObject)
 {
    jsonObject.AddNameValuePairNumber("team-1-points", this->team1Points);
    jsonObject.AddNameValuePairNumber("team-2-points", this->team2Points);
 }

 void Set::generateJson(JsonObject& jsonObject, unsigned int levels)
 {
    throw "Not implemented";
 }

 void Team::generateJson(JsonObject& jsonObject)
 {
    jsonObject.AddNameValuePairNumber("id", this->id);
    jsonObject.AddNameValuePairString("name", this->name.c_str());
    jsonObject.AddNameValuePairNumber("owner-id", this->owner->id);

    int size = this->users.size();
    int* userIds = (int*)malloc(size * sizeof(int));
    int i = 0;
    for (auto& user : this->users) {
       userIds[i] = user->id;
       i++;
    }
    
    jsonObject.AddNameValuePairArray("user-ids", userIds, size);

    free(userIds);
 }

 void Team::generateJson(JsonObject& jsonObject, unsigned int levels)
 {
    throw "Not implemented";
 }

 void Tour::generateJson(JsonObject& jsonObject)
 {
    jsonObject.AddNameValuePairNumber("id", this->id);
    jsonObject.AddNameValuePairString("name", this->name.c_str());
    jsonObject.AddNameValuePairNumber("owner-id", this->owner->id);
    jsonObject.AddNameValuePairNumber("home-id", this->home->id);
    jsonObject.AddNameValuePairNumber("away-id", this->away->id);
 }

 void Tour::generateJson(JsonObject& jsonObject, unsigned int levels)
 {
    throw "Not implemented";
 }

 Tour::Tour()
 {
    this->matches[0] = Match(0);
    this->matches[1] = Match(1);
    this->matches[2] = Match(2);
    this->matches[3] = Match(3);
 }
