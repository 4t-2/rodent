#include "./ActionWheel.hpp"
#include <functional>

#define ACTIONS 26

class ActionList : public agl::Drawable
{
	public:
		Action							  action[ACTIONS];
		std::function<void(ActionList *)> onDraw = [](auto){};
		bool							  exists;
		bool							  clickEvent;
		bool							  rightDown;
		agl::Vec<int, 2>				  clickPos;
		agl::Text						 *text;
		agl::Rectangle					 *rect;
		agl::Vec<int, 2> center;

		ActionList(agl::Text *text, agl::Rectangle *rect) : text(text), rect(rect)
		{

		}

		void drawFunction(agl::RenderWindow &window) override
		{
			if (!exists)
			{
				return;
			}

			onDraw(this);

			rect->setColor(agl::Color::White);
			rect->setPosition(center + agl::Vec<int, 2>{-RADIUS - 100, 0});
			rect->setSize({100, 100});
			rect->setOffset({});
			rect->setRotation({});

			window.drawShape(*rect);
		}

		void open()
		{
			exists = true;
		}
};
