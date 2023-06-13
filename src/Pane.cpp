#include "../inc/Pane.hpp"

Pane			*Pane::focusPane  = nullptr;
bool			 Pane::clickEvent = false;
std::string		 Pane::keybuffer  = "";
agl::Vec<int, 2> Pane::clickPos	  = {0, 0};
agl::Vec<int, 2> Pane::currentPos = {0, 0};
bool			 Pane::leftDown	  = false;

CursorInfo drawTextExtra(agl::RenderWindow &window, agl::Text &text, float width, agl::TextAlign align,
						 agl::Vec<float, 2> clickPos)
{
	CursorInfo cursorInfo = {text.getPosition(), -1};

	agl::Rectangle *shape = text.getCharBox();

	agl::Vec<float, 2> pen;

	if (align == agl::Right)
	{
		pen = {width - (text.getWidth() * text.getScale()), 0};
	}

	agl::Texture::bind(*shape->getTexture());

	for (int i = 0; i < text.getLength(); i++)
	{
		agl::Glyph *glyph = text.getGlyph(i);

		if (glyph->value == '\n' || pen.x + glyph->advance * text.getScale() > width)
		{
			pen.x = 0;
			pen.y += text.getHeight() * text.getScale();
		}

		agl::Vec<float, 3> shapeSize	 = {glyph->size.x * text.getScale(), glyph->size.y * text.getScale()};
		agl::Vec<float, 3> shapePosition = {(float)pen.x + (glyph->bearing.x * text.getScale()),
											(float)pen.y - (glyph->bearing.y * text.getScale()), 0};

		shapePosition.x += (text.getPosition().x);
		shapePosition.y += (text.getPosition().y) + (text.getHeight() * text.getScale());
		shapePosition.z = text.getPosition().z;

		shape->setSize(shapeSize);
		shape->setPosition(shapePosition);
		shape->setTextureScaling(glyph->scale);
		shape->setTextureTranslation(glyph->position);

		agl::Mat4f transform;
		agl::Mat4f translation = shape->getTranslationMatrix();
		agl::Mat4f scaling	   = shape->getScalingMatrix();

		transform = translation * scaling;

		agl::Mat4f textureTransform;
		agl::Mat4f textureTranslation = shape->getTextureTranslation();
		agl::Mat4f textureScaling	  = shape->getTextureScaling();

		textureTransform = textureTranslation * textureScaling;

		agl::Shader::setUniform(window.getTransformID(), transform);
		agl::Shader::setUniform(window.getShapeColorID(), shape->getColor().normalized());
		agl::Shader::setUniform(window.getTextureTransformID(), textureTransform);

		window.drawPrimative(shape->getShapeData());

		pen.x += glyph->advance * text.getScale();

		if (clickPos.y > (pen.y + text.getPosition().y))
		{
			if (clickPos.x > (pen.x + text.getPosition().x - (shape->getSize().x / 2)))
			{
				cursorInfo.pos = pen + text.getPosition();
				cursorInfo.i   = i;
			}
		}
	}

	return cursorInfo;
}

// returns cursor position at index
CursorInfo drawTextExtra(agl::RenderWindow &window, agl::Text &text, float width, agl::TextAlign align, int index)
{
	CursorInfo cursorInfo = {text.getPosition(), -1};

	agl::Rectangle *shape = text.getCharBox();

	agl::Vec<float, 2> pen;

	if (align == agl::Right)
	{
		pen = {width - (text.getWidth() * text.getScale()), 0};
	}

	agl::Texture::bind(*shape->getTexture());

	for (int i = 0; i < text.getLength(); i++)
	{
		agl::Glyph *glyph = text.getGlyph(i);

		if (glyph->value == '\n' || pen.x + glyph->advance * text.getScale() > width)
		{
			pen.x = 0;
			pen.y += text.getHeight() * text.getScale();
		}

		agl::Vec<float, 3> shapeSize	 = {glyph->size.x * text.getScale(), glyph->size.y * text.getScale()};
		agl::Vec<float, 3> shapePosition = {(float)pen.x + (glyph->bearing.x * text.getScale()),
											(float)pen.y - (glyph->bearing.y * text.getScale()), 0};

		shapePosition.x += (text.getPosition().x);
		shapePosition.y += (text.getPosition().y) + (text.getHeight() * text.getScale());
		shapePosition.z = text.getPosition().z;

		shape->setSize(shapeSize);
		shape->setPosition(shapePosition);
		shape->setTextureScaling(glyph->scale);
		shape->setTextureTranslation(glyph->position);

		agl::Mat4f transform;
		agl::Mat4f translation = shape->getTranslationMatrix();
		agl::Mat4f scaling	   = shape->getScalingMatrix();

		transform = translation * scaling;

		agl::Mat4f textureTransform;
		agl::Mat4f textureTranslation = shape->getTextureTranslation();
		agl::Mat4f textureScaling	  = shape->getTextureScaling();

		textureTransform = textureTranslation * textureScaling;

		agl::Shader::setUniform(window.getTransformID(), transform);
		agl::Shader::setUniform(window.getShapeColorID(), shape->getColor().normalized());
		agl::Shader::setUniform(window.getTextureTransformID(), textureTransform);

		window.drawPrimative(shape->getShapeData());

		pen.x += glyph->advance * text.getScale();

		if (i == index)
		{
			cursorInfo.pos = pen + text.getPosition();
			cursorInfo.i   = i;
		}
	}

	return cursorInfo;
}

void Pane::drawVerticalSplit(agl::RenderWindow &window)
{
	Achild->pos	   = pos;
	Achild->size.y = size.y * percent;
	Achild->size.x = size.x;

	Bchild->pos = pos;
	Bchild->pos.y += Achild->size.y;
	Bchild->size.y = size.y * (1.0 - percent);
	Bchild->size.x = size.x;

	window.draw(*Achild);
	window.draw(*Bchild);
}

void Pane::drawHorizontalSplit(agl::RenderWindow &window)
{
	Achild->pos	   = pos;
	Achild->size.x = size.x * percent;
	Achild->size.y = size.y;

	Bchild->pos = pos;
	Bchild->pos.x += Achild->size.x;
	Bchild->size.x = size.x * (1.0 - percent);
	Bchild->size.y = size.y;

	window.draw(*Achild);
	window.draw(*Bchild);
}

void Pane::processLogic()
{
	if (clickEvent)
	{
		if (clickPos.x > pos.x && clickPos.y > pos.y)
		{
			if (clickPos.x < (pos.x + size.x) && clickPos.y < (pos.y + size.y))
			{
				focusPane = this;
			}
		}
	}

	if (focusPane == this && clickEvent)
	{
		mode = Mode::Insert;
	}

	if (focusPane == this && leftDown && mode != Mode::Select)
	{
		if (currentPos.x != clickPos.x || currentPos.y != clickPos.y)
		{
			selection.start = CursorInfo();
			selection.end	= CursorInfo();

			mode = Mode::Select;
		}
	}

	if (focusPane != this)
	{
		mode = Mode::Empty;
	}

	if (focusPane == this)
	{
		if (str.length() == 0)
		{
			str += keybuffer;
		}
		else
		{
			switch (mode)
			{
				case Mode::Insert:
					if (textCursorIndex < -1)
					{
						textCursorIndex = -1;
					}

					str.insert(textCursorIndex + 1, keybuffer);
					break;
				case Mode::Select:
					if (keybuffer != "")
					{
						if (keybuffer[0] == 127 || keybuffer[0] == 8)
						{
							keybuffer = keybuffer.substr(1, keybuffer.length());
						}

						str = str.substr(0, selection.start.i + 1) + keybuffer +
							  str.substr(selection.end.i + 1, str.length() - selection.end.i + 1);

						textCursorIndex = selection.start.i;

						mode = Mode::Insert;
					}
					break;
				case Mode::Empty:
					break;
			}
		}

		int length = 0;

		for (char &c : keybuffer)
		{
			if (c == 8)
			{
				length--;
			}
			else if (c == 127)
			{
			}
			else
			{
				length++;
			}
		}

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
			else if (str[i] == 8)
			{
				str.erase(i - 1, 2);
				i -= 2;
			}
			else if (str[i] == 127)
			{
				str.erase(i, 2);
				i--;
			}
		}

		textCursorIndex += length;
	}
}

void Pane::drawRoot(agl::RenderWindow &window)
{
	// draw pane body

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
	text->setText(str);

	// draw selections / cursor & text

	CursorInfo ci;

	switch (mode)
	{
		case Mode::Insert:
			if (clickEvent)
			{
				ci = drawTextExtra(window, *text, INFINITY, agl::Left, clickPos);
			}
			else
			{
				ci = drawTextExtra(window, *text, INFINITY, agl::Left, textCursorIndex);
			}

			textCursorPos	= ci.pos;
			textCursorIndex = ci.i;

			rect->setPosition(textCursorPos);
			rect->setSize({1, FONTSIZE});
			rect->setColor(agl::Color::White);
			window.drawShape(*rect);

			break;

		case Mode::Select:
			// start fill

			rect->setPosition(selection.start.pos);

			if (selection.start.pos.y == selection.end.pos.y)
			{
				rect->setSize({selection.end.pos.x - selection.start.pos.x, FONTSIZE});
			}
			else
			{
				rect->setSize({((size.x - (selection.start.pos.x - pos.x)) - BORDER) - PADDING, FONTSIZE});
			}
			rect->setColor(LIGHTERG);
			window.drawShape(*rect);

			if (selection.start.pos.y != selection.end.pos.y)
			{
				// middle fill
				rect->setPosition({pos.x + (float)PADDING + BORDER, selection.start.pos.y + FONTSIZE});
				rect->setSize({size.x - float(2. * (PADDING + BORDER)),
							   selection.end.pos.y - (selection.start.pos.y + FONTSIZE)});
				rect->setColor(LIGHTERG);
				window.drawShape(*rect);

				// end fill

				rect->setPosition(selection.end.pos);
				rect->setSize({(pos.x - selection.end.pos.x) + BORDER + PADDING, FONTSIZE});
				rect->setColor(LIGHTERG);
				window.drawShape(*rect);
			}

			if (leftDown)
			{
				ci = drawTextExtra(window, *text, INFINITY, agl::Left, currentPos);

				selection.start = {textCursorPos, textCursorIndex};

				if (ci.i < selection.start.i)
				{
					selection.end	= selection.start;
					selection.start = ci;
				}
				else
				{
					selection.end = ci;
				}
			}
			else
			{
				window.drawText(*text);
			}
			break;

		case Mode::Empty:
			window.drawText(*text);

			break;
	}
}

void Pane::drawFunction(agl::RenderWindow &window)
{
	switch (split)
	{
		case Split::Horizontal:
			this->drawVerticalSplit(window);

			break;
		case Split::Vertical:
			this->drawHorizontalSplit(window);

			break;
		case Split::Root:
			this->processLogic();
			this->drawRoot(window);

			break;
		case Split::Base:
			Achild->size = size;
			Achild->pos = {0, 0};
			Achild->drawFunction(window);

			break;
	}
}
