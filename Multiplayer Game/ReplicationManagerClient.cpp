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

		ReplicationAction res;
		packet >> res;
		if(act == ReplicationAction::Destroy || res == ReplicationAction::Success)
		switch (act)
		{
		case ReplicationAction::Create:
			get = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(get, netID);
			get->ReadCreate(packet);
			Spawn(get);
			get->ReadUpdate(packet);
			break;
		case ReplicationAction::Update:
			get = App->modLinkingContext->getNetworkGameObject(netID);
			if(get) get->ReadUpdate(packet);
			else
			{
				get = App->modGameObject->Instantiate();
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(get, netID);
				get->ReadUpdate(packet);
				Spawn(get);
				
			}

			break;
		case ReplicationAction::Destroy:
			get = App->modLinkingContext->getNetworkGameObject(netID);
			App->modLinkingContext->unregisterNetworkGameObject(get);
			App->modGameObject->Destroy(get);
		}
		--steps;
	}
}

void ReplicationManagerClient::Spawn(GameObject* go)
{
	if (go->player)
	{
		go->texture = App->modResources->spacecraft1;
		go->collider = App->modCollision->addCollider(ColliderType::Player, go);
		go->collider->isTrigger = true;

		//go->behaviour = new Spaceship;
		//go->behaviour->gameObject = go;
	}
	else
	{
		go->texture = App->modResources->laser;
		go->collider = App->modCollision->addCollider(ColliderType::Laser, go);

		//go->behaviour = new Laser;
		//go->behaviour->gameObject = go;
	}
}