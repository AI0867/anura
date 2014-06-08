#pragma once

#include <set>
#include "widget.hpp"

namespace gui 
{
	class layout_widget : public widget
	{
	public:
		enum LayoutType {
			ABSOLUTE_LAYOUT,
			RELATIVE_LAYOUT,
		};

		layout_widget(const variant& v, game_logic::FormulaCallable* e);
		virtual ~layout_widget();

		std::vector<WidgetPtr> getChildren() const;

		void reflow_children();
	protected:
		variant handleWrite();
		void recalcLoc();
		void handleDraw() const;
		bool handleEvent(const SDL_Event& event, bool claimed);
	private:
		DECLARE_CALLABLE(layout_widget);

		void visitValues(game_logic::FormulaCallableVisitor& visitor);

		LayoutType layout_type_;

		// If width is specified then we keep a track of it here.
		int fixed_width_;
		// If height is specified then we keep a track of it here.
		int fixed_height_;

		typedef std::set<WidgetPtr, WidgetSortZOrder> widget_list;
		widget_list children_;
	};
	typedef boost::intrusive_ptr<layout_widget> layout_WidgetPtr;
}
