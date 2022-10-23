#include "graphics/renderer/renderer.hpp"

#include "spdlog/spdlog.h"

using namespace graphics::renderer;

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::initialize() {
	spdlog::info("Renderer initialized");
	return true;
}

bool Renderer::update() {
	return false;
}

void Renderer::render() const {}
