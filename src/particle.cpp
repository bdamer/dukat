#include "stdafx.h"
#include <dukat/particle.h>

namespace dukat
{
	MemoryPool<Particle> Particle::_pool(4096);
}