#include "../inc/Context.hpp"

template <typename T> bool pointInBox(agl::Vec<T, 2> point, agl::Vec<T, 2> size, agl::Vec<T, 2> pos)
{
	if (point.x > pos.x && point.x < pos.x + size.x)
	{
		if (point.y > pos.y && point.y < pos.y + size.y)
		{
			return true;
		}
	}

	return false;
}

void Context::drawFunction(agl::RenderWindow &window)
{
	if (exists)
	{
		if (leftDown)
		{
			exists = false;
		}

		agl::Vec<int, 2> mousePos = event->getPointerWindowPosition();

		rect->setSize({width, 24 * CONTEXTACTIONS});
		rect->setColor(BACK);
		rect->setPosition(pos);

		window.drawShape(*rect);

		for (int i = 0; i < CONTEXTACTIONS; i++)
		{
			text->clearText();
			text->setText(actions[i].label);
			text->setPosition(pos + agl::Vec<int, 2>{0, 24 * i});

			window.drawText(*text);

			if (leftDown && pointInBox(mousePos, {(int)width, 24}, pos + agl::Vec<int, 2>{0, 24 * i}))
			{
				actions[i].action();
			}
		}
	}
}
