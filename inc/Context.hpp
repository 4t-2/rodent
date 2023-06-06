#include "../lib/AGL/agl.hpp"

#define BACK       \
	{              \
		31, 31, 31 \
	}

class Context : public agl::Drawable
{
	public:
		agl::Rectangle	*rect = nullptr;
		agl::Vec<int, 2> size = {200, 100};
		agl::Vec<int, 2> pos  = {0, 0};
		std::string		 str  = "Cut\nCopy\nPaste\nDelete";
		agl::Event		*event;
		agl::Text		*text;

		bool exists	  = false;
		bool leftDown = false;
		bool rising	  = false;

		std::string *keyBuffer;

		Context(agl::Rectangle *rect, agl::Text *text, agl::Event *event, std::string *keyBuffer) : rect(rect), text(text), event(event), keyBuffer(keyBuffer)
		{
		}

		void create()
		{
			exists = true;
			pos	   = event->getPointerWindowPosition();
		}

		void drawFunction(agl::RenderWindow &window) override;
};
