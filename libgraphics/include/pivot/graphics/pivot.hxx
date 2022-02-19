#pragma once

#include "pivot/graphics/AssetStorage.hxx"

#include <Logger.hpp>

using namespace pivot::graphics;

#define CATCH_PIVOT_EXCEPTIONS                                                                                  \
    catch (const AssetStorage::AssetStorageException &ase) { logger.err("THROW/Asset Storage") << ase.what(); } \
    catch (const Window::WindowError &we) { logger.err("THOW/Window") << we.what(); }                           \
    catch (const VulkanException &ve) { logger.err("THROW/VulkanException") << ve.what(); }
