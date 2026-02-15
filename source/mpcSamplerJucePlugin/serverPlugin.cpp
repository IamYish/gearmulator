// ReSharper disable once CppUnusedIncludeDirective
#include "client/plugin.h"

#include "mpcSamplerLib/samplerDevice.h"

synthLib::Device* createBridgeDevice(const synthLib::DeviceCreateParams& _params)
{
	return new mpcSamplerLib::SamplerDevice(_params);
}
