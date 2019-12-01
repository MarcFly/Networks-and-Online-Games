#include "Networks.h"
void ReplicationManagerClient::read(const InputMemoryStream &packet)
{
	int steps;
	packet >> steps;
	while (steps > 0)
	{
		uint32 netID;
		ReplicationAction act;
		packet >> netID;
		packet >> act;
		GameObject* get = nullptr;
		switch (act)
		{
		case ReplicationAction::Create:
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(App->modGameObject->Instantiate(), netID);
			break;
		case ReplicationAction::Update:
			get = App->modLinkingContext->getNetworkGameObject(netID);
			if(get) get->ReadUpdate(packet);
			break;
		case ReplicationAction::Destroy:
			get = App->modLinkingContext->getNetworkGameObject(netID);
			App->modLinkingContext->unregisterNetworkGameObject(get);
			App->modGameObject->Destroy(get);
		}
		--steps;
	}
}