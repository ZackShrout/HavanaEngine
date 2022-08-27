#include "Geometry.h"
#include "Utilities/IOStream.h"

namespace havana::tools
{
	namespace
	{
		using namespace math;
		using namespace DirectX;

		void
		recalculate_normals(mesh& m)
		{
			const u32 num_indices{ (u32)m.raw_indices.size() };
			m.normals.resize(num_indices);

			for (u32 i{ 0 }; i < num_indices; ++i)
			{
				const u32 i0{ m.raw_indices[i] };
				const u32 i1{ m.raw_indices[++i] };
				const u32 i2{ m.raw_indices[++i] };

				XMVECTOR v0{ XMLoadFloat3(&m.positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&m.positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&m.positions[i2]) };

				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };
				XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0, e1)) };

				XMStoreFloat3(&m.normals[i], n);
				m.normals[i - 1] = m.normals[i];
				m.normals[i - 2] = m.normals[i];
			}
		}

		void
		process_normals(mesh& m, f32 smoothing_angle)
		{
			const f32 cos_alpha{ XMScalarCos(pi - smoothing_angle * pi / 180.0f) };
			const bool is_hard_edge{ XMScalarNearEqual(smoothing_angle, 180.0f, epsilon) };
			const bool is_soft_edge{ XMScalarNearEqual(smoothing_angle, 0.0f, epsilon) };
			const u32 num_indices{ (u32)m.raw_indices.size() };
			const u32 num_vertices{ (u32)m.positions.size() };

			assert(num_indices && num_vertices);

			m.indices.resize(num_indices);
			utl::vector<utl::vector<u32>> idx_ref(num_vertices);
			
			for (u32 i{ 0 }; i < num_indices; ++i)
			{
				idx_ref[m.raw_indices[i]].emplace_back(i);
			}

			for (u32 i{ 0 }; i < num_vertices; ++i)
			{
				auto& refs{ idx_ref[i] };
				u32 num_refs{ (u32)refs.size() };

				for (u32 j{ 0 }; j < num_refs; ++j)
				{
					m.indices[refs[j]] = (u32)m.vertices.size();
					vertex& v{ m.vertices.emplace_back() };
					v.position = m.positions[m.raw_indices[refs[j]]];

					// calculate where this is a hard edge or a soft edge
					XMVECTOR n1{ XMLoadFloat3(&m.normals[refs[j]]) };
					if (!is_hard_edge)
					{
						for (u32 k{ j + 1 }; k < num_refs; ++k)
						{
							// This value represents the cosine of the angle between the normals
							f32 cos_theta{ 0.0f };
							XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
							if (!is_soft_edge)
							{
								// NOTE: n2 is already normalized, so it's lenth is 1, so we don't divide
								// by it's length to get the cosine
								// cos(angle) = dot(n1, n2) / (||n1|| * ||n2||)
								XMStoreFloat(&cos_theta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (is_soft_edge || cos_theta >= cos_alpha)
							{
								n1 += n2;
								m.indices[refs[k]] = m.indices[refs[j]];
								refs.erase(refs.begin() + k);
								--num_refs;
								--k;
							}
						}
					}
					XMStoreFloat3(&v.normal, XMVector3Normalize(n1));
				}
			}
		}

		void
		process_uvs(mesh& m)
		{
			utl::vector<vertex> old_vertices;
			old_vertices.swap(m.vertices);
			utl::vector<u32> old_indices(m.indices.size());
			old_indices.swap(m.indices);
			const u32 num_indices{ (u32)old_indices.size() };
			const u32 num_vertices{ (u32)old_vertices.size() };

			assert(num_vertices && num_indices);

			utl::vector<utl::vector<u32>> idx_ref(num_vertices);

			for (u32 i{ 0 }; i < num_indices; ++i)
			{
				idx_ref[old_indices[i]].emplace_back(i);
			}

			for (u32 i{ 0 }; i < num_vertices; ++i)
			{
				auto& refs{ idx_ref[i] };
				u32 num_refs{ (u32)refs.size() };

				for (u32 j{ 0 }; j < num_refs; ++j)
				{
					m.indices[refs[j]] = (u32)m.vertices.size();
					vertex& v{ old_vertices[old_indices[refs[j]]] };
					v.uv = m.uv_sets[0][refs[j]];
					m.vertices.emplace_back(v);

					for (u32 k{ j + 1 }; k < num_refs; ++k)
					{
						v2 uv1{ m.uv_sets[0][refs[k]] };
						
						if (XMScalarNearEqual(v.uv.x, uv1.x, epsilon) &&
							XMScalarNearEqual(v.uv.y, uv1.y, epsilon))
						{
							m.indices[refs[k]] = m.indices[refs[j]];
							refs.erase(refs.begin() + k);
							--num_refs;
							--k;
						}
					}
				}
			}
		}

		u64
		get_vertex_element_size(elements::elements_type::type elements_type)
		{
			using namespace elements;

			switch (elements_type)
			{
			case elements_type::static_normal:
				return sizeof(static_normal);
			case elements_type::static_normal_texture:
				return sizeof(static_normal_texture);
			case elements_type::static_color:
				return sizeof(static_color);
			case elements_type::skeletal:
				return sizeof(skeletal);
			case elements_type::skeletal_color:
				return sizeof(skeletal_color);
			case elements_type::skeletal_normal:
				return sizeof(skeletal_normal);
			case elements_type::skeletal_normal_color:
				return sizeof(skeletal_normal_color);
			case elements_type::skeletal_normal_texture:
				return sizeof(skeletal_normal_texture);
			case elements_type::skeletal_normal_texture_color:
				return sizeof(skeletal_normal_texture_color);
			}
			
			return 0;
		}

		void
		pack_vertices(mesh& m)
		{
			const u32 num_vertices{ (u32)m.vertices.size() };
			assert(num_vertices);

			m.position_buffer.resize(sizeof(math::v3) * num_vertices);
			math::v3* const position_buffer{ (math::v3* const)m.position_buffer.data() };

			for (u32 i{ 0 }; i < num_vertices; ++i)
			{
				position_buffer[i] = m.vertices[i].position;
			}

			struct u16v2 { u16 x, y; };
			struct u8v3 { u8 x, y, z; };

			utl::vector<u8>	t_signs(num_vertices);
			utl::vector<u16v2> normals(num_vertices);
			utl::vector<u16v2> tangents(num_vertices);
			utl::vector<u8v3> joint_weights(num_vertices);

			if (m.elements_type & elements::elements_type::static_normal)
			{
				// normals only
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					t_signs[i] = (u8)((v.normal.z > 0.0f) << 1);
					normals[i] = { (u16)pack_float<16>(v.normal.x, -1.0f, 1.0f), (u16)pack_float<16>(v.normal.y, -1.0f, 1.0f) };
				}

				if (m.elements_type & elements::elements_type::static_normal_texture)
				{
					// full t-space
					for (u32 i{ 0 }; i < num_vertices; i++)
					{
						vertex& v{ m.vertices[i] };
						t_signs[i] |= (u8)((v.tangent.w > 0.0f) && (v.tangent.z > 0.0f));
						tangents[i] = { (u16)pack_float<16>(v.tangent.x, -1.0f, 1.0f), (u16)pack_float<16>(v.tangent.y, -1.0f, 1.0f) };
					}
				}
			}

			if (m.elements_type & elements::elements_type::skeletal)
			{
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					// pack joint weights (from [0.0, 1.0] to [0...255])
					joint_weights[i] =
					{
						(u8)pack_unit_float<8>(v.joint_weights.x),
						(u8)pack_unit_float<8>(v.joint_weights.y),
						(u8)pack_unit_float<8>(v.joint_weights.z)
					};
					// NOTE: w3 will be calculated in the shader since joint weights sum to one(1).
				}
			}

			m.element_buffer.resize(get_vertex_element_size(m.elements_type) * num_vertices);
			using namespace elements;

			switch (m.elements_type)
			{
			case elements_type::static_color:
			{
				static_color* const element_buffer{ (static_color* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					element_buffer[i] = { { v.red, v.green, v.blue }, {/*pad*/}};
				}
			}
			break;
			case elements_type::static_normal:
			{
				static_normal* const element_buffer{ (static_normal* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					element_buffer[i] = { { v.red, v.green, v.blue }, t_signs[i], {normals[i].x, normals[i].y} };
				}
			}
			break;
			case elements_type::static_normal_texture:
			{
				static_normal_texture* const element_buffer{ (static_normal_texture* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					element_buffer[i] = { { v.red, v.green, v.blue }, t_signs[i],
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y},
										 v.uv };
				}
			}
			break;
			case elements_type::skeletal:
			{
				skeletal* const element_buffer{ (skeletal* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, {/*pad*/},
										 {indices[0], indices[1], indices[2], indices[3]} };
				}
			}
			break;
			case elements_type::skeletal_color:
			{
				skeletal_color* const element_buffer{ (skeletal_color* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, {/*pad*/},
										 {indices[0], indices[1], indices[2], indices[3]},
										 {v.red, v.green, v.blue}, {/*pad*/}};
				}
			}
			break;
			case elements_type::skeletal_normal:
			{
				skeletal_normal* const element_buffer{ (skeletal_normal* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y} };
				}
			}
			break;
			case elements_type::skeletal_normal_color:
			{
				skeletal_normal_color* const element_buffer{ (skeletal_normal_color* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {v.red, v.green, v.blue}, {/*pad*/} };
				}
			}
			break;
			case elements_type::skeletal_normal_texture:
			{
				skeletal_normal_texture* const element_buffer{ (skeletal_normal_texture* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv };
				}
			}
			break;
			case elements_type::skeletal_normal_texture_color:
			{
				skeletal_normal_texture_color* const element_buffer{ (skeletal_normal_texture_color* const)m.element_buffer.data() };
				for (u32 i{ 0 }; i < num_vertices; ++i)
				{
					vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y, (u16)v.joint_indices.z, (u16)v.joint_indices.w };
					element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv,
										 {v.red, v.green, v.blue}, {/*pad*/} };
				}
			}
			break;
			}
		}

		void
		determine_elements_type(mesh& m)
		{
			using namespace elements;

			if (m.normals.size())
			{
				if (m.uv_sets.size() && m.uv_sets[0].size())
				{
					m.elements_type = elements_type::static_normal_texture;
				}
				else
				{
					m.elements_type = elements_type::static_normal;
				}
			}
			else if (m.colors.size())
			{
				m.elements_type = elements_type::static_color;
			}

			// TODO: We lack data for skeletal meshes. Expand for skeletal meshes later.
		}

		void
		process_vertices(mesh& m, const geometry_import_settings& settings)
		{
			assert((m.raw_indices.size() % 3) == 0);
			
			if (settings.calculate_normals || m.normals.empty())
			{
				recalculate_normals(m);
			}

			process_normals(m, settings.smoothing_angle);

			if (!m.uv_sets.empty())
			{
				process_uvs(m);
			}

			determine_elements_type(m);
			pack_vertices(m);
		}

		void
		pack_mesh_data(const mesh& m, utl::blob_stream_writer& blob)
		{
			// Mesh name
			blob.write((u32)m.name.size());
			blob.write(m.name.c_str(), m.name.size());
			// LoD ID
			blob.write(m.lod_id);
			// vertex element size
			const u32 elements_size{ (u32)get_vertex_element_size(m.elements_type) };
			blob.write(elements_size);
			// elements type enumeration
			blob.write((u32)m.elements_type);
			// Number of vertices
			const u32 num_vertices{ (u32)m.vertices.size() };
			blob.write(num_vertices);
			// Index size (16 bit or 32 bit)
			const u32 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			blob.write(index_size);
			// Number of indices
			const u32 num_indices{ (u32)m.indices.size() };
			blob.write(num_indices);
			// LoD threshold
			blob.write(m.lod_threshold);
			// Position buffer
			assert(m.position_buffer.size() == sizeof(math::v3) * num_vertices);
			blob.write(m.position_buffer.data(), m.position_buffer.size());
			// Element buffer
			assert(m.element_buffer.size() == elements_size * num_vertices);
			blob.write(m.element_buffer.data(), m.element_buffer.size());
			// Index data
			const u32 index_buffer_size{ index_size * num_indices };
			const u8* data{ (const u8*)m.indices.data() };
			utl::vector<u16> indices;

			if (index_size == sizeof(u16))
			{
				indices.resize(num_indices);
				for (u32 i{ 0 }; i < num_indices; ++i)
				{
					indices[i] = (u16)m.indices[i];
					data = (const u8*)indices.data();
				}
			}

			blob.write(data, index_buffer_size);
		}

		u64
		get_mesh_size(const mesh& m)
		{
			const u64 num_vertices{ m.vertices.size() };
			const u64 position_buffer_size{ m.position_buffer.size() };
			assert(position_buffer_size == sizeof(math::v3) * num_vertices);
			const u64 element_buffer_size{ m.element_buffer.size() };
			assert(element_buffer_size == get_vertex_element_size(m.elements_type) * num_vertices);
			const u64 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			const u64 index_buffer_size{ index_size * m.indices.size() };
			constexpr u64 su32{ sizeof(u32) };
			const u64 size
			{
				su32 +					// name length (number of characters)
				m.name.size() +			// room for mesh name string
				su32 +					// LoD id
				su32 +					// vertex element size (vertex size excluding position element)
				su32 +					// element type enumeration
				su32 +					// number of vertices
				su32 +					// index size (16 bit or 32 bit)
				su32 +					// number of indices
				sizeof(f32) +			// LoD threshold
				position_buffer_size +	// room for verticex positions
				element_buffer_size +	// room for verticex elements
				index_buffer_size		// room for the indices
			};

			return size;
		}

		u64
		get_scene_size(const scene& scene)
		{
			constexpr u64 su32{ sizeof(u32) };
			u64 size
			{
				su32 +				// name length (number of characters)
				scene.name.size() +	// room for scene name string
				su32				// number of LoDs
			};

			for (auto& lod : scene.lod_groups)
			{
				u64 lod_size
				{
					su32 + lod.name.size() +	// LoD name length (number of characters) and room for LoD name string
					su32						// number of meshes in this LoD
				};

				for (auto& m : lod.meshes)
				{
					lod_size += get_mesh_size(m);
				}

				size += lod_size;
			}

			return size;
		}

		bool
		split_meshes_by_material(u32 material_idx, const mesh& m, mesh& submesh)
		{
			submesh.name = m.name;
			submesh.lod_threshold = m.lod_threshold;
			submesh.lod_id = m.lod_id;
			submesh.material_used.emplace_back(material_idx);
			submesh.uv_sets.resize(m.uv_sets.size());

			const u32 num_polys{ (u32)m.raw_indices.size() / 3 };
			utl::vector<u32> vertex_ref(m.positions.size(), u32_invalid_id);

			for (u32 i{ 0 }; i < num_polys; ++i)
			{
				const u32 mtl_idx{ m.material_indices[i] };
				if (mtl_idx != material_idx) continue;

				const u32 index{ i * 3 };
				for (u32 j = index; j < index + 3; ++j)
				{
					const u32 v_idx{ m.raw_indices[j] };
					if (vertex_ref[v_idx] != u32_invalid_id)
					{
						submesh.raw_indices.emplace_back(vertex_ref[v_idx]);
					}
					else
					{
						submesh.raw_indices.emplace_back((u32)submesh.positions.size());
						vertex_ref[v_idx] = submesh.raw_indices.back();
						submesh.positions.emplace_back(m.positions[v_idx]);
					}

					if (m.normals.size())
					{
						submesh.normals.emplace_back(m.normals[j]);
					}

					if (m.tangents.size())
					{
						submesh.tangents.emplace_back(m.tangents[j]);
					}

					for (u32 k{ 0 }; k < m.uv_sets.size(); ++k)
					{
						if (m.uv_sets[k].size())
						{
							submesh.uv_sets[k].emplace_back(m.uv_sets[k][j]);
						}
					}
				}
			}

			assert((submesh.raw_indices.size() % 3) == 0);
			return !submesh.raw_indices.empty();
		}

		void
		split_meshes_by_material(scene& scene)
		{
			for (auto& lod : scene.lod_groups)
			{
				utl::vector<mesh> new_meshes;
				for (auto& m : lod.meshes)
				{
					// If moew than one material is used in this mesh
					// then split it into submeshes
					const u32 num_materials{ (u32)m.material_used.size() };
					if (num_materials > 1)
					{
						for (u32 i{ 0 }; i < num_materials; i++)
						{
							mesh submesh{};
							if (split_meshes_by_material(m.material_used[i], m, submesh))
							{
								new_meshes.emplace_back(submesh);
							}
						}
					}
					else
					{
						new_meshes.emplace_back(m);
					}
				}

				new_meshes.swap(lod.meshes);
			}
		}
	} // anonymous namespace

	void
	process_scene(scene& scene, const geometry_import_settings& settings)
	{
		split_meshes_by_material(scene);
		
		for (auto& lod : scene.lod_groups)
		{
			for (auto& m : lod.meshes)
			{
				process_vertices(m, settings);
			}
		}
	}

	void
	pack_data(const scene& scene, scene_data& data)
	{
		const u64 scene_size{ get_scene_size(scene) };
		data.buffer_size = (u32)scene_size;
		data.buffer = (u8*)CoTaskMemAlloc(scene_size);
		assert(data.buffer);

		utl::blob_stream_writer blob{ data.buffer, data.buffer_size };

		// Scene name
		blob.write((u32)scene.name.size());
		blob.write(scene.name.c_str(), scene.name.size());
		// Number of LoDs
		blob.write((u32)scene.lod_groups.size());

		for (auto& lod : scene.lod_groups)
		{
			// LoD name
			blob.write((u32)lod.name.size());
			blob.write(lod.name.c_str(), lod.name.size());
			// Number of meshes in this LoD
			blob.write((u32)lod.meshes.size());

			for (auto& m : lod.meshes)
			{
				pack_mesh_data(m, blob);
			}
		}

		assert(scene_size == blob.offset());
	}
}