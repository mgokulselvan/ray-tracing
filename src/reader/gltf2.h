#ifndef GLTF2READER_H
#define GLTF2READER_H

#include "../external/tiny_gltf/tiny_gltf_v3.h"

#include "../core/hittable_list.h"
#include "../core/camera.h"
#include "../shape/tri.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <algorithm>

inline tg3_model* gltf2_open(const char* filepath) {
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model *model = new tg3_model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(model, &errors, filepath,
                                        10, &opts);
    if (err != TG3_OK) {
        for (int i = 0; i < errors.count; i++) {
            fprintf(stderr, "[%d] %s\n", (int)errors.entries[i].severity,
                    errors.entries[i].message ? errors.entries[i].message : "(null)");
        }
        tg3_model_free(model);
        tg3_error_stack_free(&errors);
        throw std::runtime_error("error reading gltf2 file");
    }

    tg3_error_stack_free(&errors);
    return model;
}

inline void gltf2_close(tg3_model *model) {
    if (model) tg3_model_free(model);
}

// -------------------------------------------------


struct mat4 {
    double m[4][4] = {};

    static mat4 identity() {
        mat4 result;

        for (int i = 0; i < 4; i++)
            result.m[i][i] = 1.0;

        return result;
    }
};

inline mat4 operator*(const mat4& a, const mat4& b) {
    mat4 result{};
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                result.m[row][col] += a.m[row][k] * b.m[k][col];
    return result;
}

inline mat4 translation_matrix(const vec3& t) {
    mat4 result = mat4::identity();
    result.m[0][3] = t.x();
    result.m[1][3] = t.y();
    result.m[2][3] = t.z();
    return result;
}

inline mat4 scale_matrix(const vec3& s) {
    mat4 result = mat4::identity();
    result.m[0][0] = s.x();
    result.m[1][1] = s.y();
    result.m[2][2] = s.z();
    return result;
}

// glTF quaternions are stored as (x, y, z, w).
inline mat4 quaternion_matrix(double x, double y, double z, double w) {
    // Normalize defensively; a zero quaternion is treated as identity.
    const double length = std::sqrt(x*x + y*y + z*z + w*w);
    if (length > 1e-12) {
        x /= length; y /= length; z /= length; w /= length;
    } else {
        x = y = z = 0.0;
        w = 1.0;
    }

    mat4 result = mat4::identity();
    result.m[0][0] = 1.0 - 2.0 * (y*y + z*z);
    result.m[0][1] = 2.0 * (x*y - z*w);
    result.m[0][2] = 2.0 * (x*z + y*w);
    result.m[1][0] = 2.0 * (x*y + z*w);
    result.m[1][1] = 1.0 - 2.0 * (x*x + z*z);
    result.m[1][2] = 2.0 * (y*z - x*w);
    result.m[2][0] = 2.0 * (x*z - y*w);
    result.m[2][1] = 2.0 * (y*z + x*w);
    result.m[2][2] = 1.0 - 2.0 * (x*x + y*y);
    return result;
}

inline point3 transform_point(const mat4& transform, const point3& p) {
    const double x = transform.m[0][0] * p.x() + transform.m[0][1] * p.y() +
                     transform.m[0][2] * p.z() + transform.m[0][3];
    const double y = transform.m[1][0] * p.x() + transform.m[1][1] * p.y() +
                     transform.m[1][2] * p.z() + transform.m[1][3];
    const double z = transform.m[2][0] * p.x() + transform.m[2][1] * p.y() +
                     transform.m[2][2] * p.z() + transform.m[2][3];
    const double w = transform.m[3][0] * p.x() + transform.m[3][1] * p.y() +
                     transform.m[3][2] * p.z() + transform.m[3][3];

    if (std::abs(w) < 1e-12)
        throw std::runtime_error("Invalid node transform: point has w=0");
    return point3(x / w, y / w, z / w);
}

inline vec3 transform_vector(const mat4& transform, const vec3& v) {
    // A direction has homogeneous w=0, so translation is not applied.
    return vec3(
        transform.m[0][0] * v.x() + transform.m[0][1] * v.y() + transform.m[0][2] * v.z(),
        transform.m[1][0] * v.x() + transform.m[1][1] * v.y() + transform.m[1][2] * v.z(),
        transform.m[2][0] * v.x() + transform.m[2][1] * v.y() + transform.m[2][2] * v.z()
    );
}

inline mat4 get_node_transform(const tg3_node& node) {
    // glTF specifies either matrix OR TRS. Matrix takes precedence.
    if (node.has_matrix) {
        mat4 result{};
        // The glTF matrix array is column-major; mat4 is indexed row, column.
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                result.m[row][col] = node.matrix[col * 4 + row];
        return result;
    }

    const vec3 translation(node.translation[0], node.translation[1], node.translation[2]);
    const vec3 scale(node.scale[0], node.scale[1], node.scale[2]);
    const mat4 T = translation_matrix(translation);
    const mat4 R = quaternion_matrix(node.rotation[0], node.rotation[1],
                                     node.rotation[2], node.rotation[3]);
    const mat4 S = scale_matrix(scale);

    // Column-vector convention: local point = T * R * S * point.
    return T * R * S;
}

inline vec3 read_position(const tg3_model& model, int accessor_index, int vertex_index) {
    if (accessor_index < 0 || static_cast<uint32_t>(accessor_index) >= model.accessors_count)
        throw std::runtime_error("POSITION accessor index out of range");

    const tg3_accessor& accessor = model.accessors[accessor_index];
    if (accessor.type != TG3_TYPE_VEC3 ||
        accessor.component_type != TG3_COMPONENT_TYPE_FLOAT ||
        accessor.buffer_view < 0 ||
        static_cast<uint32_t>(vertex_index) >= accessor.count) {
        throw std::runtime_error("Unsupported POSITION accessor (expected float VEC3 with a bufferView)");
    }
    if (static_cast<uint32_t>(accessor.buffer_view) >= model.buffer_views_count)
        throw std::runtime_error("POSITION bufferView index out of range");

    const tg3_buffer_view& view = model.buffer_views[accessor.buffer_view];
    if (view.buffer < 0 || static_cast<uint32_t>(view.buffer) >= model.buffers_count)
        throw std::runtime_error("POSITION buffer index out of range");

    const tg3_buffer& buffer = model.buffers[view.buffer];
    const uint64_t element_size = 3 * sizeof(float);
    const uint64_t stride = view.byte_stride ? view.byte_stride : element_size;
    if (stride < element_size)
        throw std::runtime_error("Invalid POSITION byteStride");

    const uint64_t offset = static_cast<uint64_t>(view.byte_offset) +
                            static_cast<uint64_t>(accessor.byte_offset) +
                            static_cast<uint64_t>(vertex_index) * stride;
    if (offset > buffer.data.count || element_size > buffer.data.count - offset)
        throw std::runtime_error("POSITION data out of bounds");

    float p[3];
    std::memcpy(p, buffer.data.data + offset, sizeof(p));
    return vec3(p[0], p[1], p[2]);
}

inline uint32_t read_index(const tg3_model& model, int accessor_index, uint32_t index_number) {
    if (accessor_index < 0 || static_cast<uint32_t>(accessor_index) >= model.accessors_count)
        throw std::runtime_error("Index accessor index out of range");

    const tg3_accessor& accessor = model.accessors[accessor_index];
    if (accessor.type != TG3_TYPE_SCALAR || accessor.buffer_view < 0 || index_number >= accessor.count)
        throw std::runtime_error("Invalid index accessor (expected scalar) or index offset");
    if (static_cast<uint32_t>(accessor.buffer_view) >= model.buffer_views_count)
        throw std::runtime_error("Index bufferView index out of range");

    const tg3_buffer_view& view = model.buffer_views[accessor.buffer_view];
    if (view.buffer < 0 || static_cast<uint32_t>(view.buffer) >= model.buffers_count)
        throw std::runtime_error("Index buffer index out of range");

    const tg3_buffer& buffer = model.buffers[view.buffer];
    uint32_t size = 0;
    switch (accessor.component_type) {
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:  size = 1; break;
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: size = 2; break;
        case TG3_COMPONENT_TYPE_UNSIGNED_INT:   size = 4; break;
        default: throw std::runtime_error("Unsupported glTF index component type");
    }

    // Index accessors are tightly packed; byteStride is not used for indices.
    const uint64_t offset = static_cast<uint64_t>(view.byte_offset) +
                            static_cast<uint64_t>(accessor.byte_offset) +
                            static_cast<uint64_t>(index_number) * size;
    if (offset > buffer.data.count || size > buffer.data.count - offset)
        throw std::runtime_error("Index data out of bounds");

    const uint8_t* ptr = buffer.data.data + offset;
    if (size == 1) return ptr[0];
    if (size == 2) {
        uint16_t value;
        std::memcpy(&value, ptr, sizeof(value));
        return value;
    }
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return value;
}


// Convert glTF's metallic-roughness material model to the simpler RTIOW
// material classes available in material.h.
//
// Policy:
//   * Any non-zero emissiveFactor -> diffuse_light (emission wins).
//   * metallicFactor >= 0.5       -> metal; roughnessFactor becomes fuzz.
//   * metallicFactor < 0.5        -> lambertian.
//   * No glTF core property maps reliably to dielectric or isotropic.
// This is an intentional approximation: RTIOW has no mixed metallic/dielectric
// BSDF, transmission, IOR, or texture-backed PBR implementation.
inline double gltf_clamp01(double value) {
    if (!std::isfinite(value)) return 0.0;
    return (value < 0.0 ? 0.0 : (value > 1.0 ? 1.0 : value));
}

inline color gltf_base_color(const tg3_material& source) {
    const auto& pbr = source.pbr_metallic_roughness;
    return color(
        gltf_clamp01(pbr.base_color_factor[0]),
        gltf_clamp01(pbr.base_color_factor[1]),
        gltf_clamp01(pbr.base_color_factor[2])
    );
}

inline shared_ptr<material> gltf2_make_material(
    const tg3_model& model,
    int material_index
) {
    ///////////////////////////
    /* return make_shared<diffuse_light>(color(5, 0, 0)); */
    ///////////////////////////

    // glTF's implicit default material uses white base color, metallic=1,
    // roughness=1. Model that as a very rough white metal in this renderer.
    if (material_index < 0) {
        make_shared<lambertian>(color(0.8, 0.8, 0.8));
        return make_shared<metal>(color(1.0, 1.0, 1.0), 1.0);
    }

    if (static_cast<uint32_t>(material_index) >= model.materials_count)
        throw std::runtime_error("glTF primitive material index out of range");

    const tg3_material& source = model.materials[material_index];
    const auto& pbr = source.pbr_metallic_roughness;
    const color base = gltf_base_color(source);

    const double emissive_r = std::max(0.0, std::isfinite(source.emissive_factor[0]) ? source.emissive_factor[0] : 0.0);
    const double emissive_g = std::max(0.0, std::isfinite(source.emissive_factor[1]) ? source.emissive_factor[1] : 0.0);
    const double emissive_b = std::max(0.0, std::isfinite(source.emissive_factor[2]) ? source.emissive_factor[2] : 0.0);

    if (emissive_r > 0.0 || emissive_g > 0.0 || emissive_b > 0.0) {
        return make_shared<diffuse_light>(color(emissive_r, emissive_g, emissive_b));
    }

    const double metallic = gltf_clamp01(pbr.metallic_factor);
    const double roughness = gltf_clamp01(pbr.roughness_factor);

    // This renderer has only discrete Lambertian and metal models. A midpoint
    // threshold is a predictable compromise for intermediate metallic values.
    if (metallic >= 0.5)
        return make_shared<metal>(base, roughness);

    return make_shared<lambertian>(base);
}

inline const tg3_scene& gltf2_get_scene(const tg3_model* model) {
    if (!model || model->scenes_count == 0)
        throw std::runtime_error("glTF has no scenes");

    int scene_index = model->default_scene;
    if (scene_index < 0 || static_cast<uint32_t>(scene_index) >= model->scenes_count)
        scene_index = 0; // glTF default scene is optional.
    return model->scenes[scene_index];
}

inline void import_node(
    const tg3_model* model,
    int node_index,
    hittable_list* world,
    int& triangle_count,
    const mat4& parent_transform
) {
    if (!model || !world)
        throw std::runtime_error("import_node received a null model or world");
    if (node_index < 0 || static_cast<uint32_t>(node_index) >= model->nodes_count)
        throw std::runtime_error("glTF node index out of range");

    const tg3_node& node = model->nodes[node_index];
    // glTF hierarchy: world = parentWorld * local.
    const mat4 world_transform = parent_transform * get_node_transform(node);

    if (node.mesh >= 0) {
        if (static_cast<uint32_t>(node.mesh) >= model->meshes_count)
            throw std::runtime_error("glTF node mesh index out of range");

        const tg3_mesh& mesh = model->meshes[node.mesh];
        for (uint32_t primitive_index = 0; primitive_index < mesh.primitives_count; ++primitive_index) {
            const tg3_primitive& primitive = mesh.primitives[primitive_index];
            // Materials belong to primitives, not nodes or meshes.
            const shared_ptr<material> primitive_material =
                gltf2_make_material(*model, primitive.material);

            // This reader intentionally supports triangle lists only.
            if (primitive.mode != -1 && primitive.mode != TG3_MODE_TRIANGLES)
                continue;

            int position_accessor = -1;
            for (uint32_t attribute_index = 0; attribute_index < primitive.attributes_count; ++attribute_index) {
                const tg3_str_int_pair& attribute = primitive.attributes[attribute_index];
                if (attribute.key.data && std::strcmp(attribute.key.data, "POSITION") == 0) {
                    position_accessor = attribute.value;
                    break;
                }
            }
            if (position_accessor < 0) continue;
            if (static_cast<uint32_t>(position_accessor) >= model->accessors_count)
                throw std::runtime_error("POSITION accessor index out of range");

            const tg3_accessor& positions = model->accessors[position_accessor];
            uint32_t index_count = positions.count;
            if (primitive.indices >= 0) {
                if (static_cast<uint32_t>(primitive.indices) >= model->accessors_count)
                    throw std::runtime_error("Primitive index accessor out of range");
                index_count = model->accessors[primitive.indices].count;
            }

            if (index_count % 3 != 0)
                throw std::runtime_error("Triangle primitive has an index count not divisible by 3");

            for (uint32_t i = 0; i < index_count; i += 3) {
                const uint32_t i0 = primitive.indices >= 0
                    ? read_index(*model, primitive.indices, i) : i;
                const uint32_t i1 = primitive.indices >= 0
                    ? read_index(*model, primitive.indices, i + 1) : i + 1;
                const uint32_t i2 = primitive.indices >= 0
                    ? read_index(*model, primitive.indices, i + 2) : i + 2;

                if (i0 >= positions.count || i1 >= positions.count || i2 >= positions.count)
                    throw std::runtime_error("Triangle vertex index exceeds POSITION accessor count");

                const vec3 a = read_position(*model, position_accessor, static_cast<int>(i0));
                const vec3 b = read_position(*model, position_accessor, static_cast<int>(i1));
                const vec3 c = read_position(*model, position_accessor, static_cast<int>(i2));

                // Bake the full node-to-world transform into the triangle vertices.
                const point3 A = transform_point(world_transform, a);
                const point3 B = transform_point(world_transform, b);
                const point3 C = transform_point(world_transform, c);
                world->add(make_shared<tri>(A, B, C, primitive_material));
                ++triangle_count;
            }
        }
    }

    for (uint32_t child = 0; child < node.children_count; ++child)
        import_node(model, node.children[child], world, triangle_count, world_transform);
}

inline void gltf2_import_into_world(
    const tg3_model* model,
    hittable_list* world
) {
    if (!model || !world) throw std::runtime_error("Invalid glTF import arguments");
    const tg3_scene& scene = gltf2_get_scene(model);
    int triangle_count = 0;
    const mat4 identity = mat4::identity();

    for (uint32_t i = 0; i < scene.nodes_count; ++i)
        import_node(model, scene.nodes[i], world, triangle_count, identity);

    std::cerr << "Total imported triangles: " << triangle_count << '\n';
}

inline bool import_camera_node(
    const tg3_model& model,
    int node_index,
    const mat4& parent_transform,
    camera& cam
) {
    if (node_index < 0 || static_cast<uint32_t>(node_index) >= model.nodes_count)
        throw std::runtime_error("Camera node index out of range");

    const tg3_node& node = model.nodes[node_index];
    const mat4 world_transform = parent_transform * get_node_transform(node);

    if (node.camera >= 0) {
        if (static_cast<uint32_t>(node.camera) >= model.cameras_count)
            throw std::runtime_error("glTF camera index out of range");

        const tg3_camera& gltf_camera = model.cameras[node.camera];
        if (gltf_camera.type.data && std::strcmp(gltf_camera.type.data, "perspective") == 0) {
            const auto& perspective = gltf_camera.perspective;
            constexpr double pi = 3.14159265358979323846;
            cam.vfov = perspective.yfov * 180.0 / pi;
            if (perspective.aspect_ratio > 0)
                cam.aspect_ratio = perspective.aspect_ratio;

            // glTF cameras look down local -Z, with +Y as up.
            const point3 position = transform_point(world_transform, point3(0, 0, 0));
            const vec3 forward = transform_vector(world_transform, vec3(0, 0, -1));
            const vec3 up = transform_vector(world_transform, vec3(0, 1, 0));
            cam.lookfrom = position;
            cam.lookat = position + forward;
            cam.vup = up;
            return true;
        }
    }

    for (uint32_t i = 0; i < node.children_count; ++i)
        if (import_camera_node(model, node.children[i], world_transform, cam))
            return true;
    return false;
}

inline void gltf2_import_camera(const tg3_model& model, camera* cam) {
    if (!cam) throw std::runtime_error("gltf2_import_camera received a null camera");
    const tg3_scene& scene = gltf2_get_scene(&model);
    const mat4 identity = mat4::identity();
    for (uint32_t i = 0; i < scene.nodes_count; ++i)
        if (import_camera_node(model, scene.nodes[i], identity, *cam))
            return;
}

#endif
