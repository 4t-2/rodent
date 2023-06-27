#include "./ActionWheel.hpp"
#include <functional>

#define ACTIONS	   26
#define LISTOFFSET 10
#define ITEMSIDE   50
#define LISTBORDER 1
#define LISTPADDING 10

class ListAction
{
	public:
		std::string			  label;
		std::function<void()> action;
};

class ActionList : public agl::Drawable
{
	public:
		ListAction							  action[ACTIONS];
		std::function<void(ActionList *)> onDraw = [](auto) {};
		bool							  exists;
		bool							  clickEvent;
		bool							  rightDown;
		agl::Vec<int, 2>				  clickPos;
		agl::Text						 *text;
		agl::Rectangle					 *rect;
		agl::Vec<int, 2>				  center;

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

			rect->setOffset({});
			rect->setRotation({});

			for (int i = 0; i < ACTIONS; i++)
			{
				agl::Vec<float, 2> pos = {float(ACTIONS * ITEMSIDE * -.5) + (ITEMSIDE * i), -RADIUS - 100 - LISTOFFSET};
				pos += center;

				if (clickEvent)
				{
					if (clickPos.x > pos.x && clickPos.x < pos.x + ITEMSIDE)
					{
						if (clickPos.y > pos.y && clickPos.y < pos.y + ITEMSIDE)
						{
							action[i].action();
						}
					}
				}

				rect->setSize({ITEMSIDE, ITEMSIDE});
				rect->setPosition(pos);
				rect->setColor(DGRAY);

				window.drawShape(*rect);

				pos += {LISTBORDER, LISTBORDER};

				rect->setSize({ITEMSIDE - LISTBORDER, ITEMSIDE - LISTBORDER * 2});
				rect->setPosition(pos);
				rect->setColor(DBLUE);

				window.drawShape(*rect);

				pos += {LISTPADDING, LISTPADDING};

				text->clearText();
				text->setColor(agl::Color::White);
				text->setText(action[i].label);
				text->setPosition(pos);
				
				window.drawText(*text);
			}
		}

		void open()
		{
			exists = true;
		}
};
