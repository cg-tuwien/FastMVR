#pragma once
#include <chrono>
#include <string>

class Stopwatch
{
public:
	Stopwatch(std::string pMeasurementName = "", int pColor1 = 0xD, int pColor2 = 0x5);
	Stopwatch(Stopwatch&&) = default;
	Stopwatch(const Stopwatch&) = default;
	Stopwatch& operator=(Stopwatch&&) = default;
	Stopwatch& operator=(const Stopwatch&) = default;
	~Stopwatch();

	void EnablePrintingToConsole(bool enable);
	void PrintStartEndMarkers();
	std::chrono::nanoseconds Measure();
	std::chrono::nanoseconds Measure(std::string_view pMeasurementName, bool pPrintName = true);

	void Stop();

private:
	std::string mName;
	bool mConsoleOutputEnabled;
	bool mStartEndMarkersRequested;
	bool mDontMeasureInDestructor;
	int mColor1;
	int mColor2;
	std::chrono::high_resolution_clock::time_point mStartTime;
	std::chrono::high_resolution_clock::time_point mLastMeasureTime;
};

