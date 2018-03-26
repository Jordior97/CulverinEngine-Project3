#ifndef _RESOURCEMANAGER_
#define _RESOURCEMANAGER_

#include "Module.h"
#include "Resource_.h"
#include <map>
#include <vector>
#include <list>
#include "Math\float3.h"

#define ResourcePrimitive 2

struct Vertex;

struct ReImport
{
	uint uuid = 0;
	const char* name_mesh = nullptr;
	const char* directory_obj = nullptr;
	const char* path_dll = nullptr; //Only use Script 
};

class ModuleResourceManager : public Module
{
public:
	ModuleResourceManager(bool start_enabled = true);
	virtual ~ModuleResourceManager();

	bool Init(JSON_Object* node);
	bool Start();
	update_status PreUpdate(float dt);
	//update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void ImportFile(std::list<const char*>& file);
	void ImportFile(std::vector<const char*>& file, std::vector<ReImport>& resourcesToReimport, bool auto_reimport = false);
	
	Resource* CreateNewResource(Resource::Type type, uint uuid = 0);
	Resource* GetResource(uint id);
	Resource* GetResource(const char* name);
	Resource::Type CheckFileType(const char* filedir);

	void Init_IndexVertex(float3* vertex_triangulate, uint num_index, std::vector<uint>& indices, std::vector<float3>& vertices);
	void CreateResourceCube();
	void CreateResourcePlane();

	Resource* ShowResources(bool& active, Resource::Type type);
	void ShowAllResources(bool& active);
	bool ReImportAllScripts();

	// Check if a resources haven't file imported in Library
	void CheckLibrary();

	void NewLoad();

public:
	
	std::vector<ReImport> resources_to_reimport;
	std::vector<uint> files_to_delete;

	bool reimportAll = false;

private:
	
	std::map<uint, Resource*> resources;
	std::vector<const char*> files_reimport;
	bool reimport_now = false;
	bool delete_now = false;
	bool load_resources = true;
	bool reimported_scripts = false;
	bool scripts_set_normal = false;
};

#endif