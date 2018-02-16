#ifndef _CSHARPSCRIPT_
#define _CSHARPSCRIPT_

// Info-> http://docs.go-mono.com/?link=root:/embed

#define DefaultParam 0

#include "Globals.h"
#include <string>
#include <vector>
#include <map>
#include <mono/metadata/metadata.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>

class CSharpScript;
class GameObject;
class Component;
typedef struct json_object_t JSON_Object;

enum FunctionBase
{
	CS_Start, CS_Update, CS_FixedUpdate, CS_OnGUI, CS_OnEnable, CS_OnDisable
};

enum VarType
{
	Var_UNKNOWN = -1,
	Var_INT = 0,
	Var_FLOAT = 1,
	Var_BOOL = 2,
	Var_STRING = 3,
	Var_CLASS = 4,
	Var_GAMEOBJECT = 5
};

enum VarAccess
{
	Var_PUBLIC = 0,
	Var_PRIVATE,
};

class ScriptVariable
{
public:
	ScriptVariable(const char* name, VarType type, VarAccess access, CSharpScript* script);
	virtual ~ScriptVariable();

	void SetMonoValue(void* newVal);
	void EreaseMonoValue(void* newVal);
	void SetMonoField(MonoClassField* mfield);
	void SetMonoType(MonoType* mtype);

public:
	const char* name = nullptr;
	VarType type = Var_UNKNOWN;
	void* value = nullptr;
	std::string str_value;
	GameObject* game_object = nullptr;
	bool select_game_object = false; //Only use for GameObjects
	VarAccess access = VarAccess::Var_PRIVATE;

private:
	//Mono properties to link with he script
	MonoClassField* mono_field = nullptr;
	MonoType* mono_type = nullptr;

	//To access the script
	CSharpScript* script = nullptr;
};


struct MainMonoMethod
{
	FunctionBase type;
	MonoMethod* method = nullptr;
};

class CSharpScript
{
public:
	CSharpScript();
	~CSharpScript();

	//Importing/Reimporting/Initializing --------
	void LoadScript();
	bool ReImport(std::string pathdll);
	void Clear();
	MainMonoMethod CreateMainFunction(std::string function, int parameters, FunctionBase type);
	void DoMainFunction(FunctionBase function);
	void DoFunction(MonoMethod* function, void ** parameter);

	bool CheckMonoObject(MonoObject* object);

	//GET functions ------------------------------------
	MonoObject* GetMonoObject() const;
	MonoClass* GetMonoClass() const;

	// Functions set initial info ------------------------
	void SetDomain(MonoDomain* domain);
	void SetAssembly(MonoAssembly* assembly);
	void SetImage(MonoImage* image);
	void SetClass(MonoClass* klass);
	void SetClassName(std::string _name);
	void SetNameSpace(std::string _name_space);


	//Variables METHODS -------------------------------------------------
	void ResetScriptVariables();
	void CreateOwnGameObject();
	void GetScriptVariables();
	void UpdateScriptVariables();
	void RemoveReferences(GameObject* go);

	VarType GetTypeFromMono(MonoType* mtype);
	bool GetValueFromMono(ScriptVariable* variable, MonoClassField* mfield, MonoType* mtype);
	bool UpdateValueFromMono(ScriptVariable* variable, MonoClassField* mfield, MonoType* mtype);
	bool LinkVarToMono(ScriptVariable* variable, MonoClassField* mfield, MonoType* mtype);
	void SetVarValue(ScriptVariable* variable, void* new_val);
	// ------------------------------------------------------------------

	/*Input*/
	MonoObject* GetMousePosition();

	/*Game Object*/
	mono_bool	IsActive(MonoObject* object);
	void		SetActive(MonoObject* object, mono_bool active);
	void		SetName(MonoObject* object, MonoString* name);
	MonoString* GetName(MonoObject* object);
	void		SetTag(MonoObject* object, MonoString* tag);
	MonoString* GetTag(MonoObject* object);

	MonoObject* GetOwnGameObject();
	void		SetCurrentGameObject(GameObject* current);
	void		SetOwnGameObject(GameObject* gameobject);
	void		CreateGameObject(MonoObject* object);
	bool		DestroyGameObject(MonoObject* object);

	MonoObject* GetPosition(MonoObject* object);
	void		SetPosition(MonoObject* object, MonoObject* vector3);
	MonoObject* GetRotation(MonoObject* object);
	void		SetRotation(MonoObject* object, MonoObject* vector3);
	void		IncrementRotation(MonoObject* object, MonoObject* vector3);

	/*Audio*/
	void		PlayAudioEvent(MonoObject* object, MonoString* event_name);
	void		StopAudioEvent(MonoObject* object, MonoString* event_name);


	/*Components*/
	MonoObject* GetComponent(MonoObject* object, MonoReflectionType* type);
	MonoObject* GetParentGameObject();

	/*Childs*/
	MonoObject* Find(MonoObject* object, MonoString* name);

	// Map --------------------
	MonoString* GetMapString(MonoObject* object);

	// LOAD - SAVE METHODS ------------------
	void Save(JSON_Object* object, std::string name) const;
	void Load(const JSON_Object* object, std::string name);
	void LoadValues();

public:
	//Variables/Info containers (public to have access through other modules)
	std::vector<ScriptVariable*> variables;

private:
	//Auxiliar map to fill variables vector with info
	std::map<MonoClassField*, MonoType*> field_type;
	std::string name;
	std::string name_space;

	MonoDomain* CSdomain = nullptr;
	MonoAssembly* CSassembly = nullptr;
	MonoImage* CSimage = nullptr;
	MonoClass* CSClass = nullptr;
	MonoObject* CSObject = nullptr;
	MonoObject* CSSelfObject = nullptr;
	GameObject* own_game_object = nullptr;

	// Main Functions
	MainMonoMethod Start;
	MainMonoMethod Update;
	MainMonoMethod FixedUpdate;
	MainMonoMethod OnGUI;
	MainMonoMethod OnEnable;
	MainMonoMethod OnDisable;

	GameObject* current_game_object = nullptr;
	std::vector<uint> re_load_values;

	/* Components */
	std::vector<Component*> temp;

public:
	std::map<MonoObject*, GameObject*> game_objects;

};

#endif


