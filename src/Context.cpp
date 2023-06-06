#include "../inc/Context.hpp"

void Context::drawFunction(agl::RenderWindow &window)
{
	if (exists)
	{
		leftDown = event->isPointerButtonPressed(Button1Mask);

		if (leftDown)
		{
			exists = false;
		}

		agl::Vec<int, 2> mousePos = event->getPointerWindowPosition();

		if (mousePos.x > pos.x && mousePos.x < pos.x + size.x)
		{
			if (mousePos.y > pos.y && mousePos.y < pos.y + size.y)
			{
				if (leftDown)
				{
					*keyBuffer += 8;
					std::cout << "fcesd" << '\n';
				}
			}
		}

		rect->setSize(size);
		rect->setColor(BACK);
		rect->setPosition(pos);

		window.drawShape(*rect);

		text->clearText();
		text->setText(str);
		text->setPosition(pos);

		window.drawText(*text);
	}
}
