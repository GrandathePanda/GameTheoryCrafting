#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <unordered_map>


class playerClass {
	/*playerClass is a class containing every single variable a WoW character can have in relation to combat, for the sake of data manipulation
	all variable types are kept to type double to limit potential math blunders though certain variables from the Blizzard api such as stam are
	integers. All classes will derive these variables from this class and its constructer other than that there is nothing else of notable import
	*CURRENTLY* This may be subject to change as the program evolves. Really characters in WoW only share these statistics and outside of that each 
	class varies heavily as do base statistics.

	--UPDATE 31/8/14: Though classes do not share the same talents, they all have talents that can be stored in a look-up table just like stats,
	so this has been added to playerClass
	*/
public:
	playerClass() {};
	playerClass(std::unordered_map<std::string,double>& statList, int classType)
		:statList(statList)
	{

	};
	std::unordered_map<std::string, double>& getStatList() {
		return statList;
	}
private:
	std::unordered_map<std::string, double> statList;
	std::unordered_map<std::string, bool> talents;
	int classType;

};

class Warlock : public playerClass {
public:
	Warlock( std::unordered_map<std::string,double>& statList, int classType )
		:playerClass(statList,classType)
	{
	
	}
private:
	enum Spec {
		demo,
		destro,
		affli
	};
	Spec cSpec;
	enum Demon {
		imp,
		voidwalker,
		succubus,
		felhunter,
		felguard,
		felimp,
		voidlord,
		shivarra,
		watcher,
		wrathguard
	};
	Demon currentDem; 
	
	
};
class stringtStdO { //Simple functor class to more easily handle string conversions between std::string and utility::string_t until
public:				//functionality is baked into Casablanca REST sdk in a future update. TODO::REMOVE WHEN REST SDK IS UPDATED
	std::string operator()(utility::string_t str) {
		std::string std;
		std = utility::conversions::to_utf8string(str);
		return std;
	}
	utility::string_t operator()(std::string str) {
		utility::string_t strT;
		strT = utility::conversions::to_string_t(str);
		return strT;
	}
};

pplx::task<playerClass*> loadCharacterStats(std::string& name, std::string& server) {
	stringtStdO strConv;
	//example request from WoW API: http://us.battle.net/api/wow/character/Medivh/Uther?fields=guild,items,professions,reputation,stats
	web::http::uri_builder build(U("http://us.battle.net"));
	build.append_path(U("/api/wow/character/")).append_path(strConv(server)).append_path(U("/")).append_path(strConv(name)).append_query(U("fields=stats"));
	auto uri = build.to_uri(); 
	std::cout << strConv(uri.to_string()) << std::endl;
	web::http::client::http_client client(uri);
	pplx::task<playerClass*> pClassGrab = client.request(web::http::methods::GET)
		.then([](web::http::http_response response) -> pplx::task<web::json::value> {
		if (response.status_code() != web::http::status_codes::OK) {
			std::cerr << "Unable to connect to host. CODE: " << response.status_code() << std::endl;
			return pplx::task_from_result(web::json::value());
		}
		else
			std::cout << response.status_code() << std::endl;
			return response.extract_json();

	})
	.then([](pplx::task<web::json::value> jsonResp) {
		try {
			stringtStdO strConv;
			web::json::value& jsonInfo = jsonResp.get();
			web::json::object& statList = jsonInfo.at(U("stats")).as_object();
			std::unordered_map<std::string,double> stats;
			for (web::json::object::const_iterator iter = statList.begin(); iter != statList.end(); iter++) {
				if (iter->second.is_string() == false) // In the blizzard stat json there is one stat that is a string which is the energy source the class uses...
					//...e.g mana,rage,energy etc this takes that into account and excludes it so it can't cause an error. 
					stats.insert(std::pair<std::string, double>(strConv(iter->first), iter->second.as_double()));

				
			}
			playerClass *pClass = nullptr;
			if (jsonInfo.at(U("class")).as_integer() == 9) {
				pClass = new Warlock(stats, 9); //
			}
			return pClass;
			/* NOTE LOOK HERE IAN:: MAKE SURE TO ADD IN OTHER CLASSES LOADING CODE AFTER YOU FINISH WARLOCK*/

		}
		catch (web::json::json_exception& e)
		{
			std::cerr << e.what() << std::endl;
			playerClass *pClass = nullptr;
			return pClass;
			
		}
		
	})
		.then([](pplx::task<playerClass*> pClassTask) {
		return pClassTask;
	});
	return pClassGrab;



}
pplx::task<std::unordered_map<std::string,bool>> loadCharacterTalents(std::string& charName, std::string& server) {
	web::http::uri_builder uriBuild(U("http://us.battle.net"));
	stringtStdO strConv;
	uriBuild.append_path(U("/api/wow/character/")).append_path(strConv(server)).append_path(U("/")).append_path(strConv(charName))
		.append_query(U("fields=talents"));
	auto uri = uriBuild.to_uri();
	web::http::client::http_client client(uri);
	pplx::task<std::unordered_map<std::string, bool>> characterTalents = client.request(web::http::methods::GET)
		.then([](web::http::http_response response) -> pplx::task<web::json::value> 
	{
		if (response.status_code() != web::http::status_codes::OK) {
			std::cerr << "Error unable to connect. CODE:" << response.status_code();
			return pplx::task_from_result(web::json::value());
		}
		else {
			return response.extract_json();
		}
	})
		.then([](pplx::task<web::json::value> jsonResp){
		try {
			web::json::value& jsonVal = jsonResp.get();
			std::unordered_map<std::string, bool> talentList;
			std::cout << "HERE\n";
			web::json::object& talentJsonObj = jsonVal.at(U("talents")).as_object();
			for (web::json::object::const_iterator iter = talentJsonObj.cbegin(); iter != talentJsonObj.cend(); iter++)
			{
				
			}
			return talentList;
		}
		catch (web::json::json_exception& e)
		{
			std::cout << e.what() << std::endl;
			std::unordered_map<std::string, bool> talentList;
			return talentList;
		}
	
	})
		.then([](pplx::task<std::unordered_map<std::string, bool>> talentTask){
		return talentTask;

	});
	return characterTalents; 
}

void loadCharacter(std::string& charName, std::string server, playerClass*& pClass) {
	pplx::task<playerClass*> pClassTask = loadCharacterStats(charName, server);
	pplx::task<std::unordered_map<std::string, bool>> pTalentTask = loadCharacterTalents(charName, server);
	pClassTask.wait();
	pTalentTask.wait();
	if (pClassTask.is_done() == true) {
		pClass = pClassTask.get();
		std::cout << "Loading Character " << charName << "...Successful" << std::endl;
	}
	std::unordered_map<std::string, double> stats = pClass->getStatList();
	for (std::unordered_map<std::string, double>::const_iterator iter = stats.cbegin(); iter != stats.cend(); iter++) {
		std::cout << iter->first << ": " << iter->second << std::endl;
	}
	

}

int main() {

	std::string server = "Illidan";
	std::string charName = "Mathendris";
	playerClass* pClass = nullptr;
	loadCharacter(charName, server, pClass);
	delete(pClass);

}