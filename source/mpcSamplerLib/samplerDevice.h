#pragma once

#include "samplerEngine.h"
#include "synthLib/device.h"

#include <vector>
#include <mutex>

namespace mpcSamplerLib
{
	class SamplerDevice final : public synthLib::Device
	{
	public:
		explicit SamplerDevice(const synthLib::DeviceCreateParams& _params);
		~SamplerDevice() override;

		float getSamplerate() const override;
		bool setSamplerate(float _samplerate) override;
		bool isValid() const override;

		bool getState(std::vector<uint8_t>& _state, synthLib::StateType _type) override;
		bool setState(const std::vector<uint8_t>& _state, synthLib::StateType _type) override;

		uint32_t getChannelCountIn() override { return 2; }
		uint32_t getChannelCountOut() override { return 2; }

		bool setDspClockPercent(uint32_t _percent) override { return true; }
		uint32_t getDspClockPercent() const override { return 100; }
		uint64_t getDspClockHz() const override { return 0; }

		uint32_t getInternalLatencyMidiToOutput() const override { return 0; }
		uint32_t getInternalLatencyInputToOutput() const override { return 0; }

		SamplerEngine& getEngine() { return m_engine; }
		const SamplerEngine& getEngine() const { return m_engine; }

	protected:
		void readMidiOut(std::vector<synthLib::SMidiEvent>& _midiOut) override;
		void processAudio(const synthLib::TAudioInputs& _inputs, const synthLib::TAudioOutputs& _outputs, size_t _samples) override;
		bool sendMidi(const synthLib::SMidiEvent& _ev, std::vector<synthLib::SMidiEvent>& _response) override;

	private:
		SamplerEngine m_engine;
		float m_sampleRate = 44100.0f;
		std::vector<synthLib::SMidiEvent> m_pendingMidiOut;
	};
}
