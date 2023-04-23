#include "../lib/AGL/agl.hpp"

int main()
{
	agl::RenderWindow window;
	window.setup({1920, 1080}, "Rodent");
	window.setClearColor(agl::Color::Black);
	window.setFPS(30);

	agl::Event event;
	event.setWindow(window);

	agl::ShaderBuilder sbFrag;
	sbFrag.setDefaultFrag();
	std::string fragSrc = sbFrag.getSrc();

	agl::ShaderBuilder sbVert;
	sbVert.setDefaultVert();
	std::string vertSrc = sbVert.getSrc();

	agl::Shader shader;
	shader.compileSrc(vertSrc, fragSrc);

	window.getShaderUniforms(shader);
	shader.use();

	agl::Camera camera;
	camera.setView({0, 0, 10}, {0, 0, 0}, {0, 1, 0});
	camera.setOrthographicProjection(0, 1920, 1080, 0, 0.1, 100);

	window.updateMvp(camera);

	agl::Texture blank;
	blank.setBlank();

	agl::Rectangle rectangle;
	rectangle.setColor(agl::Color::Cyan);
	rectangle.setTexture(&blank);
	rectangle.setPosition({100, 100});
	rectangle.setSize({1720, 880});

	while (!event.windowClose())
	{
		event.poll();

		window.clear();

		window.drawShape(rectangle);
		
		window.display();
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
