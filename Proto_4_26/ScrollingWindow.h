#include <iostream>
#include <deque>
#include <cstdlib>
#include <algorithm>
#include <vector>

using namespace std;

// deque<int> window1;
// int window_size1 = 4;
// int threshold1 = 4;

////////////////////////////////////////////////////////////////
////Prototypes
///////////////////////////////////////////////////////////////
int median(deque<int> set);
void MAD(deque<int> &window, int window_size, int threshold);
void maintain_window(int insert_data, deque<int> &window, int window_size, int threshold);

////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// int main()
// {
//     vector<int> bpm = {500.24, 29.53, 85.23, 86, 331.49, 70.92, 62.18, 90.23, 106.57};

//     for (int i = 0; i < bpm.size(); ++i)
//     {
//         // Put function here
//         maintain_window(bpm[i], window1, window_size1, threshold1);
//     }

//     return 0;
// }

int median(deque<int> set)
{
    int size = set.size();
    int result;
    if (size % 2 == 0)
    {
        // will sort left half of it from lowest to greatest
        nth_element(set.begin(), set.begin() + size / 2 - 1, set.end());
        // Gets the middle element thats to the left
        int median1 = set[size / 2 - 1];

        // will sort the right half of it from lowest to greatest
        nth_element(set.begin(), set.begin() + size / 2, set.end());
        // Gets the middle element thats to the right
        int median2 = set[size / 2];

        result = (median1 + median2) / 2.0;
    }
    else
    {
        // Gets the middle point
        nth_element(set.begin(), set.begin() + set.size() / 2, set.end());
        result = set[size / 2];
    }
    result = round(result * 100) / 100;
    return result;
}

void MAD(deque<int> &window, int window_size, int threshold)
{
    deque<int> values; // Only using because I made median take a dequeue
    int MAD_value;

    for (int x : window)
    {
        int result = abs(x - median(window));
        result = round(result * 100) / 100;
        values.push_back(result); // Push the the differences into values
        cout << "abs"
             << "( " << x << " - " << median(window) << ") = " << result << endl;
    }

    MAD_value = median(values); // Calculate overall median in values
    cout << "MAD value = " << MAD_value << endl;

    for (int i = 0; i < window_size; i++)
    {
        if (values[i] > threshold * MAD_value)
        {
            cout << "Threshold is: " << threshold * MAD_value << endl;
            cout << window[i] << " is an outlier " << endl;
            window.erase(window.begin() + i); // remove outlier from window
            // TODO: Erase in data struc as well
        }
    }
}

void maintain_window(int insert_data, deque<int> &window, int window_size, int threshold)
{
    insert_data = round(insert_data * 100) / 100;
    // Push data until full
    if (window.size() != window_size)
    {
        window.push_back(insert_data);
        if (window.size() == window_size)
        {
            cout << "Median = " << median(window) << endl;
            MAD(window, window_size, threshold);
        }
    }
    else
    {
        cout << "Median = " << median(window) << endl;
        // Remove the top
        window.pop_front();
        /* it will return and iterator to its end if it can't find the data
            so, it doesn't screw the window with duplicate values */
        auto it = find(window.begin(), window.end(), insert_data);
        if (it == window.end())
        {
            // The value is not found, push the new data
            window.push_back(insert_data);
            MAD(window, window_size, threshold);
        }
        else
        {
            cout << insert_data << " ALREADY IS IN" << endl;
        }
    }
}