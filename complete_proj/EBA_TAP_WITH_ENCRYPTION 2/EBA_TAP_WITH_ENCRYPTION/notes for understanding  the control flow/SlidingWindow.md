The `sliding window` is used for inforcing the limit of performing the operation (1000 window size) in 1000 ms (1 sec).

1. Capture the Current Time:
-------------------------

As soon as the object of the sliding window is created current time is captured.
auto current_time_pt = steady_clock::now();

This captures the current time point.

2. duration b/w start time and current time:
--------------------------------------------

milliseconds tmp_ml = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_pt - clock_start_time_pt);

This calculates the duration in milliseconds from the start time (clock_start_time_pt) to the current time point (current_time_pt).

3. Calculate Time Difference:
-----------------------------

int i_diff = tmp_ml.count() - ml[i_idx_pos].count();

This calculates the difference between the current elapsed time and the time stored in the ml array at the current index position (i_idx_pos).

4. Check If Rate Limit Is Exceeded:
-----------------------------------

if (i_diff < i_window_sz_milliseconds)

This checks if the time difference (i_diff) is less than the window size (i_window_sz_milliseconds, which is set to 1000 milliseconds or 1 second). If it is, it means the operation is happening too quickly and must be delayed.

5. Sleep If Necessary:
----------------------

int i_sleep = i_window_sz_milliseconds - i_diff;
std::this_thread::sleep_for(std::chrono::milliseconds(i_sleep));

If the time difference is less than the window size, the thread sleeps for the remaining time to ensure that operations are spaced out appropriately.

6. Update Current Time After Sleep:
-----------------------------------

current_time_pt = steady_clock::now();
tmp_ml = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_pt - clock_start_time_pt);

After sleeping, the current time is captured again, and the elapsed time is recalculated.

7. Store the Current Time in the Array:
---------------------------------------

ml[i_idx_pos] = tmp_ml;

The updated elapsed time is stored in the ml array at the current index position.

8. Update Index Position:
-------------------------

i_idx_pos = (i_idx_pos >= (i_max_orders_in_window - 1)) ? 0 : ++i_idx_pos;

This line updates the index position. If the current index position is greater than or equal to the maximum allowed orders minus one (i_max_orders_in_window - 1), it wraps around to zero. Otherwise, it increments the index position by one.