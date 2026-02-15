#pragma once

#include <cstdint>
#include <vector>
#include <cmath>

namespace mpcSamplerLib
{
	struct SampleData
	{
		std::vector<float> left;
		std::vector<float> right;
		float sampleRate = 44100.0f;
		bool isStereo = false;

		bool isEmpty() const { return left.empty(); }
		size_t getNumSamples() const { return left.size(); }
	};

	struct PadConfig
	{
		uint8_t midiNote = 0;
		float volume = 1.0f;
		float pan = 0.0f;           // -1.0 to 1.0
		float tune = 0.0f;          // semitones, -24.0 to +24.0
		float attack = 0.001f;      // seconds
		float decay = 0.1f;         // seconds
		float sustain = 1.0f;       // 0.0 to 1.0
		float release = 0.05f;      // seconds
		uint8_t muteGroup = 0;      // 0 = no group
		bool oneShot = true;        // typical for drum pads
	};

	class SamplerVoice
	{
	public:
		SamplerVoice() = default;

		void noteOn(uint8_t _note, uint8_t _velocity, const SampleData* _sample, const PadConfig& _config, float _sampleRate);
		void noteOff();
		void forceStop();

		void process(float* _left, float* _right, size_t _numSamples);

		bool isActive() const { return m_active; }
		bool isReleasing() const { return m_envelopeStage == EnvelopeStage::Release; }
		uint8_t getNote() const { return m_note; }
		uint8_t getPadIndex() const { return m_padIndex; }
		void setPadIndex(uint8_t _idx) { m_padIndex = _idx; }
		uint8_t getMuteGroup() const { return m_muteGroup; }
		uint32_t getAge() const { return m_age; }
		void incrementAge() { ++m_age; }

	private:
		enum class EnvelopeStage
		{
			Attack,
			Decay,
			Sustain,
			Release,
			Off
		};

		float getEnvelopeLevel();
		void advanceEnvelope(size_t _numSamples);

		const SampleData* m_sample = nullptr;
		double m_playbackPos = 0.0;
		double m_playbackRate = 1.0;
		float m_velocity = 1.0f;
		float m_volume = 1.0f;
		float m_panLeft = 1.0f;
		float m_panRight = 1.0f;
		float m_sampleRate = 44100.0f;
		uint8_t m_note = 0;
		uint8_t m_padIndex = 0;
		uint8_t m_muteGroup = 0;
		bool m_active = false;
		bool m_oneShot = true;
		uint32_t m_age = 0;

		// Envelope
		EnvelopeStage m_envelopeStage = EnvelopeStage::Off;
		float m_envelopeLevel = 0.0f;
		float m_attackRate = 0.0f;
		float m_decayRate = 0.0f;
		float m_sustainLevel = 1.0f;
		float m_releaseRate = 0.0f;
	};
}
