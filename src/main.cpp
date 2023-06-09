// this is very bad and ugly code

#include "../lib/AGL/agl.hpp"
#include "../lib/IntNet/intnet.hpp"
#include <fstream>

#include "../inc/ActionList.hpp"
#include "../inc/ActionWheel.hpp"
#include "../inc/Context.hpp"
#include "../inc/Listener.hpp"
#include "../inc/Pane.hpp"
#include "../inc/TextPredictor.hpp"

long currentMilli()
{
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	long long milliseconds = spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
	return (long)milliseconds;
}

void printGenText(TextPredictor *tp)
{
	std::string newText = "starting";

	for (int i = 0; i < 500 && newText[newText.length() - 1] != 0; i++)
	{
		newText += tp->predict(newText.substr(newText.length() - CHARBUFFERSIZE, CHARBUFFERSIZE));
	}
	std::cout << newText << '\n';
}

void train()
{
	TextPredictor tp;

	std::fstream trainingStream("training.txt", std::ios::in);

	std::string text = "";

	while (true)
	{
		char c = trainingStream.get();

		if (c != -1)
		{
			text += c;
		}
		else
		{
			break;
		}
	}

	std::cout << text << '\n';

	for (int i = 0; i < text.length() - (CHARBUFFERSIZE - 1); i++)
	{
		float error = tp.train(text.substr(i, CHARBUFFERSIZE), text[CHARBUFFERSIZE + i]);

		if (i % 100 == 0)
		{
			std::cout << error << '\n';
			std::cout << i << " / " << text.length() - CHARBUFFERSIZE << '\n' << '\n';
		}
	}

	std::fstream fs("modelstruct", std::ios::out);

	fs << ((in::NetworkStructure *)&tp.network->structure)->serialize();

	fs.close();
	trainingStream.close();

	printGenText(&tp);

	return;
}

TextPredictor *loadModel()
{
	std::fstream fs("modelstruct", std::ios::in);

	std::string structure = "";
	for (char c; fs.get(c); structure.append(1, c))
		;

	fs.close();

	in::NetworkStructure ns((unsigned char *)structure.c_str());

	in::NeuralNetwork *nn = new in::NeuralNetwork(ns);

	TextPredictor *tp = new TextPredictor(nn);

	return tp;
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

	agl::Vec<int, 2> clickPos;
	agl::Vec<int, 2> currentPos;
	bool			 clickEvent;
	bool			 leftDown;
	bool			 rightDown;

	Listener mouseListener(
		[&]() {
			clickPos   = event.getPointerWindowPosition();
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

	// QWERTYUIOPASDFGHJKLZXCVBNM
	// qwertyuiopasdfghjklzxcvbnm
	// 1234567890

	ActionWheel mainWheel(&circle, &text, &line);

	ActionGroup startGroup(6, false);
	ActionGroup splitGroup(3, false);
	ActionGroup letterGroup(26, true);
	ActionGroup mathGroup(20, false);
	ActionGroup seperatorGroup(6, true);
	ActionGroup miscGroup(6, true);

	startGroup.action[0] = {"Split", [&](ActionWheel *actionWheel) { actionWheel->currentGroup = &splitGroup; }};
	startGroup.action[1] = {"Letter", [&](ActionWheel *actionWheel) { actionWheel->currentGroup = &letterGroup; }};
	startGroup.action[2] = {"Math", [&](ActionWheel *actionWheel) { actionWheel->currentGroup = &mathGroup; }};
	startGroup.action[3] = {"Seperators",
							[&](ActionWheel *actionWheel) { actionWheel->currentGroup = &seperatorGroup; }};
	startGroup.action[4] = {"Misc", [&](ActionWheel *actionWheel) { actionWheel->currentGroup = &miscGroup; }};
	startGroup.action[5] = {"Browse", [&](auto) { Pane::focusPane->open("./"); }};

	splitGroup.action[0] = {"Vertical", [&](auto) { Pane::focusPane->splitPane(Split::Vertical, .5); }};
	splitGroup.action[1] = {"Horizontal", [&](auto) { Pane::focusPane->splitPane(Split::Horizontal, .5); }};
	splitGroup.action[2] = {"Close", [&](auto) { Pane::closePane(Pane::focusPane); }};

	for (int i = 0; i < 26; i++)
	{
		std::string str = "A";
		str[0] += i;

		letterGroup.modAction[i] = {str, [=](auto) { Pane::keybuffer += str; }};
	}

	for (int i = 0; i < 26; i++)
	{
		std::string str = "a";
		str[0] += i;

		letterGroup.action[i] = {str, [=](auto) { Pane::keybuffer += str; }};
	}

	mathGroup.action[0]	 = {"%", [&](auto) { Pane::keybuffer += "%"; }};
	mathGroup.action[1]	 = {"*", [&](auto) { Pane::keybuffer += "*"; }};
	mathGroup.action[2]	 = {"+", [&](auto) { Pane::keybuffer += "+"; }};
	mathGroup.action[3]	 = {"-", [&](auto) { Pane::keybuffer += "-"; }};
	mathGroup.action[4]	 = {".", [&](auto) { Pane::keybuffer += "."; }};
	mathGroup.action[5]	 = {"/", [&](auto) { Pane::keybuffer += "/"; }};
	mathGroup.action[6]	 = {"0", [&](auto) { Pane::keybuffer += "0"; }};
	mathGroup.action[7]	 = {"1", [&](auto) { Pane::keybuffer += "1"; }};
	mathGroup.action[8]	 = {"2", [&](auto) { Pane::keybuffer += "2"; }};
	mathGroup.action[9]	 = {"3", [&](auto) { Pane::keybuffer += "3"; }};
	mathGroup.action[10] = {"4", [&](auto) { Pane::keybuffer += "4"; }};
	mathGroup.action[11] = {"5", [&](auto) { Pane::keybuffer += "5"; }};
	mathGroup.action[12] = {"6", [&](auto) { Pane::keybuffer += "6"; }};
	mathGroup.action[13] = {"7", [&](auto) { Pane::keybuffer += "7"; }};
	mathGroup.action[14] = {"8", [&](auto) { Pane::keybuffer += "8"; }};
	mathGroup.action[15] = {"9", [&](auto) { Pane::keybuffer += "9"; }};
	mathGroup.action[16] = {"<", [&](auto) { Pane::keybuffer += "<"; }};
	mathGroup.action[17] = {"=", [&](auto) { Pane::keybuffer += "="; }};
	mathGroup.action[18] = {">", [&](auto) { Pane::keybuffer += ">"; }};
	mathGroup.action[19] = {"^", [&](auto) { Pane::keybuffer += "^"; }};

	// seperatorGroup.action[0]  = {"\"", [&](auto) { Pane::keybuffer += "\""; }};
	// seperatorGroup.action[1]  = {"'", [&](auto) { Pane::keybuffer += "'"; }};
	// seperatorGroup.action[2]  = {"(", [&](auto) { Pane::keybuffer += "("; }};
	// seperatorGroup.action[3]  = {")", [&](auto) { Pane::keybuffer += ")"; }};
	// seperatorGroup.action[4]  = {",", [&](auto) { Pane::keybuffer += ","; }};
	// seperatorGroup.action[5]  = {";", [&](auto) { Pane::keybuffer += ";"; }};
	// seperatorGroup.action[6]  = {"[", [&](auto) { Pane::keybuffer += "["; }};
	// seperatorGroup.action[7]  = {"]", [&](auto) { Pane::keybuffer += "]"; }};
	// seperatorGroup.action[8]  = {"{", [&](auto) { Pane::keybuffer += "{"; }};
	// seperatorGroup.action[9]  = {"}", [&](auto) { Pane::keybuffer += "}"; }};
	// seperatorGroup.action[10] = {"Space", [&](auto) { Pane::keybuffer += " ";
	// }}; seperatorGroup.action[11] = {"Return", [&](auto) { Pane::keybuffer +=
	// "\n"; }};
	//
	// miscGroup.action[0]	 = {"!", [&](auto) { Pane::keybuffer += "!"; }};
	// miscGroup.action[1]	 = {"#", [&](auto) { Pane::keybuffer += "#"; }};
	// miscGroup.action[2]	 = {"$", [&](auto) { Pane::keybuffer += "$"; }};
	// miscGroup.action[3]	 = {"&", [&](auto) { Pane::keybuffer += "&"; }};
	// miscGroup.action[4]	 = {":", [&](auto) { Pane::keybuffer += ":"; }};
	// miscGroup.action[5]	 = {"?", [&](auto) { Pane::keybuffer += "?"; }};
	// miscGroup.action[6]	 = {"@", [&](auto) { Pane::keybuffer += "@"; }};
	// miscGroup.action[7]	 = {"\\", [&](auto) { Pane::keybuffer += "\\";
	// }}; miscGroup.action[8]	 = {"_", [&](auto) { Pane::keybuffer += "_"; }};
	// miscGroup.action[9]	 = {"|", [&](auto) { Pane::keybuffer += "|"; }};
	// miscGroup.action[10] = {"~", [&](auto) { Pane::keybuffer += "~"; }};
	// miscGroup.action[11] = {"Backspace", [&](auto) { Pane::keybuffer += 8; }};

	seperatorGroup.action[0]	= {"\"", [&](auto) { Pane::keybuffer += "\""; }};
	seperatorGroup.modAction[0] = {"'", [&](auto) { Pane::keybuffer += "'"; }};
	seperatorGroup.action[1]	= {"(", [&](auto) { Pane::keybuffer += "("; }};
	seperatorGroup.modAction[1] = {")", [&](auto) { Pane::keybuffer += ")"; }};
	seperatorGroup.action[2]	= {"[", [&](auto) { Pane::keybuffer += "["; }};
	seperatorGroup.modAction[2] = {"]", [&](auto) { Pane::keybuffer += "]"; }};
	seperatorGroup.action[3]	= {"{", [&](auto) { Pane::keybuffer += "{"; }};
	seperatorGroup.modAction[3] = {"}", [&](auto) { Pane::keybuffer += "}"; }};
	seperatorGroup.action[4]	= {";", [&](auto) { Pane::keybuffer += ";"; }};
	seperatorGroup.modAction[4] = {",", [&](auto) { Pane::keybuffer += ","; }};
	seperatorGroup.action[5]	= {"Space", [&](auto) { Pane::keybuffer += " "; }};
	seperatorGroup.modAction[5] = {"Return", [&](auto) { Pane::keybuffer += "\n"; }};

	miscGroup.action[0]	   = {"!", [&](auto) { Pane::keybuffer += "!"; }};
	miscGroup.modAction[0] = {"?", [&](auto) { Pane::keybuffer += "?"; }};
	miscGroup.action[1]	   = {"#", [&](auto) { Pane::keybuffer += "#"; }};
	miscGroup.modAction[1] = {"$", [&](auto) { Pane::keybuffer += "$"; }};
	miscGroup.action[2]	   = {"&", [&](auto) { Pane::keybuffer += "&"; }};
	miscGroup.modAction[2] = {"@", [&](auto) { Pane::keybuffer += "@"; }};
	miscGroup.action[3]	   = {":", [&](auto) { Pane::keybuffer += ":"; }};
	miscGroup.modAction[3] = {"_", [&](auto) { Pane::keybuffer += "_"; }};
	miscGroup.action[4]	   = {"|", [&](auto) { Pane::keybuffer += "|"; }};
	miscGroup.modAction[4] = {"\\", [&](auto) { Pane::keybuffer += "\\"; }};
	miscGroup.action[5]	   = {"Backspace", [&](auto) { Pane::keybuffer += 8; }};
	miscGroup.modAction[5] = {"~", [&](auto) { Pane::keybuffer += "~"; }};

	TextPredictor *tp = loadModel();

	ActionList actionList(&text, &rect);

	actionList.onDraw = [&](auto) {
		if (Pane::focusPane->paneType == PaneType::FileBrowser)
		{
			for (ListAction &action : actionList.action)
			{
				action = {"", []() {}};
			}

			return;
		}

		std::string str = Pane::focusPane->str;

		std::string newStr = "";

		int max = 0;

		for (int i = Pane::focusPane->textCursorIndex; i >= 0 && newStr.length() < CHARBUFFERSIZE; i--)
		{
			max++;
			char c = str[i];
			if (c >= 97 && c <= 122)
			{
				newStr = c + newStr;
			}
			else if (c >= 65 && c <= 90)
			{
				newStr = char(c + 32) + newStr;
			}
		}

		if (newStr.length() < CHARBUFFERSIZE)
		{
			for (ListAction &action : actionList.action)
			{
				action = {"", []() {}};
			}

			return;
		}

		std::cout << newStr << '\n';

		tp->predict(newStr.substr(newStr.length() - CHARBUFFERSIZE, CHARBUFFERSIZE));

		for (int i = 0; i < 26; i++)
		{
			int mod = 0;

			if (actionList.rightDown)
			{
				mod = -32;
			}

			actionList.action[i].label	= std::string() + char(tp->rank[i].c + mod);
			actionList.action[i].action = [=]() { Pane::keybuffer += std::string() + char((&(tp->rank[i]))->c + mod); };
		}
	};

	actionList.action[0] = {"a", []() { std::cout << "ad" << '\n'; }};

	while (!event.windowClose())
	{
		event.poll();

		window.clear();

		pane.updateSize(window);
		mainWheel.center.x = window.getWindowAttributes().width / 2;
		mainWheel.center.y = window.getWindowAttributes().height / 2;
		actionList.center  = mainWheel.center;

		window.draw(pane);

		Pane::keybuffer = "";

		window.draw(context);
		window.draw(mainWheel);
		window.draw(actionList);

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

		if (event.isPointerButtonPressed(Button2Mask))
		{
			if (mainWheel.exist == false)
			{
				mainWheel.open(&startGroup);
				actionList.open();
			}
		}
		else
		{
			mainWheel.exist	  = false;
			actionList.exists = false;
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
		leftDown   = event.isPointerButtonPressed(Button1Mask);
		rightDown  = event.isPointerButtonPressed(Button3Mask);

		if (event.isPointerButtonPressed(Button3Mask) && !mainWheel.exist)
		{
			context.create();
		}

		if (!context.exists && !mainWheel.exist)
		{
			Pane::clickPos	 = clickPos;
			Pane::clickEvent = clickEvent;
			Pane::currentPos = currentPos;
			Pane::leftDown	 = leftDown;
		}

		if (!mainWheel.exist)
		{
			context.leftDown = leftDown;
		}

		mainWheel.clickEvent = clickEvent;
		mainWheel.clickPos	 = clickPos;
		mainWheel.rightDown	 = rightDown;

		actionList.clickEvent = clickEvent;
		actionList.rightDown  = rightDown;
		actionList.clickPos	  = clickPos;

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

	delete tp;

	return 0;
}
