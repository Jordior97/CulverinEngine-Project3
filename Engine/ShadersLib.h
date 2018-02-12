// MathLibrary.h - Contains declaration of Function class  
#pragma once  

#include "MathGeoLib.h"


#include <string>
#include <list>
#include <vector>
#include "GL3W\include\glew.h"
#include "Resource_.h"
#include "ResourceMaterial.h"
#include "TextEditor.h"

#pragma comment (lib, "GL3W/libx86/glew32.lib") 
typedef unsigned int uint;

enum ShaderType {
	nullShader,
	fragment,
	geometry,
	vertex,
};

struct UniformVar {

	char name[64] = "0";
	GLenum type = GL_ZERO;

};
struct TextureVar {
	std::string var_name;
	bool selected = false;
	ResourceMaterial* res_material = nullptr;
	uint uuid_to_reimport = 0;
};

struct float3Var {
	std::string var_name;
	float3 vector = float3::zero;
};

struct ColorVar {
	std::string var_name;
	float4 color = float4::zero;
};

struct floatVar {
	std::string var_name;
	float value = 0;
};

struct intVar {
	std::string var_name;
	int value = 0;
};

struct boolVar {
	std::string var_name;
	bool value = false;
};

class Shader {
public:
	Shader() {

	}
	~Shader() {

	}

	char* GetShaderText();
public:
	uint shaderID = 0;

	std::string name = "";

	std::string shaderPath = "";

	std::string shaderText = "";

	ShaderType shaderType = ShaderType::nullShader;

};


class ShaderProgram {
public:
	ShaderProgram() {

	}
	~ShaderProgram() {

	}

	void AddFragment(Shader* fragment);
	void AddVertex(Shader* vertex);
	void  AddGeometry(Shader* geometry);

	bool  LoadProgram();
	bool UpdateShaderProgram(uint fragmentID, uint vertexID, uint geometryID);

	bool  Bind();

	void  Unbind();

	UniformVar  GetVariableInfo(uint index);

	GLint  GetVariablesSize()const;

	void CreateMaterialFile();

public:
	std::string name;
	std::string path;
	uint programID = 0;

	Shader* fragment = nullptr;
	Shader* vertex = nullptr;
	Shader* geometry = nullptr;

	//Variables Vector
	std::vector<TextureVar> textures;
	std::vector<intVar> int_variables;
	std::vector<floatVar> float_variables;
	std::vector<float3Var> float3_variables;
	std::vector<ColorVar> float4_variables;
	std::vector<boolVar> bool_variables;

	TextEditor shader_editor;

	bool get_shader_text = false;

	bool edit_fragment = false;
	bool edit_vertex = false;
	bool edit_geometry = false;
};


