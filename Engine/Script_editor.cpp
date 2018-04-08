#include "Script_editor.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleFS.h"
#include "ModuleImporter.h"
#include "ImportScript.h"
#include "Resource_.h"
#include "ResourceScript.h"
#include "JSONSerialization.h"

#include <filesystem>
#include <iostream>
#include <experimental/filesystem>
#include <fstream>


Script_editor::Script_editor(ResourceScript* parent) : parent(parent)
{
}


Script_editor::~Script_editor()
{
	editor.Delete();
}

void Script_editor::Start(std::string nameScript, bool isnew)
{
	name = nameScript;
	if (isnew)
	{
		// Create new Script with base -----
		SetInitInfo(nameScript);
	}
	else
	{
		// Get All string ----
		nameScript = "Assets\\" + nameScript + ".cs";
		editor.SetText(App->fs->LoadScript(nameScript));
	}

	editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
}

void Script_editor::SetInitInfo(std::string nameScript)
{
	editor.InsertText("// ------- New Script Culverin Engine 3D ------- \n\n");
	editor.InsertText("using CulverinEditor;\n");
	editor.InsertText("using CulverinEditor.Debug;\n\n");

	editor.InsertText("public class ");
	editor.InsertText(nameScript);
	//editor.InsertText(" : CulverinEditor\n");
	editor.InsertText("{\n\n");

	editor.InsertText("	// Use this for initialization\n");
	editor.InsertText("	void Start() {\n\n");

	editor.InsertText("	}\n\n");

	editor.InsertText("	// Update is called once per frame\n");
	editor.InsertText("	void Update() {\n\n");

	editor.InsertText("	}\n");
	editor.InsertText("}\n");
}

void Script_editor::Show(bool& active)
{
	//ImGui::PushStyleVar() // Center
	ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &active, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", NULL, false, false))
			{
			}
			if (ImGui::MenuItem("Save"))
			{
				SaveScript(true);
			}
			if (ImGui::MenuItem("Close", NULL, &active))
			{
				active = false;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl + Z"))
			{
				SetAction(ActionEditor::UNDO);
			}
			if (ImGui::MenuItem("ReUndo", "Ctrl + Shift + Z"))
			{
				SetAction(ActionEditor::REUNDO);
			}
			bool activecopy = false;
			if (editor.GetSelectedText() != "")
			{
				activecopy = true;
			}
			if (ImGui::MenuItem("Copy", "Ctrl + C", false, activecopy))
			{
				SetAction(ActionEditor::COPY);
			}
			bool activepaste = false;
			if (text_selected != "")
			{
				activepaste = true;
			}
			if (ImGui::MenuItem("Paste", "Ctrl + V", false, activepaste))
			{
				SetAction(ActionEditor::PASTE);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushFont(io.Fonts->Fonts[1]);
	editor.Render("Render...");
	ImGui::PopFont();

	ImGui::End();
}

void Script_editor::SetAction(ActionEditor action)
{
	switch (action)
	{
	case ActionEditor::COPY:
	{
		text_selected = editor.GetSelectedText();
		break;
	}
	case ActionEditor::PASTE:
	{
		editor.InsertText(text_selected);
		break;
	}
	case ActionEditor::UNDO:
	{
		editor.Undo();
		break;
	}
	case ActionEditor::REUNDO:
	{
		editor.Redo();
		break;
	}
	case ActionEditor::RETURN:
	{
		editor.InsertText("\n");
		break;
	}
	}
}

void Script_editor::SaveScript(bool ReImport)
{
	App->fs->SaveScript(name, editor, DIRECTORY_IMPORT::IMPORT_DIRECTORY_ASSETS);
	if (ReImport)
	{
		//App->resource_manager->resources_to_reimport.push_back(App->json_seria->GetUUIDScript(App->fs->GetFullPath(parent->GetPathAssets()).c_str()));

		if (App->importer->iScript->ReImportScript(parent->GetPathAssets(), std::to_string(parent->GetUUID()), parent))
		{
			LOG("ReImported Succesfully Script: %s", App->fs->GetOnlyName(parent->GetPathAssets()).c_str());
		}
		else
		{
			parent->SetState(Resource::State::FAILED);
			LOG("[error] ReImported Failed Script: %s", App->fs->GetOnlyName(parent->GetPathAssets()).c_str());
		}
	}
}
