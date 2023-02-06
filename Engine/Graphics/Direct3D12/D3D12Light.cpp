#include "D3D12Light.h"
#include "Shaders/SharedTypes.h"
#include "EngineAPI/GameEntity.h"

namespace havana::graphics::d3d12::light
{
	namespace
	{
		struct light_owner
		{
			game_entity::entity_id	entity_id{ id::invalid_id };
			u32						data_index;
			graphics::light::type	type;
			bool					is_enabled;
		};

		class light_set
		{
		public:
			constexpr graphics::light add(const light_init_info& info)
			{
				if (info.type == graphics::light::directional)
				{
					u32 index{ u32_invalid_id };
					// Find an available slot in the array if any
					for (u32 i{ 0 }; i < _non_cullable_owners.size(); ++i)
					{
						if (!id::is_valid(_non_cullable_owners[i]))
						{
							index = i;
							break;
						}
					}

					if (index == u32_invalid_id)
					{
						index = (u32)_non_cullable_owners.size();
						_non_cullable_owners.emplace_back();
						_non_cullable_lights.emplace_back();
					}

					hlsl::DirectionalLightParameters& params{ _non_cullable_lights[index] };
					params.Color = info.color;
					params.Intensity = info.intensity;

					light_owner owner{ game_entity::entity_id{info.entity_id}, index, info.type, info.is_enabled };
					const light_id id{ _owners.add(owner) };
					_non_cullable_owners[index] = id;
					
					return graphics::light{ id, info.light_set_key };
				}
				else
				{
					// TODO: cullable lights
				}
			}

			constexpr void remove(light_id id)
			{
				enable(id, false);

				const light_owner& owner{ _owners[id] };

				if (owner.type == graphics::light::directional)
				{
					_non_cullable_owners[owner.data_index] = light_id{ id::invalid_id };
				}
				else
				{
					// TODO: cullable lights
				}

				_owners.remove(id);
			}

			constexpr void enable(light_id id, bool is_enabled)
			{
				_owners[id].is_enabled = is_enabled;

				if (_owners[id].type == graphics::light::directional) return;

				// TODO: cullable lights
			}

		private:
			utl::free_list<light_owner>						_owners;
			utl::vector<hlsl::DirectionalLightParameters>	_non_cullable_lights;
			utl::vector<light_id>							_non_cullable_owners;
		};

		std::unordered_map<u64, light_set>	light_sets;
	} // anonimous namespace

	graphics::light
	create(light_init_info info)
	{
		return {};
	}

	void
	remove(light_id id, u64 light_set_key)
	{

	}

	void
	set_paramter(light_id id, u64 light_set_key, light_parameter::parameter parameter, const void* const data, u32 data_size)
	{

	}

	void
	get_paramter(light_id id, u64 light_set_key, light_parameter::parameter parameter, void* const data, u32 data_size)
	{

	}
}