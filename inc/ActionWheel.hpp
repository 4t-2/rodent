#include "../lib/AGL/agl.hpp"

#define ACTIONS 3
#define RADIUS 300

#define DBLUE       \
	{               \
		15, 15, 31, \
	}

#define DGRAY \
	{               \
		31, 31, 31, \
	}

class ActionWheel : public agl::Drawable
{
	public:
		agl::Circle		*circle = nullptr;
		agl::Text		*text	= nullptr;
		agl::Shape		*line	= nullptr;
		agl::Vec<int, 2> center;

		bool exist = false;

		ActionWheel(agl::Circle *circle, agl::Text *text, agl::Shape *line) : circle(circle), text(text), line(line)
		{
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			if (!exist)
			{
				return;
			}

			circle->setPosition(center);
			circle->setSize({RADIUS, RADIUS});
			circle->setColor(DBLUE);

			window.drawShape(*circle);

			for(int i = 0; i < ACTIONS; i++)
			{
				line->setSize({0, RADIUS});
				line->setPosition(center);
				line->setRotation({0, 0, (360 / (float)ACTIONS) * i});
				line->setColor(DGRAY);

				glLineWidth(3);

				window.drawShape(*line);
			}
				glLineWidth(1);
		}
};
