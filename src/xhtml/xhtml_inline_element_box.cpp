/*
	Copyright (C) 2003-2013 by Kristina Simpson <sweet.kristas@gmail.com>

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include "xhtml_inline_element_box.hpp"
#include "xhtml_layout_engine.hpp"

namespace xhtml
{
	// This encapsulates a replaced inline element, non-replaced inline elements are
	// dealt with elsewhere

	InlineElementBox::InlineElementBox(const BoxPtr& parent, const StyleNodePtr& node, const RootBoxPtr& root)
		: Box(BoxId::INLINE_ELEMENT, parent, node, root)
	{
	}

	void InlineElementBox::handleLayout(LayoutEngine& eng, const Dimensions& containing)
	{
		auto node = getNode();
		ASSERT_LOG(node != nullptr && node->isReplaced(), "InlineElementBox was generated for an empty node, or node which isn't replacable.");

		// set dimensions from the replaced element information.
		setContentWidth(node->getDimensions().w() * LayoutEngine::getFixedPointScale());
		setContentHeight(node->getDimensions().h() * LayoutEngine::getFixedPointScale());

		// override element dimensions from css if nescessary.
		auto css_width = getStyleNode()->getWidth();
		auto css_height = getStyleNode()->getHeight();
		if(!css_width->isAuto()) {
			setContentWidth(css_width->getLength().compute(containing.content_.width));
		}
		if(!css_height->isAuto()) {
			setContentHeight(css_height->getLength().compute(containing.content_.height));
		}
		if(!css_width->isAuto() || !css_height->isAuto()) {
			node->setDimensions(rect(0, 0,
				getDimensions().content_.width/LayoutEngine::getFixedPointScale(),
				getDimensions().content_.height/LayoutEngine::getFixedPointScale()));
		}

		// XXX we should have a default 300px width here if everything else fails.
		// or width of the largest rectangle that has a 2:1 ratio.
	}

	void InlineElementBox::layoutWidth(const Dimensions& containing)
	{
		RenderContext& ctx = RenderContext::get();
		const FixedPoint containing_width = containing.content_.width;

		auto css_width = getStyleNode()->getWidth();
		FixedPoint width = 0;
		if(!css_width->isAuto()) {
			width = css_width->getLength().compute(containing_width);
			setContentWidth(width);
		}

		calculateHorzMPB(containing_width);
		auto css_margin_left = getStyleNode()->getMargin()[static_cast<int>(css::Side::LEFT)];
		auto css_margin_right = getStyleNode()->getMargin()[static_cast<int>(css::Side::RIGHT)];

		FixedPoint total = getMBPWidth() + width;

		if(!css_width->isAuto() && total > containing.content_.width) {
			if(css_margin_left->isAuto()) {
				setMarginLeft(0);
			}
			if(css_margin_right->isAuto()) {
				setMarginRight(0);
			}
		}

		// If negative is overflow.
		FixedPoint underflow = containing.content_.width - total;

		if(css_width->isAuto()) {
			setContentWidth(underflow);
		}
	}

	void InlineElementBox::handlePreChildLayout(LayoutEngine& eng, const Dimensions& containing)
	{
		layoutWidth(containing);
		calculateVertMPB(containing.content_.height);
	}

	std::string InlineElementBox::toString() const
	{
		std::ostringstream ss;
		auto node = getNode();
		ss << "InlineElementBox: " << getDimensions().content_ << "; " << node->getActiveRect();
		return ss.str();
	}

	void InlineElementBox::handleRender(const KRE::SceneTreePtr& scene_tree, const point& offset) const
	{
		auto node = getNode();
		if(node != nullptr) {
			auto r = node->getRenderable();
			if(r != nullptr) {
				scene_tree->addObject(r);
			}
		}
	}
}
