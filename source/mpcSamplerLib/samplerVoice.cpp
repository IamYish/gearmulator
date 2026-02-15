#include "samplerVoice.h"

#include <algorithm>
#include <cstring>

namespace mpcSamplerLib
{
	void SamplerVoice::noteOn(const uint8_t _note, const uint8_t _velocity, const SampleData* _sample, const PadConfig& _config, const float _sampleRate)
	{
		if (!_sample || _sample->isEmpty())
			return;

		m_sample = _sample;
		m_note = _note;
		m_velocity = static_cast<float>(_velocity) / 127.0f;
		m_volume = _config.volume;
		m_sampleRate = _sampleRate;
		m_muteGroup = _config.muteGroup;
		m_oneShot = _config.oneShot;
		m_playbackPos = 0.0;
		m_active = true;
		m_age = 0;

		// Calculate playback rate from tuning
		m_playbackRate = static_cast<double>(_sample->sampleRate) / static_cast<double>(_sampleRate);
		if (_config.tune != 0.0f)
			m_playbackRate *= std::pow(2.0, static_cast<double>(_config.tune) / 12.0);

		// Calculate pan (constant power panning)
		const float panAngle = (_config.pan + 1.0f) * 0.25f * 3.14159265f;
		m_panLeft = std::cos(panAngle);
		m_panRight = std::sin(panAngle);

		// Setup envelope
		const float minRate = 1.0f / (_sampleRate * 10.0f);  // minimum 10 second envelope stage

		m_envelopeStage = EnvelopeStage::Attack;
		m_envelopeLevel = 0.0f;
		m_attackRate = _config.attack > 0.001f ? 1.0f / (_config.attack * _sampleRate) : 1.0f;
		m_decayRate = _config.decay > 0.001f ? 1.0f / (_config.decay * _sampleRate) : 1.0f;
		m_sustainLevel = _config.sustain;
		m_releaseRate = _config.release > 0.001f ? 1.0f / (_config.release * _sampleRate) : 1.0f;

		m_attackRate = std::max(m_attackRate, minRate);
		m_decayRate = std::max(m_decayRate, minRate);
		m_releaseRate = std::max(m_releaseRate, minRate);
	}

	void SamplerVoice::noteOff()
	{
		if (!m_active)
			return;

		if (m_oneShot)
			return;  // One-shot samples play through regardless

		m_envelopeStage = EnvelopeStage::Release;
	}

	void SamplerVoice::forceStop()
	{
		m_active = false;
		m_envelopeStage = EnvelopeStage::Off;
		m_envelopeLevel = 0.0f;
	}

	void SamplerVoice::process(float* _left, float* _right, const size_t _numSamples)
	{
		if (!m_active || !m_sample || m_sample->isEmpty())
			return;

		const auto numSampleFrames = static_cast<double>(m_sample->getNumSamples());
		const bool isStereo = m_sample->isStereo;

		for (size_t i = 0; i < _numSamples; ++i)
		{
			if (!m_active)
				break;

			// Check if we've reached the end of the sample
			if (m_playbackPos >= numSampleFrames - 1)
			{
				m_active = false;
				break;
			}

			// Linear interpolation
			const auto pos = static_cast<size_t>(m_playbackPos);
			const auto frac = static_cast<float>(m_playbackPos - static_cast<double>(pos));

			const size_t nextPos = std::min(pos + 1, m_sample->getNumSamples() - 1);

			float sampleL = m_sample->left[pos] + (m_sample->left[nextPos] - m_sample->left[pos]) * frac;
			float sampleR = isStereo
				? m_sample->right[pos] + (m_sample->right[nextPos] - m_sample->right[pos]) * frac
				: sampleL;

			// Apply envelope
			const float envLevel = getEnvelopeLevel();

			// Apply velocity, volume, envelope, and panning
			const float gain = m_velocity * m_volume * envLevel;
			_left[i] += sampleL * gain * m_panLeft;
			_right[i] += sampleR * gain * m_panRight;

			// Advance playback position
			m_playbackPos += m_playbackRate;

			// Advance envelope (per-sample for accuracy)
			advanceEnvelope(1);

			if (m_envelopeStage == EnvelopeStage::Off)
			{
				m_active = false;
				break;
			}
		}
	}

	float SamplerVoice::getEnvelopeLevel()
	{
		return m_envelopeLevel;
	}

	void SamplerVoice::advanceEnvelope(const size_t _numSamples)
	{
		for (size_t i = 0; i < _numSamples; ++i)
		{
			switch (m_envelopeStage)
			{
			case EnvelopeStage::Attack:
				m_envelopeLevel += m_attackRate;
				if (m_envelopeLevel >= 1.0f)
				{
					m_envelopeLevel = 1.0f;
					m_envelopeStage = EnvelopeStage::Decay;
				}
				break;

			case EnvelopeStage::Decay:
				m_envelopeLevel -= m_decayRate;
				if (m_envelopeLevel <= m_sustainLevel)
				{
					m_envelopeLevel = m_sustainLevel;
					m_envelopeStage = EnvelopeStage::Sustain;
				}
				break;

			case EnvelopeStage::Sustain:
				// Hold at sustain level until note off
				if (m_oneShot && m_sustainLevel <= 0.0f)
				{
					m_envelopeStage = EnvelopeStage::Off;
				}
				break;

			case EnvelopeStage::Release:
				m_envelopeLevel -= m_releaseRate;
				if (m_envelopeLevel <= 0.0f)
				{
					m_envelopeLevel = 0.0f;
					m_envelopeStage = EnvelopeStage::Off;
				}
				break;

			case EnvelopeStage::Off:
				return;
			}
		}
	}
}
