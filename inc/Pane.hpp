#include "../lib/AGL/agl.hpp"
#include <filesystem>
#include <fstream>

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
	Horizontal,
	Vertical,
	Root,
	Base,
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

enum PaneType
{
	TextEditor,
	FileBrowser
};

class Pane : public agl::Drawable
{
	public:
		agl::Rectangle	  *rect = nullptr;
		agl::Vec<int, 2>   size;
		agl::Vec<int, 2>   pos			   = {0, 0};
		Split			   split		   = Split::Root;
		float			   percent		   = 0; // 1.0 means Bchild covered, 0.0 Achild covered
		Pane			  *Achild		   = nullptr;
		Pane			  *Bchild		   = nullptr;
		Pane			  *parent		   = nullptr;
		agl::Text		  *text			   = nullptr;
		std::string		   str			   = "";
		agl::Vec<float, 2> textCursorPos   = {0, 0};
		int				   textCursorIndex = 0;
		PaneType		   paneType		   = PaneType::FileBrowser;
		std::string		   path			   = "./";

		struct
		{
				CursorInfo start = {{0, 0}, 0};
				CursorInfo end	 = {{0, 0}, 0};
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
			Pane::focusPane = this;

			split = Split::Base;

			Achild = new Pane(rect, text, this);
		}

		Pane(agl::Rectangle *rect, agl::Text *text, Pane *parent) : rect(rect), text(text), parent(parent)
		{
			Pane::focusPane = this;
		}

		void splitPane(Split type, float percent)
		{
			Achild = new Pane(rect, text, this);
			Bchild = new Pane(rect, text, this);

			split		  = type;
			this->percent = percent;

			// yes i know this is bad
			// no i dont care

			Achild->str = str;
			str			= "";

			return;
		}

		static void closePane(Pane *pane)
		{
			if (pane->parent->split == Split::Base)
			{
				return;
			}

			auto snipPaneTree = [](Pane *newRoot) {
				Pane *parent	  = newRoot->parent;
				Pane *superParent = parent->parent;

				newRoot->parent = superParent;

				if (superParent->Achild == parent)
				{
					superParent->Achild = newRoot;
				}
				if (superParent->Bchild == parent)
				{
					superParent->Bchild = newRoot;
				}
			};

			if (pane->parent->Achild == pane) // a pane
			{
				snipPaneTree(pane->parent->Bchild);
				Pane::focusPane		 = pane->parent->Bchild;
				pane->parent->Bchild = nullptr;
			}
			else // b pane
			{
				snipPaneTree(pane->parent->Achild);
				Pane::focusPane		 = pane->parent->Achild;
				pane->parent->Achild = nullptr;
			}

			delete pane->parent;
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
		void drawBrowser(agl::RenderWindow &window);

		void processRootLogic();
		void processBrowserLogic();

		void open(std::string path)
		{
			this->path = path;

			if (std::filesystem::is_directory(path))
			{
				paneType = PaneType::FileBrowser;

				str = "";
			}
			else
			{
				paneType = PaneType::TextEditor;

				str = "";

				std::fstream fs(path, std::ios::in);

				while (true)
				{
					char c = fs.get();
					if (c != -1)
					{
						str += c;
					} else {
						break;
					}
				}
			}
		}

		void drawFunction(agl::RenderWindow &window) override;

		~Pane()
		{
			delete Achild;
			delete Bchild;
		}
};
