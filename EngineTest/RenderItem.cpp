#include <filesystem>
#include "CommonHeaders.h"
#include "Content/ContentToEngine.h"
#include "ShaderCompilation.h"

using namespace havana;

bool ReadFile(std::filesystem::path, std::unique_ptr<u8[]>&, u64&);

namespace
{
	id::id_type modelId{ id::invalid_id };
	id::id_type vsId{ id::invalid_id };
	id::id_type psId{ id::invalid_id };

	void LoadModel()
	{
		std::unique_ptr<u8[]> model;
		u64 size{ 0 };
		ReadFile("..\\..\\enginetest\\model.model", model, size);

		modelId = content::CreateResource(model.get(), content::AssetType::Mesh);
		assert(id::is_valid(modelId));
	}

	void LoadShaders()
	{
		// Let's say our material uses a vertex shader and a pixel shader

	}
} // anonymous namespace

id::id_type CreateRenderItem(id::id_type entityId)
{
	// load a model, pretend it belongs to entityId
	auto _1 = std::thread{ [] { LoadModel(); } };
	// load material:
	// 1) load textures
	// 2) load shaders for that material
	auto _2 = std::thread{ [] { LoadShaders(); } };

	_1.join();
	_2.join();
	// add a render item using the model and its materials
}

void DestroyRenderItem(id::id_type itemId)
{

}