#ifndef INPUT_ACTION_H_
#define INPUT_ACTION_H_
#include<string>
#include"Math\float2.h"
#include"ModuleKeyBinding.h"

enum Keystateaction
{
	KEY_IDLE_ACTION = 0,
	KEY_DOWN_ACTION,
	KEY_REPEAT_ACTION,
	KEY_UP_ACTION
};

enum ActionInputType
{
	UNKNOWN_ACTION=-1,
	AXIS_ACTION,
	CONTROLLER_AXIS_ACTION,
	KEY_ACTION,
	MOUSE_BUTTON_ACTION,
	BUTTON_ACTION


};

enum AxisDirectionController
{
	NON_DIRECTION_C=-1,
	UP_DIRECTION_C,
	DOWN_DIRECTION_C,
	RIGHT_DIRECTION_C,
	LEFT_DIRECTION_C
};

class InputAction
{
public:

	InputAction();
	InputAction(const char* name, KeyRelation* key_relation, ActionInputType action_type = ActionInputType::UNKNOWN_ACTION);
	~InputAction();

public:

	std::string name = "";
	ActionInputType action_type = ActionInputType::UNKNOWN_ACTION;
	KeyRelation *key_relation=nullptr;

};


class AxisAction: public InputAction
{
public:
	AxisAction(){ action_type = ActionInputType::AXIS_ACTION; }
	float2 position;
	float2 motion;

};

class ControllerAxisAction : public InputAction
{
public:
	ControllerAxisAction() { action_type = ActionInputType::CONTROLLER_AXIS_ACTION; }
	float2 motion;
	AxisDirectionController direction = AxisDirectionController::NON_DIRECTION_C;

};

class MouseButtonAction : public InputAction
{
public:
	MouseButtonAction() { action_type = ActionInputType::MOUSE_BUTTON_ACTION; }
	Keystateaction state = Keystateaction::KEY_IDLE_ACTION;
	int number_clicks = 0;
};

class KeyAction : public InputAction
{
public:
	KeyAction() { action_type = ActionInputType::KEY_ACTION; }
	Keystateaction state = Keystateaction::KEY_IDLE_ACTION;
	bool OnClick() { return state == Keystateaction::KEY_DOWN_ACTION; };
	bool OnRelease() { return state == Keystateaction::KEY_UP_ACTION; };
	bool OnRepeat() { return state == Keystateaction::KEY_REPEAT_ACTION; };

};

class ButtonAction : public InputAction
{
public:
	ButtonAction() { action_type = ActionInputType::BUTTON_ACTION; }
	Keystateaction state = Keystateaction::KEY_IDLE_ACTION;
	bool OnClick() { return state == Keystateaction::KEY_DOWN_ACTION; };
	bool OnRelease() { return state == Keystateaction::KEY_UP_ACTION; };
	bool OnRepeat() { return state == Keystateaction::KEY_REPEAT_ACTION; };

};





#endif
