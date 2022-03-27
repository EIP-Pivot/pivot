#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DebugMacros.hxx"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace pivot::graphics
{

bool AssetStorage::loadTTFFont(const std::filesystem::path &path)
{
    DEBUG_FUNCTION;
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        logger.err("Asset Storage/TTF") << "Failed to initialize the FreeType library";
        return false;
    }
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        logger.err("Asset Storage/TTF") << "Failed to load ttf file !";
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    auto stem = path.stem().string();
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            logger.warn("Asset Storage/TTF") << "Failed to load glyph " << char(c);
            continue;
        }
        logger.trace("Asset Stoage/TTF") << "Loading glyph " << int(c) << ": " << char(c);
        CPUTexture texture;
        texture.format = vk::Format::eR8Unorm;
        texture.size.height = face->glyph->bitmap.rows;
        texture.size.width = face->glyph->bitmap.width;
        Character character{
            .bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top},
            .advance = unsigned(face->glyph->advance.x),
        };
        if (texture.image.size() != 0) {
            character.textureId = stem + '/' + char(c);
            texture.image.resize(texture.size.height * texture.size.width);
            std::memcpy(texture.image.data(), face->glyph->bitmap.buffer, texture.size.height * texture.size.width);
            cpuStorage.textureStaging.add(character.textureId, texture);
        }
        charStorage.emplace(character.textureId, character);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return true;
}

}    // namespace pivot::graphics
