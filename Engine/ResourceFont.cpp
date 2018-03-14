#include "ResourceFont.h"
#include "SDL2_ttf/include/SDL_ttf.h"



ResourceFont::ResourceFont(uint uuid):Resource(uuid, Resource::Type::FONT, Resource::State::UNLOADED)
{
	num_game_objects_use_me = 0;
	LOG("Resource Font Created!");
}


ResourceFont::~ResourceFont()
{
}

void ResourceFont::InitInfo(const char * nameResource, const char * path)
{
	name = nameResource;
	path_assets = path;
}

void ResourceFont::Init(Font copy_font)
{
	font.size = copy_font.size;
	font.font = copy_font.font;

}

void ResourceFont::DeleteToMemory()
{
	state = Resource::State::UNLOADED;
	TTF_CloseFont(font.font);
	LOG("UnLoaded Resource Material");
}

bool ResourceFont::LoadToMemory()
{
	state = Resource::State::LOADED;
	LOG("Loaded Resource Material");
	return true;
}

Resource::State ResourceFont::IsLoadedToMemory()
{
	return state;
}
