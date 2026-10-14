#pragma once
#include "rengb_model.hpp"

namespace rengb{
	struct Renderable
	{
		std::shared_ptr<RengbModel> model{};
		float life{};
		int partType = 0;
		bool mirrorable = false;

		float loading{0.f};
		int loadDirection{1}; // 1 is from down up, 2 is from forward back, 3 is
	};    
}
