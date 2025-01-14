/*
 * AnalogIn.h
 *
 *  Created on: 6 Sep 2018
 *      Author: David
 */

#ifndef SRC_HARDWARE_ANALOGIN_H_
#define SRC_HARDWARE_ANALOGIN_H_

#include <CoreIO.h>
#if STM32H7
#include "stm32h7xx_ll_adc.h"
// ADC VREF and MCU Temperature calibration values
#define TEMPSENSOR_CAL1_DEF 931
#define TEMPSENSOR_CAL2_DEF 1197
#define VREFINT_CAL_DEF 1500
// I'm not totally sure what is going on here H743 seems to store the values scaled by 16, but H723 does not
#if STM32H723xx
#define GET_ADC_CAL(CAL, DEF) (*CAL == 0xffff ? DEF : *CAL)
#elif STM32H743xx
#define GET_ADC_CAL(CAL, DEF) (*CAL == 0xffff ? DEF : ((*CAL)>>4))
#else
#error "Undefined mcu, check calibration data scaling"
#endif
#elif STM32F4
#include "stm32f4xx_ll_adc.h"
// ADC VREF and MCU Temperature calibration values
#define TEMPSENSOR_CAL1_DEF 931
#define TEMPSENSOR_CAL2_DEF 1197
#define VREFINT_CAL_DEF 1500
#define GET_ADC_CAL(CAL, DEF) (*CAL == 0xffff ? DEF : *CAL)
#endif

typedef void (*AnalogInCallbackFunction)(CallbackParameter p, uint32_t reading) noexcept;

namespace AnalogIn
{
	// The number of bits that the ADCs return
#if SAMC21
	constexpr unsigned int AdcBits = 16;
#elif SAME5x
	constexpr unsigned int AdcBits = 16;
#elif STM32
	constexpr unsigned int AdcBits = 14;
#elif RP2040
	constexpr unsigned int AdcBits = 12;
#endif

#ifdef RTOS
	// Initialise the analog input subsystem. Call this just once.
	// For the SAME5x we need 4 DMA channels. For the SAMC21 we need 1 DMA channel, or 2 if supporting the SDADC.
	void Init(
#if SAME5x
				NvicPriority interruptPriority
#elif SAMC21
				DmaChannel dmaChan,
				DmaPriority rxPriority,
				bool extRef
#elif RP2040
				DmaChannel dmaChan,
				DmaPriority rxPriority
#endif
			) noexcept;

	// Shut down the analog system. making it safe to terminate the AnalogIn task
	void Exit() noexcept;

	// Enable analog input on a pin.
	// Readings will be taken and about every 'ticksPerCall' milliseconds the callback function will be called with the specified parameter and ADC reading.
	// Set ticksPerCall to 0 to get a callback on every reading.
	// Warning! there is nothing to stop you enabling a channel twice, in which case in the SAME51 configuration, it will be read twice in the sequence.
	// After calling this, the result of calling ReadChannel will be zero until a reading has been taken.
	bool EnableChannel(AdcInput adcin, AnalogInCallbackFunction fn, CallbackParameter param, uint32_t ticksPerCall, bool useAlternateAdc = false) noexcept;

	// Readings will be taken and about every 'ticksPerCall' milliseconds the callback function will be called with the specified parameter and ADC reading.
	// Set ticksPerCall to 0 to get a callback on every reading.
	// Call this with fn == nullptr to stop getting callbacks.
	bool SetCallback(AdcInput adcin, AnalogInCallbackFunction fn, CallbackParameter param, uint32_t ticksPerCall, bool useAlternateAdc = false) noexcept;

	// Return whether or not the channel is enabled
	bool IsChannelEnabled(AdcInput adcin, bool useAlternateAdc = false) noexcept;

	// Disable a previously-enabled channel
	void DisableChannel(AdcInput adcin, bool useAlternateAdc = false) noexcept;

	// Get the latest result from a channel. The channel must have been enabled first.
	uint16_t ReadChannel(AdcInput adcin) noexcept;

	// Get the number of conversions that were started
	void GetDebugInfo(uint32_t &convsStarted, uint32_t &convsCompleted, uint32_t &convTimeouts, uint32_t& errs) noexcept;

#if SAME5x
	// Enable an on-chip MCU temperature sensor. We don't use this on the SAMC21 because that chip has a separate TSENS peripheral.
	bool EnableTemperatureSensor(unsigned int sensorNumber, AnalogInCallbackFunction fn, CallbackParameter param, uint32_t ticksPerCall, unsigned int adcnum) noexcept;
#endif

#if SAMC21 || RP2040
	void EnableTemperatureSensor(AnalogInCallbackFunction fn, CallbackParameter param, uint32_t ticksPerCall) noexcept;
#endif

	extern "C" [[noreturn]] void TaskLoop(void*) noexcept;

	typedef void AdcTaskHookFunction() noexcept;
	AdcTaskHookFunction *SetTaskHook(AdcTaskHookFunction *func) noexcept;

#else

# if SAMC21 || SAME5x
	// Simple analog input functions, for projects that don't use RTOS e.g. bootloaders
	void Init(Adc * device);
	void Disable(Adc * device);
	uint16_t ReadChannel(Adc * device, uint8_t channel);
# endif

#endif
}

namespace LegacyAnalogIn
{
	// Module initialisation
	void AnalogInInit() noexcept;

	// Enable or disable a channel. Use AnalogCheckReady to make sure the ADC is ready before calling this.
	void AnalogInEnableChannel(AnalogChannelNumber channel, bool enable) noexcept;

	// Return the number of bits provided by a call to AnalogInReadChannel
#if SAME70
	static constexpr unsigned int AdcBits = 14;
#elif STM32
	static constexpr unsigned int AdcBits = 14;
#else
	static constexpr unsigned int AdcBits = 12;
#endif

	// Read the most recent result from a channel
	uint16_t AnalogInReadChannel(AnalogChannelNumber channel) noexcept;

	typedef void (*AnalogCallback_t)(void) noexcept;

	// Set up a callback for when all conversions have been completed. Returns the previous callback pointer.
	AnalogCallback_t AnalogInSetCallback(AnalogCallback_t) noexcept;

	// Start converting the enabled channels, to include the specified ones. Disabled channels are ignored.
	void AnalogInStartConversion(uint32_t channels = 0xFFFFFFFFu) noexcept;

	// Finalise a conversion
#if SAME70
	void AnalogInFinaliseConversion() noexcept;
#else
	static inline void AnalogInFinaliseConversion() noexcept { }
#endif

	// Check whether all conversions of the specified channels have been completed since the last call to AnalogStartConversion.
	// Disabled channels are ignored
	bool AnalogInCheckReady(uint32_t channels = 0xFFFFFFFFu) noexcept;

	// Convert a pin number to an AnalogIn channel
	extern AnalogChannelNumber PinToAdcChannel(uint32_t pin) noexcept;

	// Get the temperature measurement channel
	extern AnalogChannelNumber GetTemperatureAdcChannel() noexcept;

#if STM32
	// Get the VRef measurement channel
	extern AnalogChannelNumber GetVREFAdcChannel() noexcept;
#endif

}

#ifdef RTOS

// This function is for backwards compatibility with CoreNG
inline uint16_t AnalogInReadChannel(AdcInput adcin)
{
#if SAME70 || SAM4E || SAM4S || STM32
	return LegacyAnalogIn::AnalogInReadChannel(adcin);
#else
	return AnalogIn::ReadChannel(adcin);
#endif
}

// This function is for backwards compatibility with CoreNG
inline void AnalogInEnableChannel(AdcInput adcin, bool enable)
{
#if SAME70 || SAM4E || SAM4S || STM32
	LegacyAnalogIn::AnalogInEnableChannel(adcin, enable);
#else
	if (enable)
	{
		if (!AnalogIn::IsChannelEnabled(adcin))
		{
			AnalogIn::EnableChannel(adcin, nullptr, CallbackParameter(), 1000);
		}
	}
	else
	{
		AnalogIn::DisableChannel(adcin);
	}
#endif
}

#endif

#endif /* SRC_HARDWARE_ANALOGIN_H_ */
