#include "../lib/AGL/agl.hpp"

#define FONTSIZE 24
#define BORDER	 2
#define PADDING	 2

#define DARKG      \
	{              \
		15, 15, 15 \
	}

#define LIGHTERG   \
	{              \
		63, 63, 63 \
	}

enum Split
{
	Vertical,
	Horizontal,
	Root,
};

struct CursorInfo
{
		agl::Vec<float, 2> pos;
		int				   i;
};

enum Mode
{
	Insert,
	Select,
	Empty,
};

class Pane : public agl::Drawable
{
	public:
		agl::Rectangle	  *rect = nullptr;
		agl::Vec<int, 2>   size;
		agl::Vec<int, 2>   pos	 = {0, 0};
		Split			   split = Split::Root;
		float			   percent; // 1.0 means Bchild covered, 0.0 Achild covered
		Pane			  *Achild = nullptr;
		Pane			  *Bchild = nullptr;
		agl::Text		  *text	  = nullptr;
		std::string		   str	  = "";
		agl::Vec<float, 2> textCursorPos;
		int				   textCursorIndex = 0;

		struct
		{
				CursorInfo start;
				CursorInfo end;
		} selection;

		static Pane			   *focusPane;
		static bool				clickEvent;
		static std::string		keybuffer;
		static agl::Vec<int, 2> clickPos;
		static agl::Vec<int, 2> currentPos;
		static bool				leftDown;

		Mode mode = Mode::Insert;

		Pane(agl::Rectangle *rect, agl::Text *text) : rect(rect), text(text)
		{
		}

		void splitPane(Split type, float percent)
		{
			Achild = new Pane(rect, text);
			Bchild = new Pane(rect, text);

			split		  = type;
			this->percent = percent;

			return;
		}

		void updateSize(agl::RenderWindow &window)
		{
			size.x = window.getWindowAttributes().width;
			size.y = window.getWindowAttributes().height;

			return;
		}

		void drawVerticalSplit(agl::RenderWindow &window);
		void drawHorizontalSplit(agl::RenderWindow &window);
		void drawRoot(agl::RenderWindow &window);

		void processLogic();

		void drawFunction(agl::RenderWindow &window) override;

		~Pane()
		{
			delete Achild;
			delete Bchild;
		}
};
