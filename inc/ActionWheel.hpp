#include "../lib/AGL/agl.hpp"

#define RADIUS 300

#define DBLUE      \
	{              \
		15, 15, 31 \
	}

#define DGRAY       \
	{               \
		31, 31, 31, \
	}

void drawTextRot(agl::RenderWindow &window, agl::Text &text, float width, agl::TextAlign align,
				 agl::Vec<float, 2> offset, float rotation)
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
		shape->setOffset(shapePosition + offset);
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

class ActionGroup
{
	public:
		Action *action;
		int		total;

		void setup(int total)
		{
			action		= new Action[total];
			this->total = total;
		}

		~ActionGroup()
		{
			delete[] action;
		}
};

class ActionWheel : public agl::Drawable
{
	public:
		agl::Circle		*circle = nullptr;
		agl::Text		*text	= nullptr;
		agl::Shape		*line	= nullptr;
		agl::Vec<int, 2> center;
		ActionGroup		*actionGroup;

		bool		 exist		  = false;
		ActionGroup *currentGroup = nullptr;

		bool			 clickEvent;
		agl::Vec<int, 2> clickPos;

		ActionWheel(agl::Circle *circle, agl::Text *text, agl::Shape *line, int actionGroups)
			: circle(circle), text(text), line(line)
		{
			actionGroup = new ActionGroup[actionGroups];
		}

		void open()
		{
			exist		 = true;
			currentGroup = &actionGroup[0];
		}

		void drawGroup(agl::RenderWindow &window, ActionGroup &group)
		{
			for (int i = 0; i < group.total; i++)
			{
				float rotation = (360 / (float)group.total) * i;

				line->setSize({0, RADIUS});
				line->setPosition(center);
				line->setRotation({0, 0, rotation});
				line->setColor(DGRAY);

				glLineWidth(3);

				window.drawShape(*line);

				glLineWidth(1);

				text->setColor(agl::Color::White);
				text->clearText();
				text->setText(group.action[i].label);
				text->setPosition(center);

				if (group.total % 2 == 0)
				{
					rotation += (360 / (float)group.total) / 2;
				}

				if (clickEvent)
				{
					float clickRotation = agl::radianToDegree(agl::Vec<float, 2>(clickPos - center).angle());
					float half			= (360 / (float)group.total) / 2;

					float lower = rotation - half;
					float upper = rotation + half;

					if(lower < 0 && clickRotation > upper)
					{
						lower += 360;
						upper = 360;
					}

					if(group.action[i].label == "split")
					{
						std::cout << '\n';
						std::cout << lower << '\n';
						std::cout << clickRotation << '\n';
					}

					if (clickRotation > lower && clickRotation < upper)
					{
						group.action[i].action();
					}
				}

				if (rotation <= 180)
				{
					drawTextRot(window, *text, 200, agl::Right, {50, 0}, rotation - 90);
				}
				else
				{
					drawTextRot(window, *text, 200, agl::Left, {-250, 0}, rotation + 90);
				}
			}
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

			drawGroup(window, *currentGroup);
		}

		~ActionWheel()
		{
			delete[] actionGroup;
		}
};
