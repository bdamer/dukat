#pragma once

namespace dukat
{
	enum Event
	{
		None, // Range marker - does not trigger

		Created,
		Destroyed,
		Selected,
		Deselected,
		ParentChanged,
		TransformChanged,
		VisibilityChanged,

		Any // catch-all to allow subscription to all supported events
	};
}