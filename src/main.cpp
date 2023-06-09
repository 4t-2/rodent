#include "../lib/AGL/agl.hpp"
#include <fstream>

#include "../inc/Context.hpp"
#include "../inc/Listener.hpp"
#include "../inc/Pane.hpp"
#include "../inc/ActionWheel.hpp"

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

	agl::Vec<int, 2> clickPos;
	agl::Vec<int, 2> currentPos;
	bool clickEvent;
	bool leftDown;

	Listener mouseListener(
		[&]() {
			clickPos	 = event.getPointerWindowPosition();
			clickEvent = true;
		},
		[&]() { clickEvent = false; }, [&]() { clickEvent = false; });

	Pane pane(&rect, &text);

	Context context(&rect, &text, &event);

	Atom clipboardProperty = XInternAtom(window.getDisplay(), "CLIPBOARD", False);
	Atom utf8String		   = XInternAtom(window.getDisplay(), "UTF8_STRING", False);

	context.actions[0] = {"Cut", [&]() {
							  context.actions[1].action();
							  context.actions[3].action();
						  }};
	context.actions[1] = {
		"Copy", [&]() {
			if (Pane::focusPane == nullptr)
			{
				return;
			}

			std::string selection;

			selection =
				Pane::focusPane->str.substr(Pane::focusPane->selection.start.i + 1,
											Pane::focusPane->selection.end.i - (Pane::focusPane->selection.start.i));

			std::cout << selection << '\n';
			XSetSelectionOwner(window.getDisplay(), clipboardProperty, window.getWindow(), CurrentTime);
			XChangeProperty(window.getDisplay(), window.getWindow(), clipboardProperty, utf8String, 8, PropModeReplace,
							(unsigned char *)selection.c_str(), selection.length());
		}};
	context.actions[2] = {"Paste", [&]() {
							  Atom			 actualType;
							  int			 actualFormat;
							  unsigned long	 itemCount, bytesAfter;
							  unsigned char *data;
							  XGetWindowProperty(window.getDisplay(), window.getWindow(), clipboardProperty, 0, 1000000,
												 False, utf8String, &actualType, &actualFormat, &itemCount, &bytesAfter,
												 &data);

							  Pane::keybuffer += std::string((char *)data, itemCount);

							  XFree(data);
						  }};
	context.actions[3] = {"Delete", [&]() { Pane::keybuffer += 8; }};

	agl::Circle circle(32);
	circle.setTexture(&blank);

	agl::Shape line([](agl::Shape &shape) {
		float vertexBufferData[6];
		float UVBufferData[4];

		vertexBufferData[0] = 0;
		vertexBufferData[1] = 0;
		vertexBufferData[2] = 0;
		vertexBufferData[3] = 1;
		vertexBufferData[4] = 1;
		vertexBufferData[5] = 0;

		UVBufferData[0] = vertexBufferData[0];
		UVBufferData[1] = vertexBufferData[1];
		UVBufferData[2] = vertexBufferData[3];
		UVBufferData[3] = vertexBufferData[4];

		shape.genBuffers();
		shape.setMode(GL_LINES);
		shape.setBufferData(vertexBufferData, UVBufferData, 2);
	});
	line.setTexture(&blank);

	ActionWheel actionWheel(&circle, &text, &line);
	actionWheel.action[0].label = "aaa";
	actionWheel.action[1].label = "bbb";
	actionWheel.action[2].label = "ccc";
	actionWheel.action[3].label = "ddd";
	actionWheel.action[4].label = "eee";
	actionWheel.action[5].label = "fff";
	// actionWheel.action[6].label = "ggg";

	pane.splitPane(Split::Horizontal, 0.5);

	while (!event.windowClose())
	{
		event.poll();

		window.clear();

		pane.updateSize(window);
		actionWheel.center.x= window.getWindowAttributes().width / 2;
		actionWheel.center.y= window.getWindowAttributes().height / 2;

		window.draw(pane);

		Pane::keybuffer = "";

		window.draw(context);
		window.draw(actionWheel);

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

		if(event.isKeyPressed(XK_Left))
		{
			actionWheel.open();
			actionWheel.exist = true;
		} else {
			actionWheel.exist = false;
		}

		if (!utf8)
		{
			Pane::keybuffer += event.keybuffer;
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

		currentPos = event.getPointerWindowPosition();
		leftDown	 = event.isPointerButtonPressed(Button1Mask);

		if (event.isPointerButtonPressed(Button3Mask))
		{
			context.create();
		}

		context.leftDown = leftDown;

		if(!context.exists)
		{
			Pane::clickPos = clickPos;
			Pane::clickEvent = clickEvent;
			Pane::currentPos = currentPos;
			Pane::leftDown = leftDown;
		}

		agl::Vec<int, 2> windowSize;
		windowSize.x = window.getWindowAttributes().width;
		windowSize.y = window.getWindowAttributes().height;

		camera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);
		window.setViewport(0, 0, windowSize.x, windowSize.y);

		window.updateMvp(camera);
	}

	text.clearText();
	font.deleteFont();
	;
	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
