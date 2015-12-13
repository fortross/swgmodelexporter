#pragma once

#include "objects/base_object.h"
#include "geometry_common.h"
//
// Animated object descriptor, builded from SAT files
//
class Animated_object_descriptor : public Base_object
{
public:
  Animated_object_descriptor()
  { }

  // getters
  // meshes
  size_t get_mesh_count() const { return m_mesh_names.size(); }
  std::string get_mesh_name(size_t idx) { return m_mesh_names[idx]; }

  // skeletons
  size_t get_skeletons_count() const { return m_skeleton_info.size(); }
  std::string get_skeleton_name(size_t idx) { return m_skeleton_info[idx].first; }
  std::string get_skeleton_attach_point(size_t idx) { return m_skeleton_info[idx].second; }

  // anim maps
  size_t get_animation_maps() const { return m_animations_map.size(); }
  std::string get_animation_for_skeleton(const std::string& skt_name) { return m_animations_map[skt_name]; }

  // fillers
  void add_mesh_name(const std::string& mesh_name) { m_mesh_names.push_back(mesh_name); }
  void add_skeleton_info(const std::string& skt_name, const std::string& slot)
  {
    m_skeleton_info.push_back(make_pair(skt_name, slot));
  }
  void add_anim_map(const std::string& skt_name, const std::string& anim_map)
  {
    m_animations_map[skt_name] = anim_map;
  }

  virtual bool is_object_correct() const override { return true; }
  virtual void store(const std::string& path) override { };
  virtual std::set<std::string> get_referenced_objects() const override;
  virtual void resolve_dependencies(const Object_cache&, const Objects_opened_by&) override { }
  virtual void set_object_name(const std::string& name) override { m_name = name;  }
  virtual std::string get_object_name() const override { return m_name; }

private:
  std::vector<std::string> m_mesh_names;
  std::vector<std::pair<std::string, std::string>> m_skeleton_info;
  std::unordered_map<std::string, std::string> m_animations_map;

  std::string m_name;
};

class Animated_lod_list : public Base_object
{
public:
  Animated_lod_list() { }

  void add_lod_name(const std::string& name) { m_lod_names.emplace_back(name); }
  // overrides
  virtual bool is_object_correct() const override { return true; }
  virtual void store(const std::string& path) override { };
  virtual std::set<std::string> get_referenced_objects() const override;
  virtual void resolve_dependencies(const Object_cache&, const Objects_opened_by&) override { }
  virtual void set_object_name(const std::string& name) override { m_name = name; }
  virtual std::string get_object_name() const override { return m_name; }
private:
  std::vector<std::string> m_lod_names;
  std::string m_name;
};

struct Bone
{
  Bone(const std::string& bone_name) 
    : name(bone_name), parent_idx(-1) { }
  std::string name;
  int32_t parent_idx; // -1 means no parent, i.e. root
  Geometry::Vector4 pre_rot_quaternion;
  Geometry::Vector4 post_rot_quaternion;
  Geometry::Vector3 bind_pose_transform;
  Geometry::Vector4 bind_pose_rotation;
  uint32_t rotation_order;
};

class Animated_mesh;

class Skeleton : public Base_object
{
public:
  Skeleton() : m_current_lod(0) { }

  void add_lod_level()
  {
    m_bones.push_back(std::vector<Bone>());
    m_current_lod = static_cast<uint32_t>(m_bones.size() - 1);
  }
  uint32_t get_lod_count() const { return static_cast<uint32_t>(m_bones.size()); }
  uint32_t get_current_lod() const { return m_current_lod; }
  void set_current_lod(uint32_t lod) { assert(lod < m_bones.size()); m_current_lod = lod; }

  void add_bone(const std::string& bone_name) { m_bones[m_current_lod].emplace_back(bone_name); }
  Bone& get_bone(uint32_t idx) { return m_bones[m_current_lod][idx]; }
  uint32_t get_bones_count() const { return static_cast<uint32_t>(m_bones[m_current_lod].size()); }

  void generate_skeleton_in_scene(FbxScene* scene_ptr, FbxNode* parent_ptr, Animated_mesh* source_mesh);
  void join_skeleton_to_point(const std::string& attach_point, const std::shared_ptr<Skeleton>& skel_to_join);

  // Inherited via Base_object
  virtual bool is_object_correct() const override;
  virtual void store(const std::string & path) override;
  virtual std::set<std::string> get_referenced_objects() const override;
  virtual void resolve_dependencies(const Object_cache & object_list, const Objects_opened_by& open_by) override;
  virtual void set_object_name(const std::string & obj_name) override;
  virtual std::string get_object_name() const override { return m_skeleton_name; }

private:
  std::string m_skeleton_name;
  std::vector<std::vector<Bone>> m_bones;
  uint32_t m_current_lod;
};

class Animated_mesh : public Base_object
{
public:
#pragma pack(push, 1)
  struct Info
  {
    uint32_t num_skeletons;
    uint32_t num_joints;
    uint32_t position_counts;
    uint32_t joint_weight_data_count;
    uint32_t normals_count;
    uint32_t num_shaders;
    uint32_t blend_targets;
    uint16_t occlusion_zones;
    uint16_t occlusion_zones_combinations;
    uint16_t occluded_zones;
    int16_t occlusion_layer;
  };
#pragma pack(pop)

  class Vertex
  {
  public:
    Vertex(const Geometry::Point& position) : m_position(position) { }
    void set_position(const Geometry::Point& point) { m_position = point; }
    const Geometry::Point& get_position() const { return m_position; }
    std::vector<std::pair<uint32_t, float>>& get_weights() { return m_weights; }
    const std::vector<std::pair<uint32_t, float>>& get_weights() const { return m_weights; }
  private:
    Geometry::Point m_position;
    std::vector<std::pair<uint32_t, float>> m_weights;
  };

  class Shader
  {
  public:
    Shader(const std::string& name) : m_name(name) { }
    const std::string& get_name() const { return m_name; }
    std::vector<uint32_t>& get_pos_indexes() { return m_position_indexes; }
    std::vector<uint32_t>& get_normal_indexes() { return m_normal_indexes; }
    std::vector<uint32_t>& get_light_indexes() { return m_light_indexes; }
    std::vector<Graphics::Tex_coord>& get_texels() { return m_texels; }
    std::vector<Graphics::Triangle_indexed>& get_triangles() { return m_triangles; }
    std::vector<std::pair<uint32_t, uint32_t>>& get_primivites() { return m_primitives; }

    void add_primitive();
    void close_primitive() { m_primitives.back().second = static_cast<uint32_t>(m_primitives.size()); }
  private:
    std::string m_name;
    std::vector<uint32_t> m_position_indexes;
    std::vector<uint32_t> m_normal_indexes;
    std::vector<uint32_t> m_light_indexes;
    std::vector<Graphics::Tex_coord> m_texels;
    std::vector<Graphics::Triangle_indexed> m_triangles;
    std::vector<std::pair<uint32_t, uint32_t>> m_primitives;
  };

  class Morph_target
  {
  public:
    Morph_target(const std::string& name) : m_name(name) { }
    std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_positions() { return m_position_offsets; }
    std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_normals() { return m_normal_offsets; }
    std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_tangents() { return m_tangent_offsets; }

    const std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_positions() const { return m_position_offsets; }
    const std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_normals() const { return m_normal_offsets; }
    const std::vector<std::pair<uint32_t, Geometry::Vector3>>& get_tangents() const { return m_tangent_offsets; }

    const std::string get_name() const { return m_name; }
  private:
    std::string m_name;
    std::vector<std::pair<uint32_t, Geometry::Vector3>> m_position_offsets;
    std::vector<std::pair<uint32_t, Geometry::Vector3>> m_normal_offsets;
    std::vector<std::pair<uint32_t, Geometry::Vector3>> m_tangent_offsets;
  };

public:
  Animated_mesh() : m_lod_level(0) { }

  const Info& get_info() const { return m_mesh_info; }
  void set_info(const Info& info);

  void add_vertex_position(const Geometry::Point& position) { m_vertices.emplace_back(position); }
  Vertex& get_vertex(const uint32_t pos_num) { return m_vertices[pos_num]; }
  const std::vector<Vertex>& get_vertices() const { return m_vertices; }
  const std::vector<std::string>& get_joint_names() const { return m_joint_names; }

  void add_normal(const Geometry::Vector3& norm) { m_normals.emplace_back(norm); }
  void add_lighting_normal(const Geometry::Vector4& light_norm) { m_lighting_normals.emplace_back(light_norm); }
  void add_skeleton_name(const std::string& name) { m_skeletons_names.emplace_back(name); }
  void add_joint_name(const std::string& name) { m_joint_names.emplace_back(name); }
  void add_new_shader(const std::string& name) { m_shaders.emplace_back(name); }
  void add_new_morph(const std::string& name) { m_morphs.emplace_back(name); }
  Shader& get_current_shader() { assert(m_shaders.empty() == false); return m_shaders.back(); }
  Morph_target& get_current_morph() { assert(m_morphs.empty() == false); return m_morphs.back(); }

  // Inherited via Base_object
  virtual bool is_object_correct() const override;
  virtual void store(const std::string & path) override;
  virtual std::set<std::string> get_referenced_objects() const override;
  virtual void resolve_dependencies(const Object_cache& object_list, const Objects_opened_by& open_by) override;
  virtual void set_object_name(const std::string& obj_name) override { m_object_name = obj_name; }
  virtual std::string get_object_name() const override { return m_object_name; }

private:
  std::string m_object_name;
  Info m_mesh_info;
  std::vector<std::string> m_skeletons_names;
  std::vector<std::string> m_joint_names;
  std::vector<Vertex> m_vertices;
  std::vector<Geometry::Vector3> m_normals;
  std::vector<Geometry::Vector4> m_lighting_normals;
  std::vector<Shader> m_shaders;
  std::vector<std::pair<std::string, std::shared_ptr<Skeleton>>> m_used_skeletons;
  std::vector<Morph_target> m_morphs;
  uint32_t m_lod_level;
};
