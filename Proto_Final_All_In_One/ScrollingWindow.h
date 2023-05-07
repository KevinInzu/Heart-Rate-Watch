#include <iostream>
#include <deque>
#include <cstdlib>
#include <algorithm>
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////////
////Prototypes
///////////////////////////////////////////////////////////////
int median(deque<int> set);
void MAD(vector<int> &data_set, deque<int> &window, int window_size, int threshold);
void maintain_window(vector<int> &data_set, int insert_data, deque<int> &window, int window_size, int threshold);
////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

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
    return result;
}

void MAD(vector<int> &data_set, deque<int> &window, int window_size, int threshold)
{
    deque<int> values; // Only using because I made median take a dequeue
    int MAD_value;

    for (int x : window)
    {
        int result = abs(x - median(window));
        values.push_back(result); // Push the the differences into values
        // cout << "abs"
        //      << "( " << x << " - " << median(window) << ") = " << result << endl;
    }

    MAD_value = median(values); // Calculate overall median in values
    // cout << "MAD value = " << MAD_value << endl;
    // cout << "Threshold is: " << threshold * MAD_value << endl;

    for (int i = 0; i < window_size; i++)
    {
        if (values[i] > threshold * MAD_value)
        {
            int outlier = window.at(i);
            // cout << values[i] << " > " << threshold * MAD_value << " so, " << outlier << " is an outlier " << endl;
            window.erase(window.begin() + i); // remove outlier from window

            /*Find the first match and remove it from the vector */
            auto it = find(data_set.begin(), data_set.end(), outlier);
            /* ^^ it will return and iterator to its end if it can't find the data*/

            if (it != data_set.end()) // If it doesn't return the end, then the value is found, so remove it from vector
            {
                // cout << "Removing from vector: " << *it << endl;
                data_set.erase(it);
            }
        }
    }
}

void maintain_window(vector<int> &data_set, int insert_data, deque<int> &window, int window_size, int threshold)
{
    // Push data until full
    if (window.size() != window_size)
    {   
        // cout << "Is not full" << endl;
        // TODO: Refactor to a function
        auto it = find(window.begin(), window.end(), insert_data);
        if (it == window.end())
        {
            // cout << "VALUE NOT FOUND PUSHING " << insert_data << endl;
            window.push_back(insert_data);
        }

        if (window.size() == window_size)
        {
            // cout << "Median = " << median(window) << endl;
            // TODO: Refactor to a function
            /* it will return and iterator to its end if it can't find the data
            so, it doesn't screw the window with duplicate values
            vvv                                                            */
            auto it1 = find(window.begin(), window.end(), insert_data);
            if (it1 == window.end())
            {
                // The value is not found, push the new data
                // cout << "VALUE NOT FOUND PUSHING " << insert_data << endl;
                window.push_back(insert_data);
                MAD(data_set, window, window_size, threshold);
            }
            else
            {
                // cout << insert_data << " ALREADY IS IN" << endl;
            }
        }
    }
    else
    {
        // cout << "Is full" << endl;
        // cout << "Median = " << median(window) << endl;
        // Remove the top
        window.pop_front();
        // TODO: Refactor to a function
        /* it will return and iterator to its end if it can't find the data
            so, it doesn't screw the window with duplicate values
            vvv                                                            */
        auto it2 = find(window.begin(), window.end(), insert_data);
        if (it2 == window.end())
        {
            // The value is not found, push the new data
            // cout << "VALUE NOT FOUND PUSHING " << insert_data << endl;
            window.push_back(insert_data);
            MAD(data_set, window, window_size, threshold);
        }
        else
        {
            // cout << insert_data << " ALREADY IS IN" << endl;
        }
    }
}
