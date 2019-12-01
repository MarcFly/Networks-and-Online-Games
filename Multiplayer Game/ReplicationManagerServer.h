#pragma once

#include <unordered_map>
#include <vector>
enum class ReplicationAction
{
	Error = -1,
	None,
	Create,
	Update,
	Destroy,
	Success
};


struct ReplicationCommand
{
	ReplicationCommand() {};
	ReplicationCommand(ReplicationAction a, uint32 n) : action(a), netID(n) {};
	ReplicationAction action;
	uint32 netID;
};


class ReplicationManagerServer
{
public:
	void create(uint32 netID) 
	{ 
		auto it = Commands.find(netID);
		if(it == Commands.end()) 
			Commands.insert(std::pair<uint32,ReplicationAction>(netID, ReplicationAction::Create)); 
	};
	void update(uint32 netID) 
	{ 
		auto it = Commands.find(netID);
		if(it == Commands.end())
			Commands.insert(std::pair<uint32, ReplicationAction>(netID, ReplicationAction::Update));
	};
	void destroy(uint32 netID) 
	{
		auto it = Commands.find(netID);
		if (it == Commands.end())
			Commands.insert(std::pair<uint32, ReplicationAction>(netID, ReplicationAction::Destroy));
		else
			it->second = ReplicationAction::Destroy;
	};

	void write(OutputMemoryStream &packet);

	// What more?	
	//std::vector<ReplicationCommand> Commands;
	std::unordered_map<uint32, ReplicationAction> Commands;
	//std::unordered_map<uint32, std::vector<ReplicationAction>*> ManageStruct;
};