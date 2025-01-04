#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include <deque>
#include <cassert>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here


struct VectorHash {
    std::size_t operator()(const std::vector<int>& vec) const {
        std::size_t hash = 0;
        for (int num : vec) {
            hash ^= std::hash<int>()(num) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};


std::vector<int> computeKMPTable(const std::deque<int>& pattern) {
    int m = pattern.size();
    std::vector<int> table(m+1);
    table[0] = -1;
    int j = 0;
    for (int i = 1; i < m; ++i,++j) {
        if (pattern[i] != pattern[j])
        {
            table[i] = j;
            while (j >= 0 && pattern[i] != pattern[j]) {
                j = table[j];
            }
        }
        else
        {
            table[i] = table[j];
        }
        
        // if (pattern[i] == pattern[j]) {
        //     ++j;
        // }
        // table[i] = j;
    }
    table[m] = j;
    return table;
}

bool kmpSearch(const std::vector<int>& text, const std::deque<int>& pattern) {
    std::vector<int> table = computeKMPTable(pattern);
   
    int j = 0 , i = 0;
    while (i < text.size())
    {
        if (text[i] == pattern[j]) {
            ++j;
            ++i;
            if (j == pattern.size())
            {
                return 1;
            }
        }
        // if (j == pattern.size()) {
        //     return 1;
        //     j = table[j - 1];
        // }
        else
        {
            j = table[j];
            if (j < 0)
            {    
                i++,j++;
            }
        }
    }
    return 0;
}
int find_accurate_matches(std::vector<int>& submission1, std::vector<int>& submission2, int minLen = 10, int max_length = 20) {
    
    int totalMatchLength = 0;
    int n = submission1.size();
    int i=0;
   
    std::deque<int> temp;
    for (int j = 0 ; j < minLen - 1 ; j ++)
    {
        temp.push_back(submission1[j]);
    }

    while(i<(n-minLen)){
        for(int j=i; temp.size() < minLen; j++)
            temp.push_back(submission1[j]);
        if(kmpSearch(submission2,temp)){
            totalMatchLength += minLen;
            i+=minLen;
            temp.clear();
        }
        else 
        {
            i++;
            temp.pop_front();
        }
    }
        

    return totalMatchLength;
}

// std::tuple<int, int, int> long_match(std::vector<int>& submission1, std::vector<int>& submission2, int min_length = 30, double threshold = 0.8) {
//     int n = submission1.size();
//     int m = submission2.size();

//     int max_len = 0;
//     int start_i = -1, start_j = -1;
//     for (int i = 0; i < n; ++i) {
//         for (int j = 0; j < m; ++j) {
//             int match_len = 0;
//             int mismatch_count = 0;
//             while (i + match_len < n && j + match_len < m) { 
//                 if (submission1[i + match_len] == submission2[j + match_len]) {
                  
//                 } else {
//                     mismatch_count++; 
//                 }

//                 match_len++;
//                 double similarity = static_cast<double>(match_len - mismatch_count) / match_len;
//                 if (match_len >= min_length && match_len > max_len && similarity >= threshold) {
//                     max_len = match_len;
//                     start_i = i;
//                     start_j = j;
//                 }
//             }
//         }
//     }

//     return (max_len >= min_length) ? std::make_tuple(max_len, start_i, start_j) : std::make_tuple(0, -1, -1);
// }

std::array<int,3> long_match(std::vector<int> &submission1, std::vector<int> &submission2 , double threshold = 0.95) // vary the values of threshold
{
    std::array<int,3> result;
    
    int length_variation = 30; // maximum variation in "longest length" allowed. increasing this reduces the time taken
    
    std::vector<std::vector<int> > dp(submission1.size()+1,std::vector<int>(submission2.size()+1));
    for (int i = 0 ; i <= submission1.size() ; i++)
    {
        dp[i][0] = i;
    }
    for (int i = 0 ; i <= submission2.size() ; i++)
    {
        dp[0][i] = i;
    }
    for (int i = 1 ; i <= submission1.size() ; i++)
    {
        for (int j = 1 ; j <= submission2.size() ; j++)
        {
            dp[i][j] = std::min(dp[i-1][j-1] + (submission1[i-1] != submission2[j-1]) , 1 + std::min(dp[i-1][j],dp[i][j-1])); // edit distance between the first i characters of the first string and first j characters of the second string.
        }
    }
 
    int l = 30 , r = std::min(submission2.size(),submission1.size()) , m ;
    while (l < r - length_variation)
    {
        bool found = false;
        m = (l+r) >> 1;
        for (int i = m ; i <= submission1.size() && !found ; i++)
        {
            for (int j = m ; j <= submission2.size() && !found; j++)
            {
                if (dp[i][j] - dp[i-m][j-m] < (1 - threshold) * m) // checks if there is a substring of length m (starting from i-m+1 for first string and j-m+1 for second string)  which has very less edit distance
                {
                    found = true;
                    result[0] = m;
                    result[1] = i-m;
                    result[2] = j-m;  
                }
            }
        }
        if (found)
        {
            l = m + 1;
        }
        else
        {
            r = m - 1;
        }
    }
    return result;
}


std::array<int, 5> match_submissions(std::vector<int> &submission1, 
        std::vector<int> &submission2) {
   
    std::array<int, 5> result = {0, 0, 0, -1, -1};
    result[1] = find_accurate_matches(submission1, submission2);
        
    auto [longest_match, start_index1, start_index2] = long_match(submission1, submission2);

    result[2] = longest_match;
    result[3] = start_index1;
    result[4] = start_index2;

    result[0] = (result[2] >= 40) ? 1 : 0;
    
    for (auto it : result)
    {
        std::cout << it << " ";
    }
    std::cout << "\n";
    
    return result; 
   
}


std::array<int,3> long_matches(std::vector<int> &submission1, std::vector<int> &submission2 , double threshold = 0.95)
{
    std::array<int,3> result;
    result[0] = 0;
    result[1] = -1;
    result[2] = -1;

    


    return result;
}

int main()
{
    std::vector<int>v1={1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9};
    std::vector<int>v2={1,2,3,4,5,6,77,7,8,9,1,2,3,4,5,6,77,7,8,9,1,2,3,4,5,6,7,8,9};
    for (auto it: match_submissions(v1,v2))
    {
        std::cout << it << " ";
    }
    return 0;
}