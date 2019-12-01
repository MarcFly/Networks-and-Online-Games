#include "Networks.h"

void ReplicationManagerServer::write(OutputMemoryStream &packet)
{
	packet << ServerMessage::Replication;
	
	int steps = Commands.size();
	packet << steps;

	for (auto it = Commands.begin(); it != Commands.end(); ++it	)
	{		
		
		packet << it->first;
		packet << it->second;

		switch (it->second)
		{
		case ReplicationAction::Create:
			App->modLinkingContext->getNetworkGameObject(it->first)->SerializeCreate(packet);
			break;
		case ReplicationAction::Update:
			GameObject* get = App->modLinkingContext->getNetworkGameObject(it->first);
			if(get) get->SerializeUpdate(packet);
			break;
		}
	}
}