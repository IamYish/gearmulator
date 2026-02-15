#include "mpcController.h"

#include "PluginProcessor.h"

#include "synthLib/midiTypes.h"

namespace mpcSamplerJucePlugin
{
	Controller::Controller(AudioPluginAudioProcessor& _processor)
		: pluginLib::Controller(_processor, "parameterDescriptions_mpc.json")
	{
		registerParams(_processor);
	}

	Controller::~Controller() = default;

	void Controller::onStateLoaded()
	{
	}

	bool Controller::parseSysexMessage(const pluginLib::SysEx& _msg, synthLib::MidiEventSource _source)
	{
		return false;
	}

	void Controller::sendParameterChange(const pluginLib::Parameter& _parameter, const pluginLib::ParamValue _value, pluginLib::Parameter::Origin _origin)
	{
		const auto& desc = _parameter.getDescription();

		// Map parameter changes to MIDI CC messages for the sampler device
		// Page 0 = master parameters, Page 1-16 = per-pad parameters
		if (desc.page == 0)
		{
			// Master parameters
			switch (desc.index)
			{
			case 0: // MasterVolume
				sendMidiEvent(synthLib::M_CONTROLCHANGE, synthLib::MC_MAINVOLUME, static_cast<uint8_t>(_value));
				break;
			default:
				break;
			}
		}
	}
}
