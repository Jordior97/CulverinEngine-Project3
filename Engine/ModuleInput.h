#ifndef _MODULEINPUT_
#define _MODULEINPUT_

#include "Module.h"
#include "Globals.h"
#include "parson.h"

#define MAX_MOUSE_BUTTONS 5
#define MAX_CONTROLLER_BUTTONS 15
class InputManager;
class PlayerActions;
enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

/*struct Controller
{
	KEY_STATE				controller_buttons[MAX_CONTROLLER_BUTTONS];
	uint					right_joystick_dir[4];
	uint					left_joystick_dir[4];
	_SDL_GameController*	pad = nullptr;
};*/

class ModuleInput : public Module
{
public:
	ModuleInput(bool start_enabled = true);
	~ModuleInput();

	bool Init(JSON_Object* node);
	//bool Start();
	update_status PreUpdate(float dt);
	//update_status Update(float dt);
	//update_status PostUpdate(float dt);
	update_status UpdateConfig(float dt);
	bool CleanUp();

	void UIInputManagerUpdate();

	int GetMouseXMotionNormalized();
	int GetMouseYMotionNormalized();

	SDL_Scancode GetKeyFromName(const char* name);

	KEY_STATE GetKey(int id) const
	{
		if (id < 300 && id >= 0)
		{
			return keyboard[id];
		}
	}

	KEY_STATE GetMouseButton(int id) const
	{
		return mouse_buttons[id];
	}

	int GetMouseX() const
	{
		return mouse_x;
	}

	int GetMouseY() const
	{
		return mouse_y;
	}

	int GetMouseZ() const
	{
		return mouse_z;
	}

	int GetMouseXMotion() const
	{
		return mouse_x_motion;
	}

	int GetMouseYMotion() const
	{
		return mouse_y_motion;
	}

	int GetMouseXGlobal() const
	{
		return mouse_x_global;
	}

	int GetMouseYGlobal() const
	{
		return mouse_y_global;
	}

	int GetMouseXMotionGlobal() const
	{
		return mouse_x_motion_global;
	}

	int GetMouseYMotionGlobal() const
	{
		return mouse_y_motion_global;
	}

	void SetMotionGlobal(int x, int y)
	{
		mouse_x_global = x;
		mouse_y_global = y;
	}

private:
	KEY_STATE* keyboard;
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x = 0;
	int mouse_y = 0;
	int mouse_z = 0;
	int mouse_x_motion = 0;
	int mouse_y_motion = 0;

	bool chek = false;

	int mouse_x_global = 0;
	int mouse_y_global = 0;
	int mouse_x_motion_global = 0;
	int mouse_y_motion_global = 0;

	std::string ui_input_manager;
	std::string submit;
	std::string cancel;
	std::string vertical;
	std::string horizontal;
	bool ui_conected = false;

public:
	std::list<const char*> dropedfiles;
	_SDL_GameController* controller = nullptr;
	PlayerActions* player_action = nullptr;
	InputManager* ui_manager = nullptr;
};

#endif