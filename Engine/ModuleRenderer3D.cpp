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
#include"Devil\include\ilut.h"
#include "DefaultShaders.h"
#include "Materials.h"

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
	RELEASE(particles_shader);
	RELEASE(lights_billboard_shader);
	RELEASE(default_material);
	RELEASE(default_texture);
}

// Called before render is available
bool ModuleRenderer3D::Init(JSON_Object* node)
{
	perf_timer.Start();

	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);

	if(context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if(ret == true)
	{
		//Use Vsync
		if(App->GetVSYNC() && SDL_GL_SetSwapInterval(1) < 0)
			LOG("[error]Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
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
		if(error != GL_NO_ERROR)
		{
			LOG("[error]Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
	
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
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

	default_texture = new ResourceMaterial(App->random->Int());
	default_texture->Init(default_text);


	default_shader = App->module_shaders->CreateDefaultShader("Default Shader", fragmentShaderSource, vertexShaderSource, nullptr, true);
	lights_billboard_shader = App->module_shaders->CreateDefaultShader("Billboard Lights Shader", DefaultFrag, DefaultVert, nullptr);

	particles_shader = App->module_shaders->CreateDefaultShader("Particles Shader", DefaultFrag, DefaultVert, nullptr);

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
	perf_timer.Start();

	// 
	if (App->mode_game == false)
	{
		App->scene->scene_buff->Bind("Scene");
	}

	// Refresh Projection of the camera
	UpdateProjection(active_camera);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(active_camera->GetViewMatrix());

	// light 0 on cam pos
	
	preUpdate_t = perf_timer.ReadMs();
	return UPDATE_CONTINUE;
}

//update_status ModuleRenderer3D::Update(float dt)
//{
//	perf_timer.Start();
//	Update_t = perf_timer.ReadMs();
//	return UPDATE_CONTINUE;
//}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	perf_timer.Start();
	
	//All draw functions moved to Scene Update

	// Draw Skybox (direct mode for now)
	//if (App->scene->draw_skybox)
	//{
	//	App->scene->skybox->DrawSkybox(800, active_camera->frustum.pos, App->scene->skybox_index);
	//}

	//Draw Test Cube
	//App->scene->DrawCube(5);

	// Draw Plane
	//App->scene->DrawPlane();

	// Draw GameObjects
	//App->scene->root->Draw();
	

	// Draw Quadtree
	//if (App->scene->quadtree_draw)
	//{
	//	App->scene->quadtree.DebugDraw();
	//}

	// Draw GUI
	App->render_gui->ScreenSpaceDraw();
	
	// Draw Mouse Picking Ray
	//glBegin(GL_LINES);
	//glLineWidth(3.0f);
	//glColor4f(1.00f, 0.761f, 0.00f, 1.00f);
	//glVertex3f(App->camera->ray.a.x, App->camera->ray.a.y, App->camera->ray.a.z); glVertex3f(App->camera->ray.b.x, App->camera->ray.b.y, App->camera->ray.b.z);
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//glEnd();

	App->scene->scene_buff->UnBind("Scene");

	//if (game_camera != nullptr)
	//{
	//	App->scene->sceneBuff->Bind("Game");
	//	UpdateProjection(game_camera);
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	glLoadIdentity();
	//	glMatrixMode(GL_MODELVIEW);
	//	glLoadMatrixf(game_camera->GetViewMatrix());

	//	// Render Lights
	//	for (uint i = 0; i < MAX_LIGHTS; ++i)
	//		lights[i].Render();

	//	// Draw Plane
	//	App->scene->DrawPlane();

	//	// Draw GameObjects
	//	for (uint i = 0; i < App->scene->gameobjects.size(); i++)
	//	{
	//		App->scene->gameobjects[i]->Draw();
	//	}

	//	App->scene->gameBuff->UnBind("Game");

	//	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	//glLoadIdentity();
	//}

	
	ImGui::Render();

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_img, height_img,
		0, GL_RGBA, GL_UNSIGNED_BYTE, id_pixels);
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

