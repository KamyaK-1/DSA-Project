#include "structures.hpp"
#include<thread>
#include<chrono>
#include<map>
#include<algorithm>
#include<utility>
#include<set>
#include<mutex>
#include<atomic>
#include<condition_variable>
#include<cassert>
#include<queue>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

class plagiarism_checker_t {
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> &__submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // TODO: Add members and function signatures here
    
    bool finished ;
    int initial_length;
    
    void start_plag_checking(void);
    void calculate_hashes(std::shared_ptr<submission_t>);
    void call_plag(std::shared_ptr<submission_t>);
    void queue_submission(std::chrono::steady_clock::time_point x, std::shared_ptr<submission_t> __submission);

    std::vector<std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<submission_t>>> submissions;
    std::queue<std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<submission_t>>> queued_submissions;
    std::map<std::shared_ptr<submission_t>, std::vector<unsigned int> > hashes_15, hashes_75;
    std::map<std::shared_ptr<submission_t>, std::vector<unsigned int> > hashes_15_unsorted;
    std::vector<bool> already_plagged;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> is_adding_submission;
    std::thread evaluate_submissions;

    // End TODO
};