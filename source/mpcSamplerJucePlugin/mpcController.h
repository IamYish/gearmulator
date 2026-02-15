#pragma once

#include "jucePluginLib/controller.h"

namespace mpcSamplerJucePlugin
{
	class AudioPluginAudioProcessor;

	class Controller : public pluginLib::Controller
	{
	public:
		explicit Controller(AudioPluginAudioProcessor& _processor);
		~Controller() override;

		uint8_t getPartCount() const override { return 1; }

	private:
		void onStateLoaded() override;
		bool parseSysexMessage(const pluginLib::SysEx&, synthLib::MidiEventSource _source) override;
		void sendParameterChange(const pluginLib::Parameter& _parameter, pluginLib::ParamValue _value, pluginLib::Parameter::Origin _origin) override;
	};
}
