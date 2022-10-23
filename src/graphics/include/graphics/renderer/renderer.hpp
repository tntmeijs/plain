#ifndef GRAPHICS_RENDERER_RENDERER_HPP
#define GRAPHICS_RENDERER_RENDERER_HPP

namespace graphics::renderer {

	class Renderer final {
	public:
		Renderer();
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;
		~Renderer();
	};

}

#endif // !GRAPHICS_RENDERER_RENDERER_HPP
