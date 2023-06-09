#include "../lib/AGL/agl.hpp"

#define ACTIONS 6
#define RADIUS	300

#define DBLUE      \
	{              \
		15, 15, 31 \
	}

#define DGRAY       \
	{               \
		31, 31, 31, \
	}

void drawTextRot(agl::RenderWindow &window, agl::Text &text, float width, agl::TextAlign align, float rotation)
{
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
		agl::Vec<float, 3> shapePosition = {
			(float)pen.x + (glyph->bearing.x * text.getScale()),
			(float)pen.y - (glyph->bearing.y * text.getScale()) + (text.getHeight() * text.getScale() * (float).5), 0};

		agl::Vec<float, 3> spacePos;

		spacePos.x += (text.getPosition().x);
		spacePos.y += (text.getPosition().y);
		spacePos.z = text.getPosition().z;

		shape->setRotation({0, 0, -rotation});
		shape->setSize(shapeSize);
		shape->setPosition(spacePos);
		shape->setOffset(shapePosition);
		shape->setTextureScaling(glyph->scale);
		shape->setTextureTranslation(glyph->position);

		window.drawShape(*shape);

		pen.x += glyph->advance * text.getScale();
	}
}

class Action
{
	public:
		std::string			  label;
		std::function<void()> action;
};

class ActionWheel : public agl::Drawable
{
	public:
		agl::Circle		*circle = nullptr;
		agl::Text		*text	= nullptr;
		agl::Shape		*line	= nullptr;
		agl::Vec<int, 2> center;
		Action			 action[ACTIONS];

		bool exist = false;

		std::function<void(agl::RenderWindow &window)> rootDraw = [&](agl::RenderWindow &window) {
			if (!exist)
			{
				return;
			}

			circle->setPosition(center);
			circle->setSize({RADIUS, RADIUS});
			circle->setColor(DBLUE);

			window.drawShape(*circle);

			// this is bad code
			// i might fix it
			// probably not

			for (int i = 0; i < ACTIONS; i++)
			{
				line->setSize({0, RADIUS});
				line->setPosition(center);
				line->setRotation({0, 0, (360 / (float)ACTIONS) * i});
				line->setColor(DGRAY);

				glLineWidth(3);

				window.drawShape(*line);

				text->setColor(agl::Color::White);
				text->clearText();
				text->setText(action[i].label);

				agl::Vec<float, 2> pos;

				if (ACTIONS % 2 == 0)
				{
					pos = agl::pointOnCircle(((PI * 2 / (float)ACTIONS) * (float)i) - ((PI * 2 / (float)ACTIONS)));
				}
				else
				{
					pos = agl::pointOnCircle(((PI * 2 / (float)ACTIONS) * (float)i) - PI / 2);
				}

				pos *= 50;
				text->setPosition(center + pos);

				float offset;

				if (ACTIONS % 2 == 0)
				{
					offset = (360. / ACTIONS);
				}
				else
				{
					offset = 90;
				}

				drawTextRot(window, *text, 200, agl::Left, (360 / ((float)ACTIONS) * i) - offset);

				// if (i <= (ACTIONS / 2))
				// {
				// 	pos *= 50;
				// 	text->setPosition(center);
				//
				// 	drawTextRot(window, *text, 200, agl::Left, (360 /
				// ((float)ACTIONS) * i) - offset);
				// }
				// else
				// {
				// 	pos *= 250;
				// 	text->setPosition(center);
				//
				// 	drawTextRot(window, *text, 200, agl::Right, ((360 /
				// ((float)ACTIONS) * i) + offset));
				// }
			}
			glLineWidth(1);
		};

		std::function<void(agl::RenderWindow &window)> currentDraw;

		ActionWheel(agl::Circle *circle, agl::Text *text, agl::Shape *line) : circle(circle), text(text), line(line)
		{
		}

		void open()
		{
			currentDraw = rootDraw;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			rootDraw(window);
		}
};
