#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <glm/gtc/type_ptr.hpp>
#include <ktx.h>
#include <ktxvulkan.h>
#include <tiny_gltf.h>

static bool loadImageDataFunc(tinygltf::Image *image, const int imageIndex, std::string *error, std::string *warning,
                              int req_width, int req_height, const unsigned char *bytes, int size, void *userData)
{
    // KTX files will be handled by our own code
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") { return true; }
    }

    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

namespace pivot::graphics
{

static std::pair<std::string, AssetStorage::CPUMaterial> loadGltfMaterial(const tinygltf::Model &gltfModel,
                                                                          const tinygltf::Material &mat)
{
    AssetStorage::CPUMaterial material;
    if (mat.values.find("baseColorTexture") != mat.values.end()) {
        material.baseColorTexture = gltfModel.textures.at(mat.values.at("baseColorTexture").TextureIndex()).name;
    }
    // Metallic roughness workflow
    if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
        material.metallicRoughnessTexture =
            gltfModel.textures.at(mat.values.at("metallicRoughnessTexture").TextureIndex()).name;
    }
    if (mat.values.find("roughnessFactor") != mat.values.end()) {
        material.roughness = mat.values.at("roughnessFactor").Factor();
    }
    if (mat.values.find("metallicFactor") != mat.values.end()) {
        material.metallic = mat.values.at("metallicFactor").Factor();
    }
    if (mat.values.find("baseColorFactor") != mat.values.end()) {
        material.baseColor = glm::make_vec4(mat.values.at("baseColorFactor").ColorFactor().data());
    }
    if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
        material.normalTexture = gltfModel.textures[mat.additionalValues.at("normalTexture").TextureIndex()].name;
    }
    if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
        material.emissiveTexture =
            gltfModel.textures.at(mat.additionalValues.at("emissiveTexture").TextureIndex()).name;
    }
    if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
        material.occlusionTexture =
            gltfModel.textures.at(mat.additionalValues.at("occlusionTexture").TextureIndex()).name;
    }
    return std::make_pair(mat.name, material);
}

static AssetStorage::CPUTexture loadGltfTexture(const tinygltf::Image &gltfimage,
                                                const std::filesystem::path &base_path)
{
    AssetStorage::CPUTexture texture;
    auto path = base_path / gltfimage.uri;
    if (path.extension() == ".ktx") {
        ktxTexture *ktxTexture = nullptr;
        auto result =
            ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        if (result != KTX_SUCCESS) { throw AssetStorage::AssetStorageException("Failed to load ktx texture"); }
        ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);
        texture.image.resize(ktxTextureSize);
        std::memcpy(texture.image.data(), ktxTextureData, ktxTextureSize);
        texture.size = vk::Extent3D{
            .width = ktxTexture->baseWidth,
            .height = ktxTexture->baseHeight,
            .depth = ktxTexture->baseDepth,
        };
        ktxTexture_Destroy(ktxTexture);
    } else {
        auto size = gltfimage.width * gltfimage.height * 4;
        texture.image.resize(size);
        std::memcpy(texture.image.data(), gltfimage.image.data(), size);
        texture.size = vk::Extent3D{
            .width = static_cast<uint32_t>(gltfimage.width),
            .height = static_cast<uint32_t>(gltfimage.height),
            .depth = 1,
        };
    }
    return texture;
}

bool AssetStorage::loadGltfModel(const std::filesystem::path &path)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    std::string error, warning;
    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;
    auto parent_dir = path.parent_path();

    gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path);
    if (!warning.empty()) { logger.warn("LOADING_OBJ") << warning; }
    if (!error.empty()) {
        logger.err("LOADING_OBJ") << error;
        return false;
    }
    for (auto &image: gltfModel.images) loadGltfTexture(image, parent_dir);
    for (auto &material: gltfModel.materials) cpuStorage.materialStaging.add(loadGltfMaterial(gltfModel, material));
    return true;
}

}    // namespace pivot::graphics