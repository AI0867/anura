/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GRID_WIDGET_HPP_INCLUDED
#define GRID_WIDGET_HPP_INCLUDED

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "grid_widget_fwd.hpp"
#include "scrollable_widget"
#include "widget.hpp"

namespace gui {

class dropdown_widget;

class grid : public ScrollableWidget
{
public:
	typedef boost::function<void (int)> callback_type;
	enum COLUMN_ALIGN { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

	explicit grid(int ncols);
	explicit grid(const variant& v, game_logic::FormulaCallable* e);
	virtual ~grid() {}
	grid& set_show_background(bool val) {
		show_background_ = val;
		return *this;
	}
	virtual void setDim(int w, int h);
	void add_row(const std::vector<WidgetPtr>& widgets);

	grid& add_col(const std::string& str);
	grid& add_col(const WidgetPtr& widget=WidgetPtr());

	grid& finish_row();

	grid& set_col_width(int col, int width);
	grid& set_align(int col, COLUMN_ALIGN align);
	grid& set_hpad(int pad);
	void reset_contents(const variant&);
	void set_header_row(int row) { header_rows_.push_back(row); }

	void set_draw_selection_highlight(bool val=true) { draw_selection_highlight_ = val; }
	void set_default_selection(int value) { default_selection_ = value; }
	int get_default_selection() const { return default_selection_; }
	void allow_selection(bool val=true) { allow_selection_ = val; }
	void must_select(bool val=true, int nrow=0) { must_select_ = val; selected_row_ = nrow; }
	bool has_must_select() const { return must_select_; }
	void swallow_clicks(bool val=true) { swallow_clicks_ = val; }
	int selection() const { return selected_row_; }
	void register_mouseover_callback(callback_type cb);
	void register_selection_callback(callback_type cb);
	void register_row_selection_callback(boost::function<void()> cb);

	void set_max_height(int amount) { max_height_ = amount; }

	void on_set_yscroll(int old_value, int value);

	void allow_draw_highlight(bool val=true) { allow_highlight_ = val; }

	bool hasFocus() const;
	virtual WidgetPtr getWidgetById(const std::string& id);
	virtual ConstWidgetPtr getWidgetById(const std::string& id) const;

	virtual std::vector<WidgetPtr> getChildren() const;
protected:
	virtual bool handleEvent(const SDL_Event& event, bool claimed);
	virtual void handleDraw() const;
	virtual void handleProcess();

private:
	DECLARE_CALLABLE(grid);

	int row_at(int x, int y) const;
	void recalculate_dimensions();

	void visitValues(game_logic::FormulaCallableVisitor& visitor);

	int nrows() const { return cells_.size()/ncols_; }
	int ncols_;
	std::vector<WidgetPtr> cells_;
	std::vector<WidgetPtr> visible_cells_;
	std::vector<int> col_widths_;
	std::vector<COLUMN_ALIGN> col_aligns_;
	std::vector<int> header_rows_;
	int row_height_;
	int selected_row_;
	bool allow_selection_;
	bool must_select_;
	bool swallow_clicks_;
	bool allow_highlight_;
	int default_selection_;
	bool draw_selection_highlight_;

	// Explicitly set dimensions
	int set_w_;
	int set_h_;

	std::vector<WidgetPtr> new_row_;
	std::vector<boost::function<void()> > row_callbacks_;
	callback_type on_mouseover_;
	callback_type on_select_;
	int hpad_, vpad_;
	bool show_background_;

	int max_height_;

	void select_delegate(int selection);
	void mouseover_delegate(int selection);

	game_logic::formula_ptr ffl_on_select_;
	game_logic::formula_ptr ffl_on_mouseover_;
	game_logic::FormulaCallablePtr select_arg_;
	game_logic::FormulaCallablePtr mouseover_arg_;

	friend class dropdown_widget;
};

typedef boost::intrusive_ptr<grid> grid_ptr;
typedef boost::intrusive_ptr<const grid> const_grid_ptr;

int show_grid_as_context_menu(grid_ptr grid, WidgetPtr draw_widget);
int show_grid_as_context_menu(grid_ptr grid, const std::vector<WidgetPtr> draw_widgets);

}

#endif
