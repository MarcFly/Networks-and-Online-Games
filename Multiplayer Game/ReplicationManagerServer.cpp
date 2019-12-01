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

		GameObject* get = App->modLinkingContext->getNetworkGameObject(it->first);

		if (get != nullptr)
		{
			packet << ReplicationAction::Success;
			switch (it->second)
			{
			case ReplicationAction::Create:
				get->SerializeCreate(packet);
			case ReplicationAction::Update:
				get->SerializeUpdate(packet);
				break;
			case ReplicationAction::Destroy:
				break;
			}
		}
		else
		{
			packet << ReplicationAction::Error;
		}
			
	}
}