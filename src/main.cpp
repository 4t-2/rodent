#include "../lib/AGL/agl.hpp"
#include <fstream>

#define FONTSIZE 24
#define BORDER	 2
#define PADDING	 2

#define DARKG      \
	{              \
		63, 63, 63 \
	}

class Listener
{
	private:
		std::function<void()> first;
		std::function<void()> hold;
		std::function<void()> last;
		bool				  pastState = false;

	public:
		Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last);
		void update(bool state);
};

Listener::Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last)
{
	this->first = first;
	this->hold	= hold;
	this->last	= last;
}

void Listener::update(bool state)
{
	if (state)
	{
		if (pastState)
		{
			hold();
		}
		else
		{
			first();

			pastState = true;
		}
	}
	else if (pastState)
	{
		last();
		pastState = false;
	}
}

enum Split
{
	Vertical,
	Horizontal,
	Root,
};

class Pane : public agl::Drawable
{
	public:
		agl::Rectangle	*rect = nullptr;
		agl::Vec<int, 2> size;
		agl::Vec<int, 2> pos   = {0, 0};
		Split			 split = Split::Root;
		float			 percent; // 1.0 means Bchild covered, 0.0 Achild covered
		Pane			*Achild = nullptr;
		Pane			*Bchild = nullptr;
		agl::Text		*text	= nullptr;
		std::string		 str;

		Pane(agl::Rectangle *rect, agl::Text *text) : rect(rect), text(text)
		{
		}

		void splitPane(Split type, float percent)
		{
			Achild = new Pane(rect, text);
			Bchild = new Pane(rect, text);

			split		  = type;
			this->percent = percent;

			return;
		}

		void updateSize(agl::RenderWindow &window)
		{
			size.x = window.getWindowAttributes().width;
			size.y = window.getWindowAttributes().height;

			return;
		}

		void drawFunction(agl::RenderWindow &window) override;

		~Pane()
		{
			delete Achild;
			delete Bchild;
		}
};

Pane			*focusPane = nullptr;
agl::Vec<int, 2> clickPos;
bool			 clickEvent = false;

void Pane::drawFunction(agl::RenderWindow &window)
{
	switch (split)
	{
		case Split::Vertical:
			Achild->pos	   = pos;
			Achild->size.y = size.y * percent;
			Achild->size.x = size.x;

			Bchild->pos = pos;
			Bchild->pos.y += Achild->size.y;
			Bchild->size.y = size.y * (1.0 - percent);
			Bchild->size.x = size.x;

			window.draw(*Achild);
			window.draw(*Bchild);

			break;
		case Split::Horizontal:
			Achild->pos	   = pos;
			Achild->size.x = size.x * percent;
			Achild->size.y = size.y;

			Bchild->pos = pos;
			Bchild->pos.x += Achild->size.x;
			Bchild->size.x = size.x * (1.0 - percent);
			Bchild->size.y = size.y;

			window.draw(*Achild);
			window.draw(*Bchild);

			break;
		case Split::Root:
			// logic

			if (clickEvent)
			{
				std::cout << clickPos << '\n';
				if (clickPos.x > pos.x && clickPos.y > pos.y)
				{
					if (clickPos.x < (pos.x + size.x) && clickPos.y < (pos.y + size.y))
					{
						focusPane = this;
					}
				}
			}

			// render
			rect->setSize(size);
			rect->setPosition(pos);
			rect->setColor(agl::Color::Gray);
			window.drawShape(*rect);

			rect->setSize(size - agl::Vec<int, 2>{BORDER + BORDER, BORDER + BORDER});
			rect->setPosition(pos + agl::Vec<int, 2>{BORDER, BORDER});

			if (focusPane == this)
			{
				rect->setColor(DARKG);
			}
			else
			{
				rect->setColor(agl::Color::Black);
			}

			window.drawShape(*rect);

			text->setPosition(pos + agl::Vec<int, 2>{BORDER, BORDER} + agl::Vec<int, 2>{PADDING, PADDING});
			text->clearText();
			text->setText("test");

			window.drawText(*text);

			break;
	}
}

int main()
{
	agl::RenderWindow window;
	window.setup({1920, 1080}, "Rodent");
	window.setClearColor(agl::Color::Black);
	window.setFPS(60);

	glDisable(GL_DEPTH_TEST);

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

	agl::Font font;
	font.setup("/usr/share/fonts/TTF/Arial.TTF", FONTSIZE);

	agl::Text text;
	text.setColor(agl::Color::White);
	text.setPosition({0, 0});
	text.setFont(&font);
	text.setScale(1);

	std::string str = "";

	bool escHeld = false;

	agl::Rectangle rect;
	rect.setTexture(&blank);

	Listener mouseListener(
		[&]() {
			clickPos   = event.getPointerRootPosition();
			clickEvent = true;
		},
		[&]() { clickEvent = false; }, [&]() { clickEvent = false; });

	Pane pane(&rect, &text);

	pane.splitPane(Split::Horizontal, 0.1);

	while (!event.windowClose())
	{
		event.poll();

		window.clear();

		pane.updateSize(window);
		window.draw(pane);

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

		if (event.isKeyPressed(XK_Escape))
		{
			if (escHeld)
			{
			}
			else
			{
				std::fstream fs("output.txt", std::ios::out);

				fs << str;

				fs.close();

				std::cout << "Wrote to file" << '\n';

				escHeld = true;
			}
		}
		else
		{
			escHeld = false;
		}

		mouseListener.update(event.isPointerButtonPressed(Button1Mask));

		agl::Vec<int, 2> windowSize;
		windowSize.x = window.getWindowAttributes().width;
		windowSize.y = window.getWindowAttributes().height;

		camera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);
		window.setViewport(0, 0, windowSize.x, windowSize.y);

		window.updateMvp(camera);
	}

	text.clearText();
	font.deleteFont();
	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
