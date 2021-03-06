#include <experimental\filesystem>

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "CompCamera.h"
#include "Scene.h"
#include "GameObject.h"
#include "WindowSceneWorld.h"
#include "parson.h"
#include "SkyBox.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderGui.h"
#include "SDL/include/SDL_opengl.h"
#include "GL3W/include/glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Devil\include\ilut.h"
#include "DefaultShaders.h"
#include "Materials.h"
#include "ModuleTextures.h"
#include "ModuleLightning.h"
#include "ModuleGUI.h"
#include "ModuleResourceManager.h"
#include "ImportMaterial.h"
#include "ResourceMaterial.h"

#pragma comment (lib, "Devil/libx86/DevIL.lib")
#pragma comment (lib, "Devil/libx86/ILU.lib")
#pragma comment (lib, "Devil/libx86/ILUT.lib")

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	Awake_enabled = true;
	Start_enabled = true;
	preUpdate_enabled = true;
	postUpdate_enabled = true;

	have_config = true;

	name = "Renderer";
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{

}

// Called before render is available
bool ModuleRenderer3D::Init(JSON_Object* node)
{
	perf_timer.Start();

	LOG("Creating 3D Renderer context");
	bool ret = true;

	//Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if (ret == true)
	{
		//Use Vsync
		if (App->GetVSYNC() && SDL_GL_SetSwapInterval(1) < 0)
			LOG("[error]Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);

		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		error = glewInit();
		if (error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing GL3W! %s\n", gluErrorString(error));
			ret = false;
		}

		//Load render config info -------
		depth_test = json_object_get_boolean(node, "Depth Test");
		cull_face = json_object_get_boolean(node, "Cull Face");
		lighting = json_object_get_boolean(node, "Lighting");
		color_material = json_object_get_boolean(node, "Color Material");
		texture_2d = json_object_get_boolean(node, "Texture 2D");
		wireframe = json_object_get_boolean(node, "Wireframe");
		normals = json_object_get_boolean(node, "Normals");
		smooth = json_object_get_boolean(node, "Smooth");

		dmg_texture_uid = json_object_get_number(node, "dmg_texture_uid");
		dmg_texture_name = json_object_get_string(node, "dmg_texture_name");

		node = json_object_get_object(node, "Fog");
		fog_active = json_object_get_boolean(node, "Active");
		fog_density = json_object_get_number(node, "Density");
	}

	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 32; j++) {
			//int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkImage[i][j][0] = (GLubyte)255;
			checkImage[i][j][1] = (GLubyte)255;
			checkImage[i][j][2] = (GLubyte)255;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &id_checkImage);
	glBindTexture(GL_TEXTURE_2D, id_checkImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	Texture default_text;
	default_text.name = "default_texture";
	default_text.id = id_checkImage;

	default_texture = new ResourceMaterial(5);
	default_texture->Init(default_text);


	default_shader = App->module_shaders->CreateDefaultShader("Default Shader", fragmentShaderSource, vertexShaderSource, nullptr, true);
	lights_billboard_shader = App->module_shaders->CreateDefaultShader("Billboard Lights Shader", DefaultFrag, DefaultVert, nullptr,true);

	particles_shader = App->module_shaders->CreateDefaultShader("Particles Shader", DefaultFrag, DefaultVert, nullptr, true);
	non_glow_shader = App->module_shaders->CreateDefaultShader("Non Glow Shader", NonGlowFrag, DefaultVert, nullptr, true);
	non_glow_skinning_shader = App->module_shaders->CreateDefaultShader("Non Glow Skinning Shader", NonGlowFrag, SkinningVert, nullptr, true);
	blur_shader_tex = App->module_shaders->CreateDefaultShader("Blur Shader", BlurFrag, TextureVert, nullptr, true);
	final_shader_tex = App->module_shaders->CreateDefaultShader("Texture Shader", FinalFrag, TextureVert, nullptr, true);
	cube_map_shader = App->module_shaders->CreateDefaultShader("CubeMapShader", CubeMapFrag, CubeMapVert, nullptr, true);

	non_glow_material = new Material();
	non_glow_material->name = "Non Glow Material";
	non_glow_material->material_shader = non_glow_shader;
	non_glow_material->GetProgramVariables();
	App->module_shaders->materials.insert(std::pair<uint, Material*>(non_glow_material->GetProgramID(), non_glow_material));
	non_glow_material->active_num = 1;

	non_glow_skinning_material = new Material();
	non_glow_skinning_material->name = "Non Glow Skinning Material";
	non_glow_skinning_material->material_shader = non_glow_skinning_shader;
	non_glow_skinning_material->GetProgramVariables();
	App->module_shaders->materials.insert(std::pair<uint, Material*>(non_glow_skinning_material->GetProgramID(), non_glow_skinning_material));
	non_glow_skinning_material->active_num = 1;

	final_tex_material = new Material();
	final_tex_material->name = "Final Tex Material";
	final_tex_material->material_shader = final_shader_tex;
	final_tex_material->GetProgramVariables();
	App->module_shaders->materials.insert(std::pair<uint, Material*>(final_tex_material->GetProgramID(), final_tex_material));
	final_tex_material->active_num = 1;

	// Dmg texture
	if (dmg_texture_uid != 0)
	{
		// Load it
		dmg_texture_res = dynamic_cast<ResourceMaterial*>(App->resource_manager->GetResource(dmg_texture_name.c_str()));
	}
	else
	{
		// Create a new one
		App->importer->Import("Assets/bloodHurt2.png", Resource::Type::MATERIAL);
		dmg_texture_res = dynamic_cast<ResourceMaterial*>(App->resource_manager->GetResource("bloodHurt2.png"));
	}

	if (dmg_texture_res)
	{
		App->importer->iMaterial->LoadResource(std::to_string(dmg_texture_res->GetUUID()).c_str(), dmg_texture_res);
		dmg_texture_res->num_game_objects_use_me++;
	}
	else
	{
		LOG("ERROR: Could not load damage texture resource");
	}

	default_material = new Material();
	default_material->name = "Default Material";
	default_material->material_shader = default_shader;
	default_material->GetProgramVariables();

	if (default_material->textures.size()>0)
		default_material->textures[0].value = default_texture;

	Awake_t = perf_timer.ReadMs();
	return ret;
}

bool ModuleRenderer3D::Start()
{
	perf_timer.Start();

	blur_amount = 15;
	blur_scale = 1.0f;
	blur_strength = 0.3f;
	GLfloat cube_vertices[] = {
		// front

		-1, -1,  0.0f,
		1, -1,  0.0f,
		1,  1,  0.0f,
		-1,  1, 0.0f,
	};

	uint cube_elements[] = {
		// front
		0, 1, 2, 3

	};

	static const GLfloat g_UV_buffer_data[] = {
		0.0f, 0.0f,
		1.0f,  0.0f,
		1.0f,  1.0f,
		0.0f, 1.0f,
	};

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), cube_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &UVbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_UV_buffer_data), g_UV_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(uint), cube_elements, GL_STATIC_DRAW);

	(depth_test) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	(cull_face) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	(lighting) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
	(color_material) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
	(texture_2d) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
	(smooth) ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);

	if (fog_active)
	{
		glEnable(GL_FOG);
		glFogfv(GL_FOG_DENSITY, &fog_density);
	}

	Start_t = perf_timer.ReadMs();
	return true;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	BROFILER_CATEGORY("PreUpdate: ModuleRenderer3D", Profiler::Color::Blue);
	perf_timer.Start();

	App->scene->scene_buff->Init("Scene");
	App->scene->final_buff->Init("Scene");
	App->scene->glow_buff->Init("Scene");
	App->scene->horizontal_blur_buff->Init("Scene");
	App->scene->vertical_blur_buff->Init("Scene");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	preUpdate_t = perf_timer.ReadMs();
	return UPDATE_CONTINUE;
}


// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	BROFILER_CATEGORY("PostUpdate: ModuleRenderer3D", Profiler::Color::Blue);
	perf_timer.Start();

	App->scene->horizontal_blur_buff->Bind("Scene");
	BlurShaderVars(0);
	RenderSceneWiewport();
	App->scene->horizontal_blur_buff->UnBind("Scene");

	App->scene->vertical_blur_buff->Bind("Scene");
	BlurShaderVars(1);
	RenderSceneWiewport();
	App->scene->vertical_blur_buff->UnBind("Scene");

	if (!App->mode_game)
		App->scene->final_buff->Bind("Scene");
	glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
	GlowShaderVars();
	RenderSceneWiewport();

	App->render_gui->ScreenSpaceDraw();
	App->scene->final_buff->UnBind("Scene");


	if (App->mode_game == false)
	{
		if (App->gui->window_Test_renderer)
		{
			if (ImGui::Begin("Test", &App->gui->window_Test_renderer))
			{
				ImGui::Image((ImTextureID*)App->module_lightning->test_fix.depthTex, ImVec2(256, 256));
				ImGui::SliderFloat("Strength", &blur_strength, 0.0f, 50.0f);
				ImGui::SliderInt("Amount", &blur_amount, 0.0f, 30.0f);
				ImGui::SliderFloat("Scale", &blur_scale, 0.0f, 50.0f);
			}
			ImGui::End();
		}
	}

	ImGui::Render();

	screenshot.TakeFullScreen();
	screenshot.TakePartScreen();
	gif.TakeFullScreen(dt);
	gif.TakePartScreen(dt);

	SDL_GL_SwapWindow(App->window->window);

	postUpdate_t = perf_timer.ReadMs();
	return UPDATE_CONTINUE;
}

update_status ModuleRenderer3D::UpdateConfig(float dt)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 3));
	if (ImGui::Checkbox("Depth Test", &depth_test))
	{
		(depth_test) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}
	if (ImGui::Checkbox("Cull Face", &cull_face))
	{
		(cull_face) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	}
	if (ImGui::Checkbox("Lighting", &lighting))
	{
		(lighting) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
	}
	if (ImGui::Checkbox("Color Material", &color_material))
	{
		(color_material) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
	}
	if (ImGui::Checkbox("Texture 2D", &texture_2d))
	{
		(texture_2d) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
	}

	ImGui::Checkbox("Wireframe", &wireframe);

	ImGui::Checkbox("Normals", &normals);

	if (ImGui::Checkbox("Smooth", &smooth))
	{
		(smooth) ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
	}
	if (ImGui::CollapsingHeader("Fog"))
	{
		if (ImGui::Checkbox("Active", &fog_active))
		{
			if (fog_active)
			{
				glEnable(GL_FOG);
				glFogfv(GL_FOG_DENSITY, &fog_density);
			}
			else
			{
				glDisable(GL_FOG);
			}
		}

		if (fog_active)
		{
			if (ImGui::SliderFloat("Density", &fog_density, 0.0f, 1.0f, "%.3f"))
			{
				glFogfv(GL_FOG_DENSITY, &fog_density);
			}
		}
	}

	ImGui::Text("Dmg texture: "); ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d - %s", dmg_texture_res->GetUUID(), dmg_texture_res->name.c_str());

	static bool dmg_tex_selected = false;
	if(ImGui::ImageButton((ImTextureID*)dmg_texture_res->GetUUID(), ImVec2(64, 64), ImVec2(-1, 1), ImVec2(0, 0)))
	{
		dmg_tex_selected = true;
	}

	if(dmg_tex_selected)
	{
		ResourceMaterial* tmp = dynamic_cast<ResourceMaterial*>(App->resource_manager->ShowResources(dmg_tex_selected, Resource::Type::MATERIAL));

		if (tmp)
		{
			if(tmp != dmg_texture_res)
			{
				// Unload the old one
				if (dmg_texture_res->num_game_objects_use_me > 0)
					dmg_texture_res->num_game_objects_use_me--;

				// Reassign the damage texture and load it
				dmg_texture_res = tmp;
				if(dmg_texture_res->IsLoadedToMemory() == Resource::State::UNLOADED)
				{
					App->importer->iMaterial->LoadResource(std::to_string(dmg_texture_res->GetUUID()).c_str(), dmg_texture_res);
					dmg_texture_res->num_game_objects_use_me++;
				}
				
				dmg_tex_selected = false;
			}
		}
	}

	ImGui::Image((ImTextureID*)dmg_texture_res->GetTextureID(), ImVec2(200, 200)); // TODO: If the image button would display the image this is not necessary

	ImGui::PopStyleVar();
	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::SaveConfig(JSON_Object * node)
{
	//Save render config info -------
	json_object_set_boolean(node, "Depth Test", depth_test);
	json_object_set_boolean(node, "Cull Face", cull_face);
	json_object_set_boolean(node, "Lighting", lighting);
	json_object_set_boolean(node, "Color Material", color_material);
	json_object_set_boolean(node, "Texture 2D", texture_2d);
	json_object_set_boolean(node, "Wireframe", wireframe);
	json_object_set_boolean(node, "Normals", normals);
	json_object_set_boolean(node, "Smooth", smooth);

	json_object_set_number(node, "dmg_texture_uid", dmg_texture_res->GetUUID());
	json_object_set_string(node, "dmg_texture_name", dmg_texture_res->name.c_str());

	node = json_object_get_object(node, "Fog");
	json_object_set_boolean(node, "Active", fog_active);
	json_object_set_number(node, "Density", fog_density);
	return true;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");
	

	SDL_GL_DeleteContext(context);

	return true;
}

void ModuleRenderer3D::SetActiveCamera(CompCamera* cam)
{
	active_camera = cam;
}

void ModuleRenderer3D::SetSceneCamera(CompCamera* cam)
{
	scene_camera = cam;
}

void ModuleRenderer3D::SetGameCamera(CompCamera* cam)
{
	game_camera = cam;
}

CompCamera * ModuleRenderer3D::GetActiveCamera()
{
	return active_camera;
}

void ModuleRenderer3D::UpdateProjection(CompCamera* cam)
{
	if (cam != nullptr)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glLoadMatrixf(cam->GetProjectionMatrix());

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	screenshot.Stop();
	gif.Stop();

	float ratio = (float)width / (float)height;
	App->window->SetWindowSize(width, height);

	//Reset the FOV
	active_camera->SetFov(active_camera->GetFOV());
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glLoadMatrixf(active_camera->GetProjectionMatrix());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

float2 ModuleRenderer3D::LoadImage_devil(const char * theFileName, GLuint *buff)
{
	float2 texture_w_h;
	//Texture loading success
	bool textureLoaded = false;

	//Generate and set current image ID
	uint imgID = 0;
	ilGenImages(1, &imgID);
	ilBindImage(imgID);

	//Load image
	ILboolean success = ilLoadImage(theFileName);

	//Image loaded successfully
	if (success == IL_TRUE)
	{
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		//Convert image to RGBA
		success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		if (success == IL_TRUE)
		{
			textureLoaded = loadTextureFromPixels32((GLuint*)ilGetData(), (GLuint)ilGetInteger(IL_IMAGE_WIDTH), (GLuint)ilGetInteger(IL_IMAGE_HEIGHT), buff);
			texture_w_h.x = (uint)ilGetInteger(IL_IMAGE_WIDTH); texture_w_h.y = (uint)ilGetInteger(IL_IMAGE_HEIGHT);
			//Create texture from file pixels
			textureLoaded = true;
		}

		//Delete file from memory
		ilDeleteImages(1, &imgID);
	}

	return texture_w_h;

}

bool ModuleRenderer3D::loadTextureFromPixels32(GLuint * id_pixels, GLuint width_img, GLuint height_img, GLuint *buff)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, buff);
	glBindTexture(GL_TEXTURE_2D, *buff);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_img, height_img, 0, GL_RGBA, GL_UNSIGNED_BYTE, id_pixels);
	glBindTexture(GL_TEXTURE_2D, NULL);

	//Check for error
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading texture from %p pixels! %s\n", id_pixels, gluErrorString(error));
		return false;
	}

	return true;
}

void ModuleRenderer3D::RenderSceneWiewport()
{

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//DRAW QUAD
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(0);
}

void ModuleRenderer3D::BlurShaderVars(int i)
{
	glViewport(0, 0, 128, 128);
	blur_shader_tex->Bind();

	glActiveTexture(GL_TEXTURE0);
	GLint texLoc = glGetUniformLocation(blur_shader_tex->programID, "albedo");

	if (i == 0)
		glBindTexture(GL_TEXTURE_2D, App->scene->glow_buff->GetTexture());
	else
		glBindTexture(GL_TEXTURE_2D, App->scene->horizontal_blur_buff->GetTexture());
	glUniform1i(texLoc, 0);


	GLint orientLoc = glGetUniformLocation(blur_shader_tex->programID, "_orientation");
	glUniform1i(orientLoc, i);

	GLint amountLoc = glGetUniformLocation(blur_shader_tex->programID, "BlurAmount");
	glUniform1i(amountLoc, blur_amount);

	GLint scaleLoc = glGetUniformLocation(blur_shader_tex->programID, "BlurScale");
	glUniform1f(scaleLoc, blur_scale);

	GLint strengthLoc = glGetUniformLocation(blur_shader_tex->programID, "BlurStrength");
	glUniform1f(strengthLoc, blur_strength);


}

void ModuleRenderer3D::GlowShaderVars()
{
	final_shader_tex->Bind();
	App->module_shaders->SetUniformVariables(final_tex_material);
	glActiveTexture(GL_TEXTURE0);
	GLint texLoc = glGetUniformLocation(final_shader_tex->programID, "_albedo");
	glBindTexture(GL_TEXTURE_2D, App->scene->scene_buff->GetTexture());
	glUniform1i(texLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	texLoc = glGetUniformLocation(final_shader_tex->programID, "_glow_tex");
	glBindTexture(GL_TEXTURE_2D, App->scene->vertical_blur_buff->GetTexture());
	glUniform1i(texLoc, 1);

	glActiveTexture(GL_TEXTURE2);
	texLoc = glGetUniformLocation(final_shader_tex->programID, "_dmg_tex");
	glBindTexture(GL_TEXTURE_2D, dmg_texture_res->GetTextureID());
	glUniform1i(texLoc, 2);

}