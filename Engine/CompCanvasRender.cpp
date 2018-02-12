#include "CompCanvasRender.h"
#include "Application.h"
#include "ModuleGUI.h"
#include "WindowInspector.h"
#include "GameObject.h"
#include "Scene.h"
#include "CompImage.h"
#include "CompText.h"
#include "CompRectTransform.h"
#include "CompCanvas.h"

#include "SDL/include/SDL_opengl.h"
#include "GL3W/include/glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
CompCanvasRender::CompCanvasRender(Comp_Type t, GameObject * parent) :Component(t, parent)
{
	uid = App->random->Int();
	name_component = "Canvas Render";
	glGetError();
	my_canvas=(CompCanvas*)parent->FindParentComponentByType(Comp_Type::C_CANVAS);
	glGenBuffers(1, &vertices_id);
	CheckOpenGlError("glGenBuffers vertices");

	glGenBuffers(1, &indices_id);
	CheckOpenGlError("glGenBuffers indices_id");

}

CompCanvasRender::CompCanvasRender(const CompCanvasRender & copy, GameObject * parent) : Component(Comp_Type::C_CANVAS_RENDER, parent)
{
}

CompCanvasRender::~CompCanvasRender()
{
}

void CompCanvasRender::Update(float dt)
{
	AddToCanvas();
}

void CompCanvasRender::Clear()
{
	glDeleteBuffers(1, &vertices_id);
	vertices_id = 0;
	glDeleteBuffers(1, &indices_id);
	indices_id = 0;

}

void CompCanvasRender::ShowOptions()
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
			CopyValues(((CompCanvasRender*)((Inspector*)App->gui->win_manager[WindowName::INSPECTOR])->GetComponentCopied()));
		}
	}
}

void CompCanvasRender::ShowInspectorInfo()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 0));
	ImGui::SameLine(ImGui::GetWindowWidth() - 26);
	if (ImGui::ImageButton((ImTextureID*)App->scene->icon_options_transform, ImVec2(13, 13), ImVec2(-1, 1), ImVec2(0, 0)))
	{
		ImGui::OpenPopup("OptionsMesh");
	}
	ImGui::PopStyleVar();

	// Button Options --------------------------------------
	if (ImGui::BeginPopup("OptionsMesh"))
	{
		ShowOptions();
		ImGui::EndPopup();
	}

	ImGui::TreePop();
}

void CompCanvasRender::CopyValues(const CompCanvasRender* component)
{
	//more...
}

void CompCanvasRender::Save(JSON_Object* object, std::string name, bool saveScene, uint& countResources) const
{
	json_object_dotset_string_with_std(object, name + "Component:", name_component);
	json_object_dotset_number_with_std(object, name + "Type", this->GetType());
	json_object_dotset_number_with_std(object, name + "UUID", uid);
	//...
}

void CompCanvasRender::Load(const JSON_Object* object, std::string name)
{
	uid = json_object_dotget_number_with_std(object, name + "UUID");
	//...
	Enable();
}

void CompCanvasRender::AddToCanvas()
{
	if (my_canvas == nullptr)
	{
		LOG("ERROR:CanvasRender don't found");
		return;
	}
	
	my_canvas->AddCanvasRender(this);
}

void CompCanvasRender::RemoveFromCanvas()
{
	if (my_canvas == nullptr)
	{
		LOG("ERROR:CanvasRender with UID &i don't have Canvas", parent->GetUUID());
		return;
	}
	my_canvas->RemoveCanvasRender(this);

}

void CompCanvasRender::ProcessImage(CompImage * image)
{
	graphic = image;
		//UV Setup
//-x+y	//0,1-------1,1 //+x+y
		//|	3     /	2|
		//|		/	 |
		//|	0 /		1|
//-x-y	//0,0-------1,0 //+x-y
	
	//Clear All Vertices and indices
	vertices.clear();
	indices.clear();


		CanvasVertex ver;
		ver.position = parent->GetComponentRectTransform()->GetSouthWestPosition();
		ver.tex_coords=float2(0.0f,0.0f);
		vertices.push_back(ver);

		ver.position = parent->GetComponentRectTransform()->GetSouthEastPosition();
		ver.tex_coords = float2(1.0f, 0.0f);
		vertices.push_back(ver);

		ver.position = parent->GetComponentRectTransform()->GetNorthEastPosition();
		ver.tex_coords = float2(1.0f, 1.0f);
		vertices.push_back(ver);

		ver.position = parent->GetComponentRectTransform()->GetNorthWestPosition();
		ver.tex_coords = float2(0.0f, 1.0f);
		vertices.push_back(ver);

		uint lastIndex = 0;

		indices.push_back(lastIndex);
		indices.push_back(lastIndex + 1);
		indices.push_back(lastIndex + 2);
		indices.push_back(lastIndex + 2);
		indices.push_back(lastIndex + 3);
		indices.push_back(lastIndex );

		//----
		glGetError();
		glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
		CheckOpenGlError("glBindBuffer vertices");
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CanvasVertex), &vertices[0], GL_STATIC_DRAW);
		CheckOpenGlError("glBufferData vertices");
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CheckOpenGlError("glBindBuffer vertices 0");


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		CheckOpenGlError("glBindBuffer indices");
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);
		CheckOpenGlError("glBufferData indices");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		CheckOpenGlError("glBindBuffer indices 0");

}

void CompCanvasRender::PorcessText(CompText * text)
{
	graphic = text;

}

void CompCanvasRender::DrawGraphic()
{
	if (graphic == nullptr)
	{
		return;
	}

	glPushMatrix();
	glMultMatrixf((float*)&graphic->GetRectTrasnform()->GetGlobalTransform().Transposed());
//	App->renderer3D->default_shader->Bind();
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glGetError();

	/*
	if (graphic->GetTextureID() != -1)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		CheckOpenGlError("glBindTexture color");
		//glColor4f(image->GetImage()->GetRGBA().x, image->GetImage()->GetRGBA().y, image->GetImage()->GetRGBA().z, image->GetImage()->GetRGBA().w);
	}
	*/
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertices_id);
	CheckOpenGlError("glBindBuffer vertices_id");
	glVertexPointer(3, GL_FLOAT, sizeof(CanvasVertex), NULL);
	CheckOpenGlError("glVertexPointer position");
	//glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normals));
	glTexCoordPointer(2, GL_FLOAT, sizeof(CanvasVertex), (void*)offsetof(CanvasVertex, tex_coords));
	CheckOpenGlError("glTexCoordPointer tex_coords");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CheckOpenGlError("glBindBuffer vertices_id 0");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indices_id);
	CheckOpenGlError("glBindBuffer indices_id");
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
	CheckOpenGlError("glDrawElements indices");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CheckOpenGlError("glBindBuffer indices_id 0 ");

	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
//	App->renderer3D->default_shader->Unbind();

}

void CompCanvasRender::SetGraphic(CompGraphic * set_graphic)
{
	graphic = set_graphic;
}

void CompCanvasRender::CheckOpenGlError(std::string info)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		LOG("[error]Error %s, %s\n", info.c_str(),gluErrorString(error));
	}
}