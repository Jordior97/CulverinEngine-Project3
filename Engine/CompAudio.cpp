#include "CompAudio.h"
#include "Application.h"
#include "WindowInspector.h"
#include "ModuleGUI.h"
#include "GameObject.h"
#include "Scene.h"
#include "ModuleInput.h"

#include "CompTransform.h"
#include "ModuleAudio.h"

CompAudio::CompAudio(Comp_Type t, GameObject * parent) : Component(t, parent)
{
	name_component = "Audio";
	CreateEmitter();
}

CompAudio::CompAudio(const CompAudio & copy, GameObject * parent) : Component(Comp_Type::C_AUDIO, parent)
{
}

CompAudio::~CompAudio()
{
	delete emitter;

	if (audio_type == AUDIO_TYPE::LISTENER)
		App->audio->SetListener(nullptr);
}

void CompAudio::Update(float dt)
{
	if (transf != nullptr)
	{
		float3 pos = transf->GetPos();
		Quat rot = transf->GetRot();

		float3 up = rot.Transform(float3(0, 1, 0));
		float3 front = rot.Transform(float3(0, 0, 1));

		up.Normalize();
		front.Normalize();
		emitter->SetPosition(-pos.x, pos.y, pos.z, -front.x, front.y, front.z, -up.x, up.y, up.z);
	}
	
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
	{
		emitter->PlayEvent("Shot");
	}
}

void CompAudio::ShowOptions()
{
	//ImGui::MenuItem("CREATE", NULL, false, false);
	if (ImGui::MenuItem("Reset"))
	{
		// Not implmented yet.
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Move to Front", NULL, false, false))
	{
		// Not implmented yet.
	}
	if (ImGui::MenuItem("Move to Back", NULL, false, false))
	{
		// Not implmented yet.
	}
	if (ImGui::MenuItem("Remove Component"))
	{
		to_delete = true;
	}
	if (ImGui::MenuItem("Move Up", NULL, false, false))
	{
		// Not implmented yet.
	}
	if (ImGui::MenuItem("Move Down", NULL, false, false))
	{
		// Not implmented yet.
	}
	if (ImGui::MenuItem("Copy Component"))
	{
		((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->SetComponentCopy(this);
	}
	if (ImGui::MenuItem("Paste Component As New", NULL, false, ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->AnyComponentCopied()))
	{
		if (parent->FindComponentByType(((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType()) == nullptr
			|| ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType() > Comp_Type::C_CAMERA)
		{
			parent->AddComponentCopy(*((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied());
		}
	}
	if (ImGui::MenuItem("Paste Component Values", NULL, false, ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->AnyComponentCopied()))
	{
		if (this->GetType() == ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType())
		{
			CopyValues(((CompAudio*)((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()));
		}
	}
}

void CompAudio::ShowInspectorInfo()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 0));
	ImGui::SameLine(ImGui::GetWindowWidth() - 26);
	if (ImGui::ImageButton((ImTextureID*)App->scene->icon_options_transform, ImVec2(13, 13), ImVec2(-1, 1), ImVec2(0, 0)))
	{
		ImGui::OpenPopup("Audio Options");
	}
	ImGui::PopStyleVar();


	// Button Options --------------------------------------
	if (ImGui::BeginPopup("Audio Options"))
	{
		ShowOptions();
		ImGui::EndPopup();
	}
	
	//Audio component options -------------------
	int selected_opt = audio_type;
	if (ImGui::Combo("Audio type", &selected_opt, "FX\0Listener\0", 2))
	{
		if (selected_opt == 1 && audio_type != LISTENER)
		{
			audio_type = LISTENER;
			App->audio->SetListener(this);
		}
		else if (selected_opt == 0)
		{
			if(audio_type==LISTENER)
				App->audio->SetListener(nullptr);			
			audio_type = FX;
		}		
	}

	ShowEventsInfo();

	ImGui::TreePop();
}

void CompAudio::ShowEventsInfo()
{
	ImGui::BeginChild(1, ImVec2(300, 150));
	int i = 0;
	for (std::multimap<int, AudioEvent>::iterator it = audio_events.begin(); it != audio_events.end(); i++)
	{
		ImGui::Separator();
		ImGui::PushID(i);
		ImGui::Text("Gameplay ev: %i", (*it).first);
		ImGui::SameLine();
		ImGui::Text("Audio ev: %s", (*it).second.name.c_str());

		ImGui::SameLine();
		if (ImGui::SmallButton("Play"))
		{
			emitter->PlayEvent((*it).second.name.c_str());
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Stop"))
		{
			emitter->StopEvent((*it).second.name.c_str());
		}
		ImGui::SameLine();

		if (ImGui::SmallButton("X"))
		{
			it = audio_events.erase(it);
		}
		else it++;

		ImGui::PopID();
	}

	ImGui::EndChild();
	if (ImGui::Button("AddEvent"))
	{
		CreateAudioEvent("", 0);
	}
}

void CompAudio::CopyValues(const CompAudio * component)
{
}

void CompAudio::Save(JSON_Object * object, std::string name, bool saveScene, uint & countResources) const
{
	json_object_dotset_string_with_std(object, name + "Component:", name_component);
	json_object_dotset_number_with_std(object, name + "Type", this->GetType());
	
}

void CompAudio::Load(const JSON_Object * object, std::string name)
{
	uid = json_object_dotget_number_with_std(object, name + "UUID");
	//...
	Enable();
}

uint CompAudio::GetEmitterID() const
{
	return emitter ? emitter->GetID() : 0;
}

void CompAudio::ResetAudio()
{
	audio_type = AUDIO_TYPE::FX;
}

void CompAudio::CreateEmitter()
{
	unsigned long emitter_id = parent->GetUUID();
	transf = parent->GetComponentTransform();
	float3 pos = transf->GetPosGlobal();
	emitter = Wwished::Utility::CreateEmitter(emitter_id, parent->GetName(), pos.x, pos.y, pos.z);
}

void CompAudio::CreateAudioEvent(std::string audio_event, int gameplay_event)
{
	AudioEvent new_ev;
	new_ev.name = audio_event;
	audio_events.insert(std::pair<int, AudioEvent> (gameplay_event, new_ev));
}


