#include "../lib/AGL/agl.hpp"
#include <functional>

#define BACK       \
	{              \
		31, 31, 31 \
	}

#define CONTEXTACTIONS 4

class ContextAction
{
	public:
		std::string			  label = "";
		std::function<void()> action;

		ContextAction()
		{
		}

		ContextAction(std::string label, std::function<void()> action) : label(label), action(action)
		{
		}
};

class Context : public agl::Drawable
{
	public:
		agl::Rectangle	*rect  = nullptr;
		float			 width = 200;
		agl::Vec<int, 2> pos   = {0, 0};
		ContextAction	 actions[CONTEXTACTIONS];
		agl::Event		*event;
		agl::Text		*text;

		bool exists	  = false;
		bool leftDown = false;
		bool rising	  = false;

		Context(agl::Rectangle *rect, agl::Text *text, agl::Event *event) : rect(rect), text(text), event(event)
		{
		}

		void create()
		{
			exists = true;
			pos	   = event->getPointerWindowPosition();
		}

		void drawFunction(agl::RenderWindow &window) override;
};
