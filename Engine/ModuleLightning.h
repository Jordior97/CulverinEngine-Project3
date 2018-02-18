#ifndef __MODULELIGHTNING_H__
#define __MODULELIGHTNING_H__

#include "Module.h"

#include "ModuleFramebuffers.h"

#include <vector>

#define DEFAULT_SHADOW_CAST_POINTS_COUNT 4

class CompLight;
class ShaderProgram;
class DepthFrameBuffer
{
public:
	DepthFrameBuffer();
	DepthFrameBuffer(int width, int height);
	~DepthFrameBuffer();

	void Create(int width, int height);
	void Destroy();
	void Resize(int width, int height);


	uint GetTexture()const;

public:
	int width = 0;
	int height = 0;
	uint frame_id = 0;
	uint texture = 0;
};

class ModuleLightning : public Module
{
public:
	ModuleLightning(bool start_enabled = true);
	virtual ~ModuleLightning();

	bool Init(JSON_Object* node) override;
	bool Start() override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	update_status UpdateConfig(float dt) override;
	bool SaveConfig(JSON_Object* node) override;
	bool CleanUp() override;
	void OnEvent(Event& event);

	void CalShadowMaps();
	void SetShadowCastPoints(uint points);
	void ResizeShadowMaps(uint w_res, uint h_res);

	std::vector<DepthFrameBuffer>* GetShadowMaps();
	void GetShadowMapsResolution(uint& w_res, uint& h_res);

	void OnLightCreated(CompLight* l);
	void OnLightDestroyed(CompLight* l);

	std::vector<CompLight*> GetSceneLights()const;
	void PushLight(CompLight* light);
	void DeleteLight(CompLight* light);

	bool SetEventListenrs();

	
private:
	void AddShadowMapCastViews(uint ammount);

public:
	//test
	ShaderProgram * shadow_Shader = nullptr;

	FrameBuffer text;
	ResourceMesh*  light_UI_plane = nullptr;
	uint texture_bulb = 0;
private:
	uint shadow_cast_points_count = DEFAULT_SHADOW_CAST_POINTS_COUNT; // This value should be able to change from config and modiffied on load
	std::vector<DepthFrameBuffer> shadow_maps;
	uint shadow_maps_res_w = 1024;
	uint shadow_maps_res_h = 1024;


	std::vector<CompLight*> scene_lights;
	std::vector<CompLight*> frame_used_lights;

	
};

#endif // __MODULELIGHTNING_H__