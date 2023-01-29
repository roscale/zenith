#include <algorithm>
#include "rect.hpp"

FlutterRect rect_union(const FlutterRect& rect1, const FlutterRect& rect2) {
	return {
		  .left = std::min(rect1.left, rect2.left),
		  .top = std::min(rect1.top, rect2.top),
		  .right = std::max(rect1.right, rect2.right),
		  .bottom = std::max(rect1.bottom, rect2.bottom),
	};
}
