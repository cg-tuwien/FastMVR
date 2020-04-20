#include "Stopwatch.h"
#define NOMINMAX
#include <Windows.h>
#include <iostream>

Stopwatch::Stopwatch(std::string pMeasurementName, int pColor1, int pColor2)
	: mName{ pMeasurementName }
	, mStartEndMarkersRequested{ false }
	, mDontMeasureInDestructor{ false }
	, mStartTime{ std::chrono::high_resolution_clock::now() }
	, mColor1{ pColor1 }
	, mColor2{ pColor2 }
{
	mLastMeasureTime = mStartTime;
}

Stopwatch::~Stopwatch()
{
	if (!mDontMeasureInDestructor) {
		if (mName.empty()) {
			mName = "End of";
			Measure("Stopwatch", true);
		} 
		else {
			Measure("End", true);
		}
	}

	if (mStartEndMarkersRequested && mConsoleOutputEnabled) {
		static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(std_output_handle, 0x8); // gray on black
		std::cout << "|______________________________________" << std::endl;
		SetConsoleTextAttribute(std_output_handle, mColor1); 	SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
	}
}

void Stopwatch::EnablePrintingToConsole(bool enable)
{
	mConsoleOutputEnabled = enable;
}


void Stopwatch::PrintStartEndMarkers()
{
	if (mConsoleOutputEnabled) {
		static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(std_output_handle, 0x8); // gray on black
		std::cout << "_______________________________________\n" 
			         "| ";
		SetConsoleTextAttribute(std_output_handle, mColor1); 
		std::cout << mName << std::endl;
		SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
	}

	mStartEndMarkersRequested = true;
}

std::chrono::nanoseconds Stopwatch::Measure()
{
	return Measure("", true);
}

std::chrono::nanoseconds Stopwatch::Measure(std::string_view pMeasurementName, bool pPrintName)
{
	static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	auto current = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds resultSinceLastFrame;

	{	// since start
		auto time_span = current - mStartTime;
		auto int_min = std::chrono::duration_cast<std::chrono::minutes>(time_span).count();
		auto int_sec = std::chrono::duration_cast<std::chrono::seconds>(time_span).count();
		auto fp_ms = std::chrono::duration<double, std::milli>(time_span).count();
		if (mConsoleOutputEnabled) {
			SetConsoleTextAttribute(std_output_handle, 0x8); // gray on black
			std::cout << "| ";
			SetConsoleTextAttribute(std_output_handle, mColor2); 
			std::cout << "Elapsed since start:        ";
			SetConsoleTextAttribute(std_output_handle, mColor1); 
			printf("%d min, %lld sec %lf ms", int_min, int_sec - static_cast<decltype(int_sec)>(int_min) * 60, fp_ms - 1000.0 * int_sec);
		}
		// Get the result which to return to the caller
		resultSinceLastFrame = time_span;
	}
	if (mLastMeasureTime != mStartTime) {
		// since last measure
		std::cout << std::endl;
		auto time_span = current - mLastMeasureTime;
		auto int_min = std::chrono::duration_cast<std::chrono::minutes>(time_span).count();
		auto int_sec = std::chrono::duration_cast<std::chrono::seconds>(time_span).count();
		auto fp_ms = std::chrono::duration<double, std::milli>(time_span).count();
		if (mConsoleOutputEnabled) {
			SetConsoleTextAttribute(std_output_handle, 0x8); // gray on black
			std::cout << "| ";
			SetConsoleTextAttribute(std_output_handle, mColor2); 
			std::cout << "Elapsed since last measure: ";
			SetConsoleTextAttribute(std_output_handle, mColor1); 
			printf("%d min, %lld sec %lf ms", int_min, int_sec - static_cast<decltype(int_sec)>(int_min) * 60, fp_ms - 1000.0 * int_sec);
		}

		// Get the result which to return to the caller
		resultSinceLastFrame = time_span;
	}
	if (mConsoleOutputEnabled) {
		if (pPrintName) {
			SetConsoleTextAttribute(std_output_handle, mColor2); 
			std::cout << "\t" << mName;
			if (!pMeasurementName.empty()) {
				std::cout << " -> " << pMeasurementName;
			}
			std::cout << std::endl;
		}
		else {
			std::cout << std::endl;
		}
	}
	mLastMeasureTime = current;

	if (mConsoleOutputEnabled) {
		SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
	}

	return resultSinceLastFrame;
}

void Stopwatch::Stop()
{
	mDontMeasureInDestructor = true;
}
