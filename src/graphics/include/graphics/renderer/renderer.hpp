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

		// Initialize the renderer's systems
		bool initialize();

		// Prepare for drawing
		bool update();

		// Draw the scene
		void render() const;
	};

}

#endif // !GRAPHICS_RENDERER_RENDERER_HPP
