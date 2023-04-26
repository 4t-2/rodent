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

	agl::Font font;
	font.setup("/usr/share/fonts/TTF/Arial.TTF", 24);

	agl::Text text;
	text.setColor(agl::Color::White);
	text.setPosition({0, 0});
	text.setFont(&font);
	text.setScale(1);

	std::string str = "";

	while (!event.windowClose())
	{
		event.poll();

		window.clear();

		window.drawText(text);

		window.display();

		// also lazy fix, cant be bothered to do utf8 yet

		bool utf8 = false;

		for (const char &byte : event.keybuffer)
		{
			if (byte & (1 << 8))
			{
				utf8 = true;
				break;
			}
		}

		if (!utf8)
		{
			str += event.keybuffer;

			while (str[0] == 127 || str[0] == 8)
			{
				str.erase(0, 1);
			}

			for (int i = 1; i < str.length(); i++)
			{
				if (str[i] == 13)
				{
					str[i] = 10;
				}

				if (str[i] == 127 || str[i] == 8)
				{
					str.erase(i - 1, 2);
					i--;
				}
			}
		}

		text.clearText();
		text.setText(str);
	}

	text.clearText();
	font.deleteFont();
	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
