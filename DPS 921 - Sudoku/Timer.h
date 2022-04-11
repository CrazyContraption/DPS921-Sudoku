#pragma once
#include <chrono> // Timer objects

using namespace std::chrono;

class Timer {
private:
	bool m_running;
	bool m_hasStarted;
	steady_clock::time_point m_start;
	steady_clock::time_point m_end;

public:
	Timer() {
		m_hasStarted = false;
		m_running = false;
	}

	/// <summary>Starts the timer at the current time of the call.
	/// </summary>
	void start() {
		m_hasStarted = true;
		m_running = true;
		m_start = high_resolution_clock::now();
	}

	/// <summary>Stops the timer at the current time of the call.
	/// </summary>
	void stop() {
		m_end = high_resolution_clock::now();
		m_running = false;
	}

	/// <summary>Gives the duration in milliseconds of the current timer points. If no end is set, gets the duration from the call time.
	/// <returns>Integer of the seconds between the start and end time points, defaults to 0 if not points set, negative if ended time is before start time</returns>
	/// </summary>
	int getDuration() {
		if (m_running)
			return duration_cast<milliseconds>(m_end - m_start).count();
		if (m_hasStarted)
			return duration_cast<milliseconds>(high_resolution_clock::now() - m_start).count();
		return 0;
	}
};