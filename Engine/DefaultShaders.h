#ifndef _DEFAULT_SHADERS_
#define _DEFAULT_SHADERS_



#include "GL3W\include\glew.h"

#pragma comment (lib, "GL3W/libx86/glew32.lib") 
typedef unsigned int uint;

static const GLchar* vertexShaderSource[] =
{
"	#version 330 core												  \n"
"	layout(location = 0) in vec3 position;							  \n"
"layout(location = 1) in vec2 texCoord;								  \n"
"layout(location = 2) in vec3 normal;								  \n"
"layout(location = 5) in vec3 tangent;								  \n"
"layout(location = 6) in vec3 bitangent;							  \n"
"																	  \n"
"out float ourTime;													  \n"
"out vec2 TexCoord;													  \n"
"out vec3 ourPos;													  \n"
"out vec3 world_pos;												  \n"
"out vec3 world_normal;												  \n"
"out mat3 TBN;														  \n"
"out vec3 FragPos;													  \n"
"out vec3 ourNormal;												  \n"
"																	  \n"
"//Outputs for shadow-mapping										  \n"
"out vec4 shadowCoord;												  \n"
"																	  \n"
"uniform float _time;												  \n"
"uniform vec4 _color;												  \n"
"uniform mat4 model;												  \n"
"uniform mat4 viewproj;												  \n"
"uniform mat4 view;													  \n"
"uniform mat4 modelview;											  \n"
"																	  \n"
"// Uniform for shadow-mapping										  \n"
"uniform mat4 depthBias;											  \n"
"																	  \n"
"																	  \n"
"																	  \n"
"void main()														  \n"
"{																	  \n"
"	TexCoord = texCoord;											  \n"
"	ourPos = position;												  \n"
"	world_pos = (model  * vec4(position, 1.0)).xyz;					  \n"
"	world_normal = normalize(mat3(model) * normal);					  \n"
"																	  \n""	ourNormal = normalize(normal);									  \n"
"	shadowCoord = depthBias * model * vec4(position, 1.0);			  \n"
"																	  \n"
"	vec3 T = normalize(vec3(model * vec4(tangent, 0)));				  \n"
"	vec3 B = normalize(vec3(model * vec4(bitangent, 0)));			  \n"
"	vec3 N = normalize(vec3(model * vec4(normal, 0)));				  \n"
"	TBN = transpose(mat3(T,B,N));									  \n"
"	FragPos = TBN * vec3(model * vec4(position,1));					  \n"
"																	  \n"
"	gl_Position = viewproj * model * vec4(position, 1.0f);			  \n"
"}																	  \n"

};

static const GLchar* fragmentShaderSource[] =
{
"	#version 330 core																																			 \n"
"																																								 \n"
"#define MAX_LIGHTS 100																																			 \n"
"																																								 \n"
"	uniform int _numLights;																																		 \n"
"uniform struct Light {																																			 \n"
"	vec3 position;																																				 \n"
"	int type;																																					 \n"
"	vec4 l_color; //a.k.a the color of the light																												 \n"
"	vec4 properties;																																			 \n"
"	float ambientCoefficient;																																	 \n"
"	float radius;																																				 \n"
"																																								 \n"
"																																								 \n"
"} _lights[MAX_LIGHTS];																																			 \n"
"																																								 \n"
"																																								 \n"
"in vec2 TexCoord;																																				 \n"
"																																								 \n"
"in vec3 ourPos;																																				 \n"
"in mat3 TBN;																																					 \n"
"in vec3 FragPos;																																				 \n"
"in vec3 ourNormal;																																				 \n"
"in vec4 shadowCoord;																																			 \n"
"																																								 \n"
"uniform vec4 diff_color;																																		 \n"
"out vec4 color;																																				 \n"
"uniform sampler2D albedo;																																		 \n"
"uniform sampler2D normal_map;																																	 \n"
"																																								 \n"
"uniform sampler2D specular_map;																																 \n"
"uniform sampler2D glossines_map;																																 \n"
"																																								 \n"
"uniform sampler2DShadow _shadowMap;																															 \n"
"																																								 \n"
"uniform vec3 _cameraPosition;																																	 \n"
"uniform float _alpha;																																			 \n"
"																																								 \n"
"																																								 \n"
"uniform float a_Kd;																																			 \n"
"uniform float a_shininess;																																		 \n"
"uniform float _farPlane;																																		 \n"
"																																								 \n"
"uniform int shadow_blur;																																		 \n"
"																																								 \n"
"//Fresnel																																						 \n"
"uniform bool activate_fresnel;																																	 \n"
"uniform float fresnel_scale;																																	 \n"
"uniform float fresnel_bias;																																	 \n"
"uniform float fresnel_power;																																	 \n"
"uniform float fresnel_lerp;																																	 \n"
"in vec3 world_pos;																																				 \n"
"in vec3 world_normal;																																			 \n"
"																																								 \n"
"uniform mat4 model;																																			 \n"
"																																								 \n"
"vec2 poissonDisk[16] = vec2[](																																	 \n"
"	vec2(-0.94201624, -0.39906216),																																 \n"
"	vec2(0.94558609, -0.76890725),																																 \n"
"	vec2(-0.094184101, -0.92938870),																															 \n"
"	vec2(0.34495938, 0.29387760),																																 \n"
"	vec2(-0.91588581, 0.45771432),																																 \n"
"	vec2(-0.81544232, -0.87912464),																																 \n"
"	vec2(-0.38277543, 0.27676845),																																 \n"
"	vec2(0.97484398, 0.75648379),																																 \n"
"	vec2(0.44323325, -0.97511554),																																 \n"
"	vec2(0.53742981, -0.47373420),																																 \n"
"	vec2(-0.26496911, -0.41893023),																																 \n"
"	vec2(0.79197514, 0.19090188),																																 \n"
"	vec2(-0.24188840, 0.99706507),																																 \n"
"	vec2(-0.81409955, 0.91437590),																																 \n"
"	vec2(0.19984126, 0.78641367),																																 \n"
"	vec2(0.14383161, -0.14100790)																																 \n"
"	);																																							 \n"
"																																								 \n"
"																																								 \n"
"																																								 \n"
"vec4 light_colors[MAX_LIGHTS];																																	 \n"
"																																								 \n"
"vec3 blinnPhongDir(Light light, float Kd, float Ks, float shininess, vec3 N, float shadow)																		 \n"
"{																																								 \n"
"	vec3 v = normalize(TBN * _cameraPosition - FragPos);																										 \n"
"																																								 \n"
"	float lightInt = light.properties[0];																														 \n"
"																																								 \n"
"	if (light.type != 0) {																																		 \n"
"																																								 \n"
"																																								 \n"
"		vec3 s = normalize(TBN * light.position);																												 \n"
"																																								 \n"
"		vec3 r = reflect(-s,N);																																	 \n"
"																																								 \n"
"		float cosTheta = clamp(dot(s, N), 0,1);																													 \n"
"		float cosAlpha = clamp(dot(v,r), 0,1);																													 \n"
"																																								 \n"
"																																								 \n"
"		float diffuse = Kd * lightInt * cosTheta;																												 \n"
"		float spec = Ks * lightInt* pow(cosAlpha,shininess);																									 \n"
"																																								 \n"
"		return vec3(diffuse,spec,1) * shadow;																													 \n"
"																																								 \n"
"	}																																							 \n"
"																																								 \n"
"	else {																																						 \n"
"		// Point																																				 \n"
"		vec3 lightpos = TBN * light.position;																													 \n"
"		vec3 s = normalize(lightpos - FragPos);      // Light direction																							 \n"
"		vec3 r = reflect(-s,N);																																	 \n"
"																																								 \n"
"		float cosTheta = clamp(dot(s,N), 0,1);																													 \n"
"		float cosAlpha = clamp(dot(v,r), 0,1);																													 \n"
"																																								 \n"
"		float d = length((lightpos - FragPos));																													 \n"
"		float attenuation = 1 / (light.properties[1] + light.properties[2] * d + light.properties[3] * d*d);													 \n"
"		attenuation *= lightInt;																																 \n"
"		float diffuse = attenuation * Kd  * cosTheta;																											 \n"
"		float spec = attenuation * Ks * pow(cosAlpha,shininess);																								 \n"
"																																								 \n"
"		return vec3(diffuse,spec,attenuation);																													 \n"
"																																								 \n"
"																																								 \n"
"	}																																							 \n"
"																																								 \n"
"																																								 \n"
"}																																								 \n"
"																																								 \n"
"float CalcShadow(vec4 shadowPos, float usedBias)																												 \n"
"{																																								 \n"
"	float shadow = 1.0;																																			 \n"
"																																								 \n"
"	if (shadowPos.z > 1.0)																																		 \n"
"		return 0.0;																																				 \n"
"																																								 \n"
"	int iterations = 10;																																		 \n"
"																																								 \n"
"	for (int i = 0; i < iterations; ++i)																														 \n"
"	{																																							 \n"
"																																								 \n"
"		float shadowVal = (1.0f - texture(_shadowMap, vec3(shadowPos.xy + poissonDisk[i] / 200.0, (shadowPos.z - usedBias) / shadowPos.w)));					 \n"
"		float tmp = 0.1* shadowVal;																																 \n""		// old value: 0.05 - iterations: 20																														 \n"
"																																								 \n"
"		shadow -= tmp;																																			 \n"
"	}																																							 \n"
"																																								 \n"
"	return shadow;																																				 \n"
"}																																								 \n"
"																																								 \n"
"																																								 \n"
"void main()																																					 \n"
"{																																								 \n"
"	vec3 color_texture = texture(albedo, TexCoord).xyz;																											 \n"
"	vec3 N = normalize(texture(normal_map,TexCoord).xyz * 2 - 1);																								 \n"
"	vec3 spec_texture = exp2(texture(specular_map, TexCoord).xyz) + 0.5;																						 \n"
"	vec3 gloss_texture = exp2(10 * abs(texture(glossines_map,TexCoord).xyz - vec3(1)) + 1) * 128;																 \n"
"																																								 \n"
"	vec3 inten = vec3(0); vec3 inten_final = vec3(0);																											 \n"
"																																								 \n"
"	float final_ambient = 0.0;																																	 \n"
"	vec3 final_color = vec3(0);																																	 \n"
"																																								 \n"
"	vec3 lightDir = vec3(0, 0, 0);																																 \n"
"	// Iterate all lights to search the first directional light for now																							 \n"
"	for (int i = 0; i < _numLights; ++i)																														 \n"
"	{																																							 \n"
"		if (_lights[i].type != 0)																																 \n"
"		{																																						 \n"
"			//Directional																																		 \n"
"			lightDir = _lights[i].position;																														 \n"
"			break;																																				 \n"
"		}																																						 \n"
"	}																																							 \n"
"																																								 \n"
"	vec3 l = normalize(lightDir);																																 \n"
"	float cosTheta = clamp(dot(ourNormal,l),0,1);																												 \n"
"	float bias = 0.00;																																			 \n"
"	float usedBias = bias * tan(acos(cosTheta));																												 \n"
"	usedBias = clamp(usedBias, 0, 0.01);																														 \n"
"																																								 \n"
"																																								 \n""	vec4 shadowPos = shadowCoord / shadowCoord.w;																												 \n"
"	float shadow = CalcShadow(shadowPos, usedBias);																												 \n"
"																																								 \n"
"	for (int i = 0; i <_numLights; ++i) {																														 \n"
"																																								 \n"
"		inten = blinnPhongDir(_lights[i], a_Kd, spec_texture.r, gloss_texture.r, N, shadow);																	 \n"
"		inten_final += inten;																																	 \n"
"		light_colors[i] = vec4(_lights[i].l_color.rgb,inten.z);																									 \n"
"																																								 \n"
"		final_color += vec3(light_colors[i]) * light_colors[i].a;																								 \n"
"		final_ambient += _lights[i].ambientCoefficient;																											 \n"
"																																								 \n"
"																																								 \n"
"	}																																							 \n"
"																																								 \n"
"	final_ambient = final_ambient / _numLights;																													 \n"
"	final_color = normalize(final_color);																														 \n"
"																																								 \n"
"	vec3 col = max(color_texture * vec3(0,0.2,0.2), color_texture * (inten_final.x + inten_final.y * spec_texture.r)*final_color.rgb);							 \n"
"																																								 \n"
"	float fade_dist = 1;																																		 \n"
"	float norm_min = (_farPlane - _farPlane / 3);																												 \n"
"	float dist = (length(_cameraPosition - vec3(model * vec4(ourPos,1))) - norm_min) / (_farPlane - norm_min);													 \n"
"	dist = abs(clamp(dist, 0.0, 1.0) - 1);																														 \n"
"	color = vec4(col* dist, _alpha);																															 \n"
"																																								 \n"
"	color = vec4(col* dist, _alpha);																															 \n"
"																																								 \n"
"																																								 \n"
"	if (activate_fresnel == true)																																 \n"
"	{																																							 \n"
"																																								 \n"
"		vec3 camera_to_vertex = normalize(_cameraPosition - world_pos);																							 \n"
"		float fresnel_coeficient = max(0.0, min(1.0, fresnel_bias + fresnel_scale * pow(1.0 + dot(camera_to_vertex, world_normal), fresnel_power)));			 \n"
"																																								 \n"
"																																								 \n"
"		color = mix(color ,vec4(vec3(col.xyz * dist) * fresnel_coeficient, 1.0), fresnel_lerp);																	 \n"
"																																								 \n"
"																																								 \n"
"	}																																							 \n"
" }																																								 \n"
};																																								 

static const GLchar* DefaultVert[] =
{
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"layout(location = 2) in vec3 normal;\n"
	"layout(location = 3) in vec4 color;\n"
	"out float ourTime;\n"
	"out vec4 ourColor;\n"
	"out vec3 ourNormal;\n"
	"out vec2 TexCoord;\n"
	"uniform float _time;\n"
	"uniform vec4 _color;\n"
	"uniform mat4 model;\n"
	"uniform mat4 viewproj;\n"
	"uniform mat4 view;\n"
	"void main()\n"
	"{\n"
	"gl_Position = viewproj *  model * vec4(position.x,position.y,position.z, 1.0f);\n"
	"ourColor = _color;\n"
	"TexCoord = texCoord;\n"
	"ourTime = _time;\n"
	"ourNormal = mat3(model) * normal;"
	"}\n"

};

static const GLchar* TextureVert[] =
{
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"


	"out vec2 TexCoord;\n"

	"void main()\n"
	"{\n"
	"gl_Position = vec4(position, 1.0f);\n"
	"TexCoord = texCoord;\n"


	"}\n"
};

static const GLchar* BlurFrag[] =
{
	"#version 330 core\n"

	"in vec2 TexCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D albedo;\n"
	"uniform int _orientation;\n"
	"uniform int BlurAmount;\n"
	"uniform float BlurScale;\n"
	"uniform float BlurStrength;\n"
	"float Gaussian(float x, float deviation)\n"
	"{\n"
	"	return (1.0 / sqrt(2.0 * 3.141592 * deviation)) * exp(-((x * x) / (2.0 * deviation)));\n"
	"}\n"

	"void main()\n"
	"{\n"

	"float halfBlur = float(BlurAmount) * 0.5;\n"
	"vec4 colour = vec4(0.0);\n"
	"vec4 texColour = vec4(0.0);\n"


	"float deviation = halfBlur * 0.35;\n"
	"deviation *= deviation;\n"
	"float strength = 1.0 - BlurStrength;\n"

	"if(_orientation == 0){\n"
	"	// Horizontal blur\n"
	"	for (int i = 0; i < BlurAmount; ++i)\n"
	"	{\n"
	"		if (i >= BlurAmount)\n"
	"			break;\n"
	"\n"
	"		float offset = float(i) - halfBlur;\n"
	"		texColour = texture2D(albedo, TexCoord + vec2(BlurScale * offset/128 , 0)) * Gaussian(offset * strength, deviation);\n"
	"		colour += texColour;\n"
	"	}\n"
	"}\n"
	"else {\n"
	"	// Horizontal blur\n"
	"	for (int i = 0; i < BlurAmount; ++i)\n"
	"	{\n"
	"		if (i >= BlurAmount)\n"
	"			break;\n"
	"\n"
	"		float offset = float(i) - halfBlur;\n"
	"		texColour = texture2D(albedo, TexCoord + vec2(0,BlurScale * offset /128)) * Gaussian(offset * strength, deviation);\n"
	"		colour += texColour;\n"
	"	}\n"
	"}\n"

	"color= vec4(colour);\n"
	"}\n"
};

static const GLchar* FinalFrag[] =
{
	"#version 330 core\n"
	"in vec4 ourColor;\n"
	"in float ourTime;\n"
	"in vec2 TexCoord;\n"
	"in vec3 ourNormal;\n"
	"in vec4 gl_FragCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D _albedo;\n"
	"uniform sampler2D _glow_tex;\n"
	"uniform sampler2D _dmg_tex;\n"
	"uniform vec4 _my_color;\n"
	"uniform bool damage;\n"
	"uniform float alpha; \n"
	"uniform float mult_dead;\n"
	"void main()\n"
	"{\n"
	"vec4 dst = texture2D(_albedo, TexCoord);\n" // rendered scene
	"vec4 src = texture2D(_glow_tex, TexCoord); \n" // glowmap
	"vec4 dmg = texture(_dmg_tex,TexCoord);\n"
	"if(damage)\n"
	"color = mix(min(vec4(src.rgb * mult_dead,1) + vec4(dst.rgb * mult_dead,1), 1.0) , dmg, dmg.a * alpha );\n"
	"else\n"
	"color= min(vec4(src.rgb * mult_dead,1) + vec4(dst.rgb * mult_dead,1) , 1.0);\n"

	"}\n"
};

static const GLchar* DefaultFrag[] =
{
	"#version 330 core\n"
	"in vec4 ourColor;\n"
	"in float ourTime;\n"
	"in vec2 TexCoord;\n"
	"in vec3 ourNormal;\n"
	"in vec4 gl_FragCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D albedo;\n"
	"uniform vec4 _my_color;\n"
	"void main()\n"
	"{\n"

	//Z-Buffer Line Shader
	"color= _my_color * texture(albedo, TexCoord);\n"
	"}\n"
};

static const GLchar* NonGlowFrag[] =
{
	"#version 330 core\n"
	"in vec4 ourColor;\n"
	"in float ourTime;\n"
	"in vec2 TexCoord;\n"
	"in vec3 ourNormal;\n"
	"in vec4 gl_FragCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D albedo;\n"
	"uniform vec4 _my_color;\n"
	"void main()\n"
	"{\n"

	//Z-Buffer Line Shader
	"color= vec4(vec3(0),1);\n"
	"}\n"
};


static const GLchar* ShadowMapVert[] =
{
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"layout(location = 2) in vec3 normal;\n"
	"layout(location = 3) in vec4 color;\n"

	"uniform mat4 depthMVP;\n"
	"uniform mat4 model;\n"

	"void main()\n"
	"{\n"
	" gl_Position =  depthMVP *model * vec4(position,1);\n"
	"}\n"
};


static const GLchar* ShadowMapFrag[] =
{
	"#version 330 core\n"
	"layout(location = 0) out vec4 fragmentdepth;\n"

	"void main()\n"
	"{\n"
	"fragmentdepth = vec4(gl_FragCoord.z);\n"
	"}\n"
};
static const GLchar* CubeMapVert[] =
{
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"layout(location = 2) in vec3 normal;\n"
	"layout(location = 3) in vec4 color;\n"

	"uniform mat4 viewproj;\n"
	"uniform mat4 model;\n"

	"void main()\n"
	"{\n"
	" gl_Position =  viewproj *model * vec4(position,1);\n"
	"}\n"
};


static const GLchar* CubeMapFrag[] =
{
	"#version 330 core\n"
	"out vec4 color;\n"

	"void main()\n"
	"{\n"
	"color = vec4(1);\n"
	"}\n"
};


static const  GLchar* UIShaderVert[] =
{
	"#version 330\n"
	"uniform mat4 ProjMtx;\n"
	"uniform mat4 model;\n"
	"uniform vec4 Color_UI_ME;\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"out vec2 Frag_UV;\n"

	"void main()\n"
	"{\n"
	"	Frag_UV = texCoord;\n"
	"	gl_Position = ProjMtx * model * vec4(position.xy,0,1);\n"
	"}\n"
};


static const GLchar* UIShaderFrag[] =
{
	"#version 330\n"
	"uniform sampler2D Texture;\n"
	"uniform vec4 Color_UI_ME;\n"

	"in vec2 Frag_UV;\n"
	"in vec4 Frag_Color;\n"
	"out vec4 Out_Color;\n"
	"void main()\n"
	"{\n"
	"	Out_Color = Color_UI_ME*texture(Texture,vec2(Frag_UV.s, 1.0 - Frag_UV.t).st);\n"
	"}\n"
};

static const GLchar* PointShadowMapVert[] =
{
	"#version 330 core\n"
	"layout (location = 0) in vec3 position;\n"
	"uniform mat4 model;\n"
	"uniform mat4 viewproj;\n"
	"out vec4 FragPos;\n"
	"void main()\n"
	"{\n"
	"	FragPos = model * vec4(position, 1.0);\n"
	"	gl_Position = viewproj * model * vec4(position, 1.0);\n"
	"}\n"
};

static const GLchar* PointShadowMapGeo[] =
{
	"#version 330 core\n"
	"layout (triangles) in;\n"
	"layout (triangle_strip, max_vertices=18) out;\n"
	"uniform mat4 shadow_matrices[6];\n"
	"out vec4 FragPos;\n"
	"void main()\n"
	"{\n"
	"	for(int face = 0; face < 6; ++face)\n"
	"	{\n"
	"		gl_Layer = face;\n"
	"		for(int i = 0; i < 3; ++i)\n"
	"		{\n"
	"			FragPos = gl_in[i].gl_Position;\n"
	"			gl_Position = shadow_matrices[face] * FragPos;\n"
	"			EmitVertex();\n"
	"		}\n"
	"		EndPrimitive();\n"
	"	}\n"
	"}\n"
};

static const GLchar* PointShadowMapFrag[] =
{
	"#version 330 core\n"
	"in vec4 FragPos;\n"
	
	"uniform vec3 _light_pos;\n"
	"uniform float _far_plane;\n"
	"void main()\n"
	"{\n"
	"	float light_dist = length(FragPos.xyz - _light_pos);\n"
	"	light_dist = (light_dist-1) / (_far_plane-1); // Normalize it\n"
	"	gl_FragDepth = light_dist;\n"
	"}\n"
};

static const GLchar* SkinningVert[] =
{
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"layout(location = 2) in vec3 normal;\n"
	"layout(location = 3) in vec4 influences;\n"
	"layout(location = 4) in vec4 bones;\n"
	"layout(location = 5) in vec3 tangent;\n"
	"layout(location = 6) in vec3 bitangent;\n"

	"out float ourTime;		\n"
	"out vec4 ourColor;		\n"
	"out vec3 ourNormal;	\n"
	"out vec2 TexCoord;		\n"
	"out vec3 ourPos;		\n"
	"out mat3 TBN;			\n"
	"out vec3 FragPos;		\n"
	"out vec4 shadowCoord;	\n"
	"uniform float _time;	\n"
	"uniform vec4 _color;	\n"
	"uniform mat4 model;	\n"
	"uniform mat4 viewproj;	\n"
	"uniform mat4 view;		\n"
	"uniform mat4 depthBias;\n"

	"uniform samplerBuffer _skinning_text;\n"
	"uniform int _num_pixels;			  \n"

	"void main()										\n"
	"{													\n"
	"	vec3 skinned_pos = vec3(0.0f, 0.0f, 0.0f);		\n"
	"	vec3 skinned_normal = vec3(0.0f, 0.0f, 0.0f);	\n"
	"	vec3 skinned_tangent = vec3(0.0f, 0.0f, 0.0f);	\n"
	"	vec3 skinned_bitangent = vec3(0.0f, 0.0f, 0.0f);\n"
	"	float total_weight = 0.0f;						\n"

	"	bool test = false;	\n"

	"	for (int i = 0; i < 4; i++)									\n"
	"	{															\n"
	"		int bone_id = int(bones[i]);							\n"

	"		if (bone_id == -1)										\n"
	"			break;												\n"

	"		int start_buffer_pos = bone_id * 4 * 4;					\n"

	"		mat4 skinning_mat = mat4(								\n"
	"			//Column 0											\n"
	"			texelFetch(_skinning_text, start_buffer_pos).r,		\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 1).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 2).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 3).r,	\n"
	"			//Column 1											\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 4).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 5).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 6).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 7).r,	\n"
	"			//Column 2											\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 8).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 9).r,	\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 10).r,\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 11).r,\n"
	"			//Column 3											\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 12).r,\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 13).r,\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 14).r,\n"
	"			texelFetch(_skinning_text, start_buffer_pos + 15).r	\n"
	"		);														\n"

	"		vec4 moved_pos = skinning_mat * vec4(position, 1.0f);		\n"
	"		vec3 normalized_pos = moved_pos.xyz / moved_pos.w;			\n"
	"		skinned_pos = normalized_pos * influences[i] + skinned_pos;	\n"

	"		vec4 moved_normals = skinning_mat * vec4(normal, 0.0f);				\n"
	"		skinned_normal = moved_normals.xyz * influences[i] + skinned_normal;\n"

	"		total_weight += influences[i];	\n"

	"		if (total_weight >= 1.0f)		\n"
	"			break;						\n"
	"	}									\n"


	"	vec3 T = normalize(vec3(model * vec4(tangent, 0)));			\n"
	"	vec3 B = normalize(vec3(model * vec4(bitangent, 0)));		\n"
	"	vec3 N = normalize(vec3(model * vec4(normal, 0)));			\n"
	"	TBN = transpose(mat3(T,B,N));								\n"
	"	FragPos = TBN * vec3(model * vec4(skinned_pos,1));			\n"
	"	shadowCoord = depthBias * model * vec4(skinned_pos, 1.0);	\n"

	"	gl_Position = viewproj * model * vec4(skinned_pos, 1.0f);	\n"
	"	ourColor = _color;											\n"
	"	TexCoord = texCoord;										\n"
	"	ourTime = _time;											\n"
	"	ourNormal = mat3(model) * normalize(skinned_normal);		\n"
	"	ourPos = position;											\n"
	"}																\n"
};


#endif 