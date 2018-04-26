#include "CompRigidBody.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleGUI.h"
#include "WindowInspector.h"
#include "Scene.h"
#include "ModulePhysics.h"
#include "JSONSerialization.h"

#include "jpPhysicsRigidBody.h"
#include "CompCollider.h"
#include "CompTransform.h"
#include "CompJoint.h"

CompRigidBody::CompRigidBody(Comp_Type t, GameObject * parent) : Component(t, parent)
{
	uid = App->random->Int();
	name_component = "CompRigidBody";

	if (parent)
	{
		transform = parent->GetComponentTransform();
		collider_comp = (CompCollider*)parent->FindComponentByType(Comp_Type::C_COLLIDER);
		if (collider_comp != nullptr)
		{
			body = collider_comp->GivePhysicsBody(this);
			App->physics->ChangeRigidActorToDynamic(body, this);
			collider_comp->SetFilterFlags();
		}
		else
		{
			body = App->physics->GetNewRigidBody(this, true);
			SetColliderPosition();
		}
	}
}

CompRigidBody::CompRigidBody(const CompRigidBody & copy, GameObject * parent) : Component(Comp_Type::C_RIGIDBODY, parent)
{
	uid = App->random->Int();
	name_component = "CompRigidBody";

	//Same as regular constructor since this properties depend on the parent
	if (parent)
	{
		collider_comp = (CompCollider*)parent->FindComponentByType(Comp_Type::C_COLLIDER);
		if (collider_comp != nullptr)
		{
			body = collider_comp->GivePhysicsBody(this);
			App->physics->ChangeRigidActorToDynamic(body, this);
			collider_comp->SetFilterFlags();
		}
		else
		{
			body = App->physics->GetNewRigidBody(this, true);
		}
	}

	kinematic = copy.kinematic;
	body->SetAsKinematic(kinematic);

	lock_move = copy.lock_move;
	SetDinamicLockFlags();
}

CompRigidBody::~CompRigidBody()
{
}

void CompRigidBody::PreUpdate(float dt)
{
	if (((dt > 0 && body && !kinematic && lock_move != 63) || (own_update)) && !transform->GetToUpdate())
	{
		own_update = true;
		UpdateParentPosition();
	}
}

void CompRigidBody::Update(float dt)
{
	if (transform->GetUpdated() && !own_update)
	{
		SetColliderPosition();
	}
	else
	{
		own_update = false;
	}
}

void CompRigidBody::Clear()
{
	if (joint != nullptr)
	{
		joint->RemoveActors(this);
		joint = nullptr;
	}

	if (collider_comp != nullptr)
	{
		App->physics->ChangeRigidActorToStatic(body, collider_comp);
		collider_comp->SetRigidBodyComp(nullptr);
		collider_comp = nullptr;
		body = nullptr;
	}
	else if (body != nullptr)
	{
		App->physics->DeleteCollider(this, body);
		body = nullptr;
	}
}

void CompRigidBody::ShowOptions()
{
	if (ImGui::MenuItem("Reset", NULL, false, false))
	{
		// Not implemented yet.
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Remove Component"))
	{
		to_delete = true;
	}
	if (ImGui::MenuItem("Move to Front", NULL, false, false))
	{
		// Not implemented yet.
	}
	if (ImGui::MenuItem("Move to Back", NULL, false, false))
	{
		// Not implemented yet.
	}
	if (ImGui::MenuItem("Move Up", NULL, false, false))
	{
		// Not implemented yet.
	}
	if (ImGui::MenuItem("Move Down", NULL, false, false))
	{
		// Not implemented yet.
	}
	if (ImGui::MenuItem("Copy Component"))
	{
		((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->SetComponentCopy(this);
	}
	if (ImGui::MenuItem("Paste Component As New", NULL, false, ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->AnyComponentCopied()))
	{
		if (parent->FindComponentByType(((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType()) == nullptr
			|| ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType() > Comp_Type::C_UNIQUE_SEPARATOR)
		{
			parent->AddComponentCopy(*((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied());
		}
	}
	if (ImGui::MenuItem("Paste Component Values", NULL, false, ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->AnyComponentCopied()))
	{
		if (this->GetType() == ((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()->GetType())
		{
			CopyValues(((CompRigidBody*)((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()));
		}
	}
}

void CompRigidBody::ShowInspectorInfo()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 0));
	ImGui::SameLine(ImGui::GetWindowWidth() - 26);
	if (ImGui::ImageButton((ImTextureID*)App->scene->icon_options_transform, ImVec2(13, 13), ImVec2(-1, 1), ImVec2(0, 0)))
	{
		ImGui::OpenPopup("OptionsCollider");
	}
	ImGui::PopStyleVar();
	if (ImGui::BeginPopup("OptionsCollider"))
	{
		ShowOptions();
		ImGui::EndPopup();
	}

	if (ImGui::Checkbox("Kinematic", &kinematic))
	{
		body->SetAsKinematic(kinematic);
		
		if (!kinematic)
		{
			SetDinamicLockFlags();
		}
	}
	ImGui::Separator();

	if (ImGui::InputFloat("Sleep Time", &sleep_time, 0.1, ImGuiInputTextFlags_EnterReturnsTrue) && body)
	{
		if (sleep_time < 0)
		{
			sleep_time = -sleep_time;
		}
		if (!kinematic)
		{
			body->SetSleepTime(sleep_time);
		}
	}

	// Lock Linear move -----------------
	ImGui::Text("Lock Linear Move");
	bool flags = (lock_move & (1 << 0));
	if (ImGui::Checkbox("X", &flags))
	{
		(flags) ? lock_move |= (1 << 0) : lock_move &= ~(1 << 0);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, flags);
	}
	ImGui::SameLine();
	flags = (lock_move & (1 << 1));
	if (ImGui::Checkbox("Y", &flags))
	{
		(flags) ? lock_move |= (1 << 1) : lock_move &= ~(1 << 1);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, flags);
	}
	ImGui::SameLine();
	flags = (lock_move & (1 << 2));
	if (ImGui::Checkbox("Z", &flags))
	{
		(flags) ? lock_move |= (1 << 2) : lock_move &= ~(1 << 2);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, flags);
	}
	ImGui::Separator();

	// Lock Angular move -----------------
	ImGui::Text("Lock Angular Move");
	flags = (lock_move & (1 << 3));
	if (ImGui::Checkbox("x", &flags))
	{
		(flags) ? lock_move |= (1 << 3) : lock_move &= ~(1 << 3);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, flags);
	}
	ImGui::SameLine();
	flags = (lock_move & (1 << 4));
	if (ImGui::Checkbox("y", &flags))
	{
		(flags) ? lock_move |= (1 << 4) : lock_move &= ~(1 << 4);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, flags);
	}
	ImGui::SameLine();
	flags = (lock_move & (1 << 5));
	if (ImGui::Checkbox("z", &flags))
	{
		(flags) ? lock_move |= (1 << 5) : lock_move &= ~(1 << 5);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, flags);
	}

	ImGui::TreePop();
}

void CompRigidBody::CopyValues(const CompRigidBody * component)
{
	//more...
}

void CompRigidBody::Save(JSON_Object * object, std::string name, bool saveScene, uint & countResources) const
{
	json_object_dotset_string_with_std(object, name + "Component:", name_component);
	json_object_dotset_number_with_std(object, name + "Type", this->GetType());
	json_object_dotset_number_with_std(object, name + "UUID", uid);

	json_object_dotset_boolean_with_std(object, name + "Kinematic", kinematic);

	json_object_dotset_number_with_std(object, name + "Move Locked", lock_move);

	json_object_dotset_number_with_std(object, name + "Sleep Time", sleep_time);
}

void CompRigidBody::Load(const JSON_Object * object, std::string name)
{
	uid = json_object_dotget_number_with_std(object, name + "UUID");

	kinematic = json_object_dotget_boolean_with_std(object, name + "Kinematic");

	lock_move = json_object_dotget_number_with_std(object, name + "Move Locked");

	sleep_time = json_object_dotget_number_with_std(object, name + "Sleep Time");
}

void CompRigidBody::SyncComponent(GameObject* sync_parent)
{
	body->SetAsKinematic(kinematic);

	if (!kinematic)
	{
		body->SetSleepTime(sleep_time);
		SetDinamicLockFlags();
	}
}

void CompRigidBody::GetOwnBufferSize(uint& buffer_size)
{
	Component::GetOwnBufferSize(buffer_size);
	buffer_size += sizeof(int);			//UID
	buffer_size += sizeof(bool);		//kinematic
	buffer_size += sizeof(int);			//lock_move
	buffer_size += sizeof(float);			//sleep_time
}

void CompRigidBody::SaveBinary(char ** cursor, int position) const
{
	Component::SaveBinary(cursor, position);
	App->json_seria->SaveIntBinary(cursor, uid);
	App->json_seria->SaveBooleanBinary(cursor, kinematic);
	App->json_seria->SaveIntBinary(cursor, lock_move);
	App->json_seria->SaveFloatBinary(cursor, sleep_time);
}

void CompRigidBody::LoadBinary(char ** cursor)
{
	uid = App->json_seria->LoadIntBinary(cursor);
	kinematic = App->json_seria->LoadBooleanBinary(cursor);
	lock_move = App->json_seria->LoadIntBinary(cursor);
	sleep_time = App->json_seria->LoadFloatBinary(cursor);

	Enable();
}

bool CompRigidBody::IsKinematic()
{
	return kinematic;
}

bool CompRigidBody::HaveBodyShape()
{
	return (body != nullptr);
}

void CompRigidBody::SetColliderPosition()
{
	Quat quat = transform->GetRotParent()*transform->GetRot();
	float3 fpos = transform->GetPosGlobal();

	if (collider_comp)
	{
		quat = quat * collider_comp->GetLocalQuat();
		fpos = fpos + quat * collider_comp->GetPosition();
	}

	body->SetTransform(fpos, quat);
	App->physics->DebugDrawUpdate();
}

void CompRigidBody::SetMomentumToZero()
{
	if (body && !kinematic)
	{
		body->ResetForces();
	}
}

void CompRigidBody::SetDinamicLockFlags()
{
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, lock_move & (1 << 0));
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, lock_move & (1 << 1));
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, lock_move & (1 << 2));
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, lock_move & (1 << 3));
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, lock_move & (1 << 4));
	body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, lock_move & (1 << 5));
}

void CompRigidBody::SetColliderComp(CompCollider * new_comp)
{
	collider_comp = new_comp;
}

void CompRigidBody::SetJointActor(CompJoint * new_joint)
{
	joint = new_joint;
}

jpPhysicsRigidBody * CompRigidBody::GetPhysicsBody() const
{
	return body;
}

void CompRigidBody::UpdateParentPosition()
{
	// Change variables to mathgeolib
	float3 fpos;
	Quat quat;
	body->GetTransform(fpos, quat);

	Quat global_rot = transform->GetRotParent().Conjugated();

	// Global to local transform
	float3 p_pos = transform->GetPosParent();
	float3 p_scale = transform->GetParentScale();
	quat = global_rot * quat;
	fpos = global_rot * (fpos - transform->GetPosParent());
	fpos = float3(fpos.x / p_scale.x, fpos.y / p_scale.y, fpos.z / p_scale.z);

	// Collider to local transform
	if (collider_comp) {
		Quat local_rot = collider_comp->GetLocalQuat().Conjugated();
		fpos -= quat * collider_comp->GetPosition();
		quat = quat * local_rot;
	}

	// GameObject local transform and position
	transform->SetPos(fpos);
	transform->SetRot(quat);
}

void CompRigidBody::SetMaxJointPose()
{
	if (joint)
	{
		//joint->SetSecondActorPose();
	}
}

void CompRigidBody::RemoveJoint()
{
	if (joint != nullptr)
	{
		joint->RemoveActors();
		joint = nullptr;
	}
}

void CompRigidBody::OnTriggerEnter(Component* actor1)
{
	if (collider_comp)
	{
		collider_comp->OnTriggerEnter(actor1);
	}
}

void CompRigidBody::OnTriggerLost(Component* actor1)
{
	if (collider_comp)
	{
		collider_comp->OnTriggerLost(actor1);
	}
}

void CompRigidBody::OnContact(CollisionData new_data)
{
	if (collider_comp)
	{
		collider_comp->OnContact(new_data);
	}
}

void CompRigidBody::MoveKinematic(float3 pos, Quat rot)
{
	if (body && kinematic)
	{
		body->MoveKinematic(pos, rot);
		own_update = true;
	}
	else if (body)
	{
		body->SetTransform(pos, rot);
	}
}

void CompRigidBody::ApplyForce(float3 force)
{
	if (body && !kinematic)
	{
		body->ApplyForce(force);
	}
}

void CompRigidBody::ApplyImpulse(float3 impulse)
{
	if (body && !kinematic)
	{
		body->ApplyImpulse(impulse);
	}
}

void CompRigidBody::ApplyTorqueForce(float3 force)
{
	if (body && !kinematic)
	{
		body->ApplyTorqueForce(force);
	}
}

void CompRigidBody::ApplyTorqueImpulse(float3 impulse)
{
	if (body && !kinematic)
	{
		body->ApplyTorqueImpulse(impulse);
	}
}

void CompRigidBody::LockMotion()
{
	if (body && !kinematic)
	{
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, true);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, true);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, true);
		lock_move |= (1 << 0);
		lock_move |= (1 << 1);
		lock_move |= (1 << 2);
	}
}

void CompRigidBody::LockRotation()
{
	if (body && !kinematic)
	{
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
		lock_move |= (1 << 3);
		lock_move |= (1 << 4);
		lock_move |= (1 << 5);
	}
}

void CompRigidBody::LockTransform()
{
	if (body && !kinematic)
	{
		body->SetDynamicLock(true);
		lock_move = 63;
	}
}

void CompRigidBody::UnLockMotion()
{
	if (body && !kinematic)
	{
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, false);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, false);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, false);
		lock_move &= ~(1 << 0);
		lock_move &= ~(1 << 1);
		lock_move &= ~(1 << 2);
	}
}

void CompRigidBody::UnLockRotation()
{
	if (body && !kinematic)
	{
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);
		body->SetDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
		lock_move &= ~(1 << 3);
		lock_move &= ~(1 << 4);
		lock_move &= ~(1 << 5);
	}
}

void CompRigidBody::UnLockTransform()
{
	if (body && !kinematic)
	{
		body->SetDynamicLock(false);
		lock_move = 0;
	}
}

void CompRigidBody::WakeUp()
{
	if (body)
	{
		body->WakeUp();
	}
}
